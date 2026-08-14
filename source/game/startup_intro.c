#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <SDL2/SDL.h>
#include <coreinit/cache.h>
#include <sndcore2/core.h>
#include <sndcore2/voice.h>
#include <whb/proc.h>
#include <zlib.h>

#include "platform/frame_clock.h"
#include "platform/graphics.h"
#include "platform/input.h"

extern const uint8_t startup_intro_video_bin[];
extern const uint8_t startup_intro_video_bin_end[];
extern const uint8_t startup_intro_audio_bin[];
extern const uint8_t startup_intro_audio_bin_end[];

#define STARTUP_INTRO_MAGIC_0 'P'
#define STARTUP_INTRO_MAGIC_1 'D'
#define STARTUP_INTRO_MAGIC_2 'D'
#define STARTUP_INTRO_MAGIC_3 '1'
#define STARTUP_INTRO_VERSION 1u
#define STARTUP_INTRO_PALETTE_COLOURS 256u
#define STARTUP_INTRO_MAX_WINDOW_ID 32u

typedef struct StartupIntroVideo {
    uint16_t width;
    uint16_t height;
    uint16_t frame_count;
    uint16_t fps_num;
    uint16_t fps_den;
    const uint8_t *palette;
    const uint8_t *record;
    const uint8_t *end;
    uint8_t *indices;
    uint8_t *delta;
    uint8_t *rgba;
    int decoded_frame;
    SDL_Renderer *tv_renderer;
    SDL_Renderer *drc_renderer;
    SDL_Texture *tv_texture;
    SDL_Texture *drc_texture;
} StartupIntroVideo;

static uint16_t startup_intro_read_be16(const uint8_t *data)
{
    return (uint16_t)(((uint16_t)data[0] << 8) | (uint16_t)data[1]);
}

static uint32_t startup_intro_read_be32(const uint8_t *data)
{
    return ((uint32_t)data[0] << 24) |
           ((uint32_t)data[1] << 16) |
           ((uint32_t)data[2] << 8) |
           (uint32_t)data[3];
}

static SDL_Renderer *startup_intro_find_renderer(const char *title_fragment)
{
    for (uint32_t id = 1u; id <= STARTUP_INTRO_MAX_WINDOW_ID; ++id) {
        SDL_Window *window = SDL_GetWindowFromID(id);
        if (window == NULL) continue;
        const char *title = SDL_GetWindowTitle(window);
        if (title != NULL && strstr(title, title_fragment) != NULL)
            return SDL_GetRenderer(window);
    }
    return NULL;
}

static SDL_Texture *startup_intro_create_texture(SDL_Renderer *renderer,
                                                 uint16_t width,
                                                 uint16_t height)
{
    if (renderer == NULL) return NULL;
    SDL_Texture *texture = SDL_CreateTexture(renderer,
                                             SDL_PIXELFORMAT_RGBA32,
                                             SDL_TEXTUREACCESS_STREAMING,
                                             width,
                                             height);
    if (texture == NULL) return NULL;
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_NONE);
    SDL_SetTextureScaleMode(texture, SDL_ScaleModeLinear);
    return texture;
}

static void startup_intro_video_shutdown(StartupIntroVideo *video)
{
    if (video == NULL) return;
    if (video->tv_texture != NULL) SDL_DestroyTexture(video->tv_texture);
    if (video->drc_texture != NULL) SDL_DestroyTexture(video->drc_texture);
    free(video->indices);
    free(video->delta);
    free(video->rgba);
    memset(video, 0, sizeof(*video));
    video->decoded_frame = -1;
}

static bool startup_intro_video_init(StartupIntroVideo *video)
{
    if (video == NULL) return false;
    memset(video, 0, sizeof(*video));
    video->decoded_frame = -1;

    const uint8_t *data = startup_intro_video_bin;
    const uint8_t *end = startup_intro_video_bin_end;
    const size_t header_size = 4u + 16u;
    const size_t palette_size = STARTUP_INTRO_PALETTE_COLOURS * 4u;
    if ((size_t)(end - data) < header_size + palette_size) return false;
    if (data[0] != STARTUP_INTRO_MAGIC_0 || data[1] != STARTUP_INTRO_MAGIC_1 ||
        data[2] != STARTUP_INTRO_MAGIC_2 || data[3] != STARTUP_INTRO_MAGIC_3)
        return false;

    const uint16_t version = startup_intro_read_be16(data + 4u);
    if (version != STARTUP_INTRO_VERSION) return false;

    video->width = startup_intro_read_be16(data + 6u);
    video->height = startup_intro_read_be16(data + 8u);
    video->frame_count = startup_intro_read_be16(data + 10u);
    video->fps_num = startup_intro_read_be16(data + 12u);
    video->fps_den = startup_intro_read_be16(data + 14u);
    if (video->width == 0u || video->height == 0u || video->frame_count == 0u ||
        video->fps_num == 0u || video->fps_den == 0u ||
        video->width > GRAPHICS_LOGICAL_WIDTH ||
        video->height > GRAPHICS_LOGICAL_HEIGHT) {
        return false;
    }

    const size_t pixel_count = (size_t)video->width * (size_t)video->height;
    video->indices = (uint8_t *)calloc(pixel_count, 1u);
    video->delta = (uint8_t *)malloc(pixel_count);
    video->rgba = (uint8_t *)malloc(pixel_count * 4u);
    if (video->indices == NULL || video->delta == NULL || video->rgba == NULL) {
        startup_intro_video_shutdown(video);
        return false;
    }

    video->palette = data + header_size;
    video->record = video->palette + palette_size;
    video->end = end;

    video->tv_renderer = startup_intro_find_renderer("Wii U TV");
    video->drc_renderer = startup_intro_find_renderer("Wii U GamePad");
    video->tv_texture = startup_intro_create_texture(video->tv_renderer,
                                                     video->width,
                                                     video->height);
    video->drc_texture = startup_intro_create_texture(video->drc_renderer,
                                                      video->width,
                                                      video->height);
    if (video->tv_renderer == NULL || video->drc_renderer == NULL ||
        video->tv_texture == NULL || video->drc_texture == NULL) {
        startup_intro_video_shutdown(video);
        return false;
    }
    return true;
}

static bool startup_intro_decode_next(StartupIntroVideo *video)
{
    if (video == NULL || video->record == NULL ||
        video->decoded_frame + 1 >= (int)video->frame_count ||
        video->record + 4u > video->end) {
        return false;
    }

    const uint32_t compressed_size = startup_intro_read_be32(video->record);
    video->record += 4u;
    if (compressed_size == 0u ||
        (size_t)(video->end - video->record) < (size_t)compressed_size) {
        return false;
    }

    const size_t pixel_count = (size_t)video->width * (size_t)video->height;
    uLongf output_size = (uLongf)pixel_count;
    const int status = uncompress(video->delta,
                                  &output_size,
                                  video->record,
                                  (uLong)compressed_size);
    video->record += compressed_size;
    if (status != Z_OK || output_size != (uLongf)pixel_count) return false;

    for (size_t pixel = 0u; pixel < pixel_count; ++pixel)
        video->indices[pixel] ^= video->delta[pixel];
    ++video->decoded_frame;
    return true;
}

static bool startup_intro_decode_to(StartupIntroVideo *video, int frame)
{
    if (video == NULL) return false;
    if (frame < 0) frame = 0;
    if (frame >= (int)video->frame_count) frame = (int)video->frame_count - 1;
    while (video->decoded_frame < frame) {
        if (!startup_intro_decode_next(video)) return false;
    }
    return true;
}

static void startup_intro_render(StartupIntroVideo *video)
{
    if (video == NULL || video->decoded_frame < 0) return;
    const size_t pixel_count = (size_t)video->width * (size_t)video->height;
    for (size_t pixel = 0u; pixel < pixel_count; ++pixel) {
        const uint8_t index = video->indices[pixel];
        const uint8_t *colour = video->palette + (size_t)index * 4u;
        uint8_t *output = video->rgba + pixel * 4u;
        output[0] = colour[0];
        output[1] = colour[1];
        output[2] = colour[2];
        output[3] = colour[3];
    }

    const int pitch = (int)video->width * 4;
    SDL_UpdateTexture(video->tv_texture, NULL, video->rgba, pitch);
    SDL_UpdateTexture(video->drc_texture, NULL, video->rgba, pitch);

    graphics_clear(GRAPHICS_TARGET_BOTH, GRAPHICS_RGB(0, 0, 0));
    const SDL_Rect destination = {
        0, 0, GRAPHICS_LOGICAL_WIDTH, GRAPHICS_LOGICAL_HEIGHT
    };
    SDL_RenderCopy(video->tv_renderer, video->tv_texture, NULL, &destination);
    SDL_RenderCopy(video->drc_renderer, video->drc_texture, NULL, &destination);
    graphics_present(GRAPHICS_TARGET_BOTH);
}

static AXVoice *startup_intro_audio_start(void)
{
    if (!AXIsInit()) return NULL;
    const uint8_t *data = startup_intro_audio_bin;
    const uint32_t byte_size =
        (uint32_t)(startup_intro_audio_bin_end - startup_intro_audio_bin);
    if (byte_size < 2u) return NULL;

    AXVoice *voice = AXAcquireVoice(31u, NULL, NULL);
    if (voice == NULL) return NULL;

    AXVoiceDeviceMixData mix[6];
    memset(mix, 0, sizeof(mix));
    mix[0].bus[0].volume = 0xC000;
    mix[1].bus[0].volume = 0xC000;
    AXVoiceVeData ve = {.volume = 0xC000, .delta = 0};
    AXVoiceOffsets offsets = {
        .dataType = AX_VOICE_FORMAT_LPCM16,
        .loopingEnabled = AX_VOICE_LOOP_DISABLED,
        .loopOffset = 0u,
        .endOffset = byte_size / 2u,
        .currentOffset = 0u,
        .data = data,
    };

    DCFlushRange((void *)data, byte_size);
    AXVoiceBegin(voice);
    AXSetVoiceType(voice, AX_VOICE_TYPE_UNKNOWN);
    AXSetVoiceVe(voice, &ve);
    AXSetVoiceDeviceMix(voice, AX_DEVICE_TYPE_TV, 0, mix);
    AXSetVoiceDeviceMix(voice, AX_DEVICE_TYPE_DRC, 0, mix);
    AXSetVoiceSrcType(voice, AX_VOICE_SRC_TYPE_LINEAR);
    (void)AXSetVoiceSrcRatio(voice, 0.5f);
    AXSetVoiceOffsets(voice, &offsets);
    AXVoiceEnd(voice);
    AXSetVoiceState(voice, AX_VOICE_STATE_PLAYING);
    return voice;
}

static void startup_intro_audio_stop(AXVoice *voice)
{
    if (voice == NULL) return;
    AXSetVoiceState(voice, AX_VOICE_STATE_STOPPED);
    AXFreeVoice(voice);
}

void startup_intro_run(void)
{
    StartupIntroVideo video;
    if (!startup_intro_video_init(&video)) return;

    AXVoice *voice = startup_intro_audio_start();
    frame_clock_reset();

    /* Source: user-supplied PDD intro. The game itself is paced at 60 Hz;
     * the converted stream stores its own frame rate so playback timing stays
     * deterministic. A or Start skips immediately with no added overlay. */
    uint32_t ticks_per_video_frame =
        (60u * (uint32_t)video.fps_den) / (uint32_t)video.fps_num;
    if (ticks_per_video_frame == 0u) ticks_per_video_frame = 1u;
    const uint32_t total_ticks =
        (uint32_t)video.frame_count * ticks_per_video_frame;

    for (uint32_t tick = 0u;
         tick < total_ticks && WHBProcIsRunning();
         ++tick) {
        InputState input;
        input_update(&input);
        if (input_was_pressed(&input, GAME_BUTTON_CONFIRM) ||
            input_was_pressed(&input, GAME_BUTTON_START)) {
            break;
        }

        const int frame = (int)(tick / ticks_per_video_frame);
        if (!startup_intro_decode_to(&video, frame)) break;
        startup_intro_render(&video);
        frame_clock_wait_next();
    }

    startup_intro_audio_stop(voice);
    startup_intro_video_shutdown(&video);
    frame_clock_reset();
}

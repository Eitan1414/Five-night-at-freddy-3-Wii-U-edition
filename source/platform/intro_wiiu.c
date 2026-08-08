#include "platform/intro.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <SDL2/SDL.h>
#include <whb/file.h>
#include <whb/proc.h>
#include <zlib.h>

#define INTRO_MAGIC "F3I1"
#define INTRO_HEADER_SIZE 16u
#define INTRO_LOGICAL_WIDTH 854
#define INTRO_LOGICAL_HEIGHT 480
#define INTRO_MAX_FRAME_BYTES (640u * 360u * 2u)
#define INTRO_MAX_COMPRESSED_FRAME_BYTES (512u * 1024u)

typedef struct IntroRenderer {
    SDL_Window *tv_window;
    SDL_Window *drc_window;
    SDL_Renderer *tv_renderer;
    SDL_Renderer *drc_renderer;
    SDL_Texture *tv_texture;
    SDL_Texture *drc_texture;
} IntroRenderer;

static uint16_t read_be16(const uint8_t *p)
{
    return (uint16_t) (((uint16_t) p[0] << 8) | (uint16_t) p[1]);
}

static uint32_t read_be32(const uint8_t *p)
{
    return ((uint32_t) p[0] << 24) |
           ((uint32_t) p[1] << 16) |
           ((uint32_t) p[2] << 8) |
           (uint32_t) p[3];
}

static SDL_Renderer *create_renderer(SDL_Window *window)
{
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) renderer = SDL_CreateRenderer(window, -1, 0u);
    if (renderer == NULL) return NULL;

    SDL_RenderSetLogicalSize(renderer, INTRO_LOGICAL_WIDTH, INTRO_LOGICAL_HEIGHT);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    return renderer;
}

static void destroy_renderer(IntroRenderer *intro)
{
    if (intro->tv_texture != NULL) SDL_DestroyTexture(intro->tv_texture);
    if (intro->drc_texture != NULL) SDL_DestroyTexture(intro->drc_texture);
    if (intro->tv_renderer != NULL) SDL_DestroyRenderer(intro->tv_renderer);
    if (intro->drc_renderer != NULL) SDL_DestroyRenderer(intro->drc_renderer);
    if (intro->tv_window != NULL) SDL_DestroyWindow(intro->tv_window);
    if (intro->drc_window != NULL) SDL_DestroyWindow(intro->drc_window);
    memset(intro, 0, sizeof(*intro));
}

static bool create_intro_renderer(IntroRenderer *intro,
                                  uint16_t width,
                                  uint16_t height)
{
    memset(intro, 0, sizeof(*intro));

    intro->tv_window = SDL_CreateWindow("FNaF3 Wii U Intro TV",
                                        SDL_WINDOWPOS_UNDEFINED,
                                        SDL_WINDOWPOS_UNDEFINED,
                                        1280,
                                        720,
                                        SDL_WINDOW_SHOWN |
                                        SDL_WINDOW_WIIU_TV_ONLY |
                                        SDL_WINDOW_WIIU_PREVENT_SWAP);
    intro->drc_window = SDL_CreateWindow("FNaF3 Wii U Intro GamePad",
                                         SDL_WINDOWPOS_UNDEFINED,
                                         SDL_WINDOWPOS_UNDEFINED,
                                         854,
                                         480,
                                         SDL_WINDOW_SHOWN |
                                         SDL_WINDOW_WIIU_GAMEPAD_ONLY);
    if (intro->tv_window == NULL || intro->drc_window == NULL) return false;

    intro->tv_renderer = create_renderer(intro->tv_window);
    intro->drc_renderer = create_renderer(intro->drc_window);
    if (intro->tv_renderer == NULL || intro->drc_renderer == NULL) return false;

    intro->tv_texture = SDL_CreateTexture(intro->tv_renderer,
                                          SDL_PIXELFORMAT_RGB565,
                                          SDL_TEXTUREACCESS_STREAMING,
                                          width,
                                          height);
    intro->drc_texture = SDL_CreateTexture(intro->drc_renderer,
                                           SDL_PIXELFORMAT_RGB565,
                                           SDL_TEXTUREACCESS_STREAMING,
                                           width,
                                           height);
    if (intro->tv_texture == NULL || intro->drc_texture == NULL) return false;

    SDL_SetTextureScaleMode(intro->tv_texture, SDL_ScaleModeLinear);
    SDL_SetTextureScaleMode(intro->drc_texture, SDL_ScaleModeLinear);
    return true;
}

static void present_frame(IntroRenderer *intro,
                          const uint8_t *frame,
                          uint16_t width)
{
    SDL_UpdateTexture(intro->tv_texture, NULL, frame, (int) width * 2);
    SDL_UpdateTexture(intro->drc_texture, NULL, frame, (int) width * 2);

    SDL_SetRenderDrawColor(intro->tv_renderer, 0, 0, 0, 255);
    SDL_SetRenderDrawColor(intro->drc_renderer, 0, 0, 0, 255);
    SDL_RenderClear(intro->tv_renderer);
    SDL_RenderClear(intro->drc_renderer);
    SDL_RenderCopy(intro->tv_renderer, intro->tv_texture, NULL, NULL);
    SDL_RenderCopy(intro->drc_renderer, intro->drc_texture, NULL, NULL);

    /* The Wii U TV window defers its GX2 swap; the DRC present completes the
     * synchronized swap for both displays, matching the main renderer. */
    SDL_RenderPresent(intro->tv_renderer);
    SDL_RenderPresent(intro->drc_renderer);
    SDL_PumpEvents();
}

static SDL_AudioDeviceID start_intro_audio(char **owned_audio)
{
    uint32_t audio_size = 0u;
    char *audio = WHBReadWholeFile("intro/intro_audio.bin", &audio_size);
    if (audio == NULL || audio_size < 2u) {
        if (audio != NULL) WHBFreeWholeFile(audio);
        return 0;
    }

    SDL_AudioSpec wanted;
    SDL_zero(wanted);
    wanted.freq = 16000;
    wanted.format = AUDIO_S16MSB;
    wanted.channels = 1;
    wanted.samples = 1024;

    SDL_AudioDeviceID device = SDL_OpenAudioDevice(NULL, 0, &wanted, NULL, 0);
    if (device == 0) {
        WHBFreeWholeFile(audio);
        return 0;
    }

    if (SDL_QueueAudio(device, audio, audio_size & ~1u) != 0) {
        SDL_CloseAudioDevice(device);
        WHBFreeWholeFile(audio);
        return 0;
    }

    *owned_audio = audio;
    SDL_PauseAudioDevice(device, 0);
    return device;
}

bool intro_play(void)
{
    uint32_t packed_size = 0u;
    char *packed_file = WHBReadWholeFile("intro/intro.f3v", &packed_size);
    if (packed_file == NULL || packed_size < INTRO_HEADER_SIZE) {
        if (packed_file != NULL) WHBFreeWholeFile(packed_file);
        return false;
    }

    const uint8_t *packed = (const uint8_t *) packed_file;
    if (memcmp(packed, INTRO_MAGIC, 4u) != 0) {
        WHBFreeWholeFile(packed_file);
        return false;
    }

    const uint16_t width = read_be16(packed + 4u);
    const uint16_t height = read_be16(packed + 6u);
    const uint16_t fps = read_be16(packed + 8u);
    const uint16_t frame_count = read_be16(packed + 10u);
    const uint32_t frame_bytes = read_be32(packed + 12u);

    if (width == 0u || height == 0u || fps == 0u || frame_count == 0u ||
        frame_bytes != (uint32_t) width * (uint32_t) height * 2u ||
        frame_bytes > INTRO_MAX_FRAME_BYTES) {
        WHBFreeWholeFile(packed_file);
        return false;
    }

    uint8_t *frame = (uint8_t *) malloc(frame_bytes);
    uint8_t *decoded = (uint8_t *) malloc(frame_bytes);
    if (frame == NULL || decoded == NULL) {
        free(frame);
        free(decoded);
        WHBFreeWholeFile(packed_file);
        return false;
    }
    memset(frame, 0, frame_bytes);

    if (SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0) {
        free(frame);
        free(decoded);
        WHBFreeWholeFile(packed_file);
        return false;
    }

    IntroRenderer renderer;
    if (!create_intro_renderer(&renderer, width, height)) {
        destroy_renderer(&renderer);
        SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER);
        free(frame);
        free(decoded);
        WHBFreeWholeFile(packed_file);
        return false;
    }

    char *audio_data = NULL;
    SDL_AudioDeviceID audio_device = start_intro_audio(&audio_data);

    const uint8_t *cursor = packed + INTRO_HEADER_SIZE;
    const uint8_t *end = packed + packed_size;
    const uint32_t start_ticks = SDL_GetTicks();
    bool played = false;

    for (uint16_t index = 0u; index < frame_count && WHBProcIsRunning(); ++index) {
        if ((size_t) (end - cursor) < 5u) break;
        const uint8_t frame_type = cursor[0];
        const uint32_t compressed_size = read_be32(cursor + 1u);
        cursor += 5u;

        if (compressed_size == 0u ||
            compressed_size > INTRO_MAX_COMPRESSED_FRAME_BYTES ||
            (size_t) (end - cursor) < compressed_size) {
            break;
        }

        uLongf decoded_size = (uLongf) frame_bytes;
        if (uncompress(decoded,
                       &decoded_size,
                       cursor,
                       (uLong) compressed_size) != Z_OK ||
            decoded_size != frame_bytes) {
            break;
        }
        cursor += compressed_size;

        if (frame_type == 0u) {
            memcpy(frame, decoded, frame_bytes);
        } else if (frame_type == 1u) {
            for (uint32_t byte = 0u; byte < frame_bytes; ++byte)
                frame[byte] ^= decoded[byte];
        } else {
            break;
        }

        present_frame(&renderer, frame, width);
        played = true;

        const uint32_t deadline = start_ticks +
            ((uint32_t) (index + 1u) * 1000u) / (uint32_t) fps;
        const uint32_t now = SDL_GetTicks();
        if (deadline > now) SDL_Delay(deadline - now);
    }

    if (audio_device != 0) {
        SDL_ClearQueuedAudio(audio_device);
        SDL_CloseAudioDevice(audio_device);
    }
    if (audio_data != NULL) WHBFreeWholeFile(audio_data);

    destroy_renderer(&renderer);
    SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER);
    free(frame);
    free(decoded);
    WHBFreeWholeFile(packed_file);
    return played;
}

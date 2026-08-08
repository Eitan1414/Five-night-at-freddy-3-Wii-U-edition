#include "platform/graphics.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include <SDL2/SDL.h>

#define TEXTURE_CACHE_CAPACITY 64u
#define TEXT_RECT_BATCH_CAPACITY 256

typedef struct RenderTarget {
    uint32_t target;
    SDL_Window *window;
    SDL_Renderer *renderer;
} RenderTarget;

typedef struct CachedTexture {
    const void *key;
    uint16_t width;
    uint16_t height;
    SDL_Texture *tv;
    SDL_Texture *gamepad;
} CachedTexture;

static SDL_Window *sTvWindow = NULL;
static SDL_Window *sGamePadWindow = NULL;
static SDL_Renderer *sTvRenderer = NULL;
static SDL_Renderer *sGamePadRenderer = NULL;
static CachedTexture sTextureCache[TEXTURE_CACHE_CAPACITY];
static uint32_t sTextureCacheCount = 0u;
static uint32_t sTvDrawColour = 0u;
static uint32_t sGamePadDrawColour = 0u;
static bool sTvDrawColourValid = false;
static bool sGamePadDrawColourValid = false;

static RenderTarget get_target(uint32_t index)
{
    if (index == 0u) {
        const RenderTarget target = {
            GRAPHICS_TARGET_TV,
            sTvWindow,
            sTvRenderer,
        };
        return target;
    }

    const RenderTarget target = {
        GRAPHICS_TARGET_GAMEPAD,
        sGamePadWindow,
        sGamePadRenderer,
    };
    return target;
}

static void set_draw_colour(SDL_Renderer *renderer, uint32_t colour)
{
    uint32_t *cached_colour = NULL;
    bool *cached_valid = NULL;

    if (renderer == sTvRenderer) {
        cached_colour = &sTvDrawColour;
        cached_valid = &sTvDrawColourValid;
    } else if (renderer == sGamePadRenderer) {
        cached_colour = &sGamePadDrawColour;
        cached_valid = &sGamePadDrawColourValid;
    }

    if (cached_valid != NULL && *cached_valid &&
        cached_colour != NULL && *cached_colour == colour) {
        return;
    }

    const uint8_t red = (uint8_t) ((colour >> 24) & 0xFFu);
    const uint8_t green = (uint8_t) ((colour >> 16) & 0xFFu);
    const uint8_t blue = (uint8_t) ((colour >> 8) & 0xFFu);
    const uint8_t alpha = (uint8_t) (colour & 0xFFu);
    SDL_SetRenderDrawColor(renderer, red, green, blue, alpha);

    if (cached_valid != NULL && cached_colour != NULL) {
        *cached_colour = colour;
        *cached_valid = true;
    }
}

static void fill_rects(uint32_t targets,
                       const SDL_Rect *rectangles,
                       int count,
                       uint32_t colour)
{
    if (rectangles == NULL || count <= 0) {
        return;
    }

    for (uint32_t index = 0u; index < 2u; ++index) {
        const RenderTarget target = get_target(index);
        if ((targets & target.target) == 0u || target.renderer == NULL) {
            continue;
        }
        set_draw_colour(target.renderer, colour);
        SDL_RenderFillRects(target.renderer, rectangles, count);
    }
}

static void destroy_texture_cache(void)
{
    for (uint32_t index = 0u; index < sTextureCacheCount; ++index) {
        if (sTextureCache[index].tv != NULL) {
            SDL_DestroyTexture(sTextureCache[index].tv);
        }
        if (sTextureCache[index].gamepad != NULL) {
            SDL_DestroyTexture(sTextureCache[index].gamepad);
        }
    }

    for (uint32_t index = 0u; index < TEXTURE_CACHE_CAPACITY; ++index) {
        sTextureCache[index].key = NULL;
        sTextureCache[index].width = 0u;
        sTextureCache[index].height = 0u;
        sTextureCache[index].tv = NULL;
        sTextureCache[index].gamepad = NULL;
    }
    sTextureCacheCount = 0u;
}

static CachedTexture *find_cached_texture(const void *key,
                                          uint16_t width,
                                          uint16_t height)
{
    for (uint32_t index = 0u; index < sTextureCacheCount; ++index) {
        CachedTexture *cached = &sTextureCache[index];
        if (cached->key == key && cached->width == width &&
            cached->height == height) {
            return cached;
        }
    }
    return NULL;
}

static SDL_Texture *create_gpu_texture(SDL_Renderer *renderer,
                                       uint16_t width,
                                       uint16_t height,
                                       const uint8_t *pixels)
{
    SDL_Texture *texture = SDL_CreateTexture(renderer,
                                             SDL_PIXELFORMAT_RGBA32,
                                             SDL_TEXTUREACCESS_STATIC,
                                             width,
                                             height);
    if (texture == NULL) {
        return NULL;
    }

    if (SDL_UpdateTexture(texture, NULL, pixels, (int) width * 4) != 0) {
        SDL_DestroyTexture(texture);
        return NULL;
    }

    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    SDL_SetTextureScaleMode(texture, SDL_ScaleModeLinear);
    return texture;
}

static CachedTexture *cache_indexed_texture(const void *key,
                                            uint16_t width,
                                            uint16_t height,
                                            uint8_t transparent_index,
                                            const uint16_t *row_offsets,
                                            const uint8_t *runs,
                                            const uint32_t *palette)
{
    if (sTextureCacheCount >= TEXTURE_CACHE_CAPACITY ||
        sTvRenderer == NULL || sGamePadRenderer == NULL) {
        return NULL;
    }

    const size_t pixel_count = (size_t) width * (size_t) height;
    uint8_t *pixels = (uint8_t *) calloc(pixel_count, 4u);
    if (pixels == NULL) {
        return NULL;
    }

    for (uint32_t source_y = 0u; source_y < height; ++source_y) {
        const uint16_t row_start = row_offsets[source_y];
        const uint16_t row_end = row_offsets[source_y + 1u];
        uint32_t source_x = 0u;

        for (uint16_t offset = row_start;
             offset + 1u < row_end && source_x < width;
             offset += 2u) {
            const uint8_t run_length = runs[offset];
            const uint8_t palette_index = runs[offset + 1u];
            uint32_t next_source_x = source_x + run_length;
            if (next_source_x > width) {
                next_source_x = width;
            }

            if (palette_index != transparent_index) {
                const uint32_t colour = palette[palette_index];
                const uint8_t red = (uint8_t) ((colour >> 24) & 0xFFu);
                const uint8_t green = (uint8_t) ((colour >> 16) & 0xFFu);
                const uint8_t blue = (uint8_t) ((colour >> 8) & 0xFFu);
                const uint8_t alpha = (uint8_t) (colour & 0xFFu);

                for (uint32_t pixel_x = source_x;
                     pixel_x < next_source_x;
                     ++pixel_x) {
                    const size_t pixel_offset =
                        ((size_t) source_y * width + pixel_x) * 4u;
                    pixels[pixel_offset + 0u] = red;
                    pixels[pixel_offset + 1u] = green;
                    pixels[pixel_offset + 2u] = blue;
                    pixels[pixel_offset + 3u] = alpha;
                }
            }

            source_x = next_source_x;
        }
    }

    SDL_Texture *tv = create_gpu_texture(sTvRenderer, width, height, pixels);
    SDL_Texture *gamepad = create_gpu_texture(sGamePadRenderer,
                                              width,
                                              height,
                                              pixels);
    free(pixels);

    if (tv == NULL || gamepad == NULL) {
        if (tv != NULL) {
            SDL_DestroyTexture(tv);
        }
        if (gamepad != NULL) {
            SDL_DestroyTexture(gamepad);
        }
        return NULL;
    }

    CachedTexture *cached = &sTextureCache[sTextureCacheCount++];
    cached->key = key;
    cached->width = width;
    cached->height = height;
    cached->tv = tv;
    cached->gamepad = gamepad;
    return cached;
}

static void draw_rle_fallback(uint32_t targets,
                              int x,
                              int y,
                              int width,
                              int height,
                              uint16_t source_width,
                              uint16_t source_height,
                              uint8_t transparent_index,
                              const uint16_t *row_offsets,
                              const uint8_t *runs,
                              const uint32_t *palette)
{
    for (uint32_t source_y = 0u; source_y < source_height; ++source_y) {
        const int destination_y0 = y +
            (int) ((source_y * (uint32_t) height) / source_height);
        const int destination_y1 = y +
            (int) (((source_y + 1u) * (uint32_t) height) / source_height);
        const int destination_height = destination_y1 - destination_y0;
        if (destination_height <= 0) {
            continue;
        }

        const uint16_t row_start = row_offsets[source_y];
        const uint16_t row_end = row_offsets[source_y + 1u];
        uint32_t source_x = 0u;

        for (uint16_t offset = row_start; offset + 1u < row_end; offset += 2u) {
            const uint8_t run_length = runs[offset];
            const uint8_t palette_index = runs[offset + 1u];
            const uint32_t next_source_x = source_x + run_length;

            if (palette_index != transparent_index) {
                const int destination_x0 = x +
                    (int) ((source_x * (uint32_t) width) / source_width);
                const int destination_x1 = x +
                    (int) ((next_source_x * (uint32_t) width) / source_width);
                const int destination_width = destination_x1 - destination_x0;
                if (destination_width > 0) {
                    graphics_draw_rect(targets,
                                       destination_x0,
                                       destination_y0,
                                       destination_width,
                                       destination_height,
                                       palette[palette_index]);
                }
            }

            source_x = next_source_x;
        }
    }
}

static bool get_glyph(char character, uint8_t rows[7])
{
#define GLYPH(a, b, c, d, e, f, g) \
    do {                              \
        rows[0] = (a);                \
        rows[1] = (b);                \
        rows[2] = (c);                \
        rows[3] = (d);                \
        rows[4] = (e);                \
        rows[5] = (f);                \
        rows[6] = (g);                \
    } while (0)

    switch (character) {
        case 'A': GLYPH(0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11); break;
        case 'B': GLYPH(0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E); break;
        case 'C': GLYPH(0x0F, 0x10, 0x10, 0x10, 0x10, 0x10, 0x0F); break;
        case 'D': GLYPH(0x1E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1E); break;
        case 'E': GLYPH(0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F); break;
        case 'F': GLYPH(0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10); break;
        case 'G': GLYPH(0x0F, 0x10, 0x10, 0x17, 0x11, 0x11, 0x0F); break;
        case 'H': GLYPH(0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11); break;
        case 'I': GLYPH(0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x1F); break;
        case 'J': GLYPH(0x01, 0x01, 0x01, 0x01, 0x11, 0x11, 0x0E); break;
        case 'K': GLYPH(0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11); break;
        case 'L': GLYPH(0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F); break;
        case 'M': GLYPH(0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11); break;
        case 'N': GLYPH(0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11); break;
        case 'O': GLYPH(0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E); break;
        case 'P': GLYPH(0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10); break;
        case 'Q': GLYPH(0x0E, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0D); break;
        case 'R': GLYPH(0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11); break;
        case 'S': GLYPH(0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E); break;
        case 'T': GLYPH(0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04); break;
        case 'U': GLYPH(0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E); break;
        case 'V': GLYPH(0x11, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x04); break;
        case 'W': GLYPH(0x11, 0x11, 0x11, 0x15, 0x15, 0x15, 0x0A); break;
        case 'X': GLYPH(0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11); break;
        case 'Y': GLYPH(0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x04); break;
        case 'Z': GLYPH(0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F); break;
        case '0': GLYPH(0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E); break;
        case '1': GLYPH(0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E); break;
        case '2': GLYPH(0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F); break;
        case '3': GLYPH(0x1E, 0x01, 0x01, 0x0E, 0x01, 0x01, 0x1E); break;
        case '4': GLYPH(0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02); break;
        case '5': GLYPH(0x1F, 0x10, 0x10, 0x1E, 0x01, 0x01, 0x1E); break;
        case '6': GLYPH(0x0E, 0x10, 0x10, 0x1E, 0x11, 0x11, 0x0E); break;
        case '7': GLYPH(0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08); break;
        case '8': GLYPH(0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E); break;
        case '9': GLYPH(0x0E, 0x11, 0x11, 0x0F, 0x01, 0x01, 0x0E); break;
        case '-': GLYPH(0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00); break;
        case ':': GLYPH(0x00, 0x04, 0x04, 0x00, 0x04, 0x04, 0x00); break;
        case '.': GLYPH(0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C); break;
        case '/': GLYPH(0x01, 0x02, 0x02, 0x04, 0x08, 0x08, 0x10); break;
        case '>': GLYPH(0x10, 0x08, 0x04, 0x02, 0x04, 0x08, 0x10); break;
        case '<': GLYPH(0x01, 0x02, 0x04, 0x08, 0x04, 0x02, 0x01); break;
        case ' ': GLYPH(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00); break;
        default: return false;
    }

#undef GLYPH
    return true;
}

static SDL_Renderer *create_renderer(SDL_Window *window)
{
    SDL_Renderer *renderer = SDL_CreateRenderer(window,
                                                -1,
                                                SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        renderer = SDL_CreateRenderer(window, -1, 0u);
    }
    if (renderer == NULL) {
        return NULL;
    }

    SDL_RenderSetLogicalSize(renderer,
                             GRAPHICS_LOGICAL_WIDTH,
                             GRAPHICS_LOGICAL_HEIGHT);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    return renderer;
}

bool graphics_init(void)
{
    if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) {
        return false;
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

    sTvWindow = SDL_CreateWindow("FNaF3 Wii U TV",
                                 SDL_WINDOWPOS_UNDEFINED,
                                 SDL_WINDOWPOS_UNDEFINED,
                                 1280,
                                 720,
                                 SDL_WINDOW_SHOWN |
                                 SDL_WINDOW_WIIU_TV_ONLY |
                                 SDL_WINDOW_WIIU_PREVENT_SWAP);
    sGamePadWindow = SDL_CreateWindow("FNaF3 Wii U GamePad",
                                      SDL_WINDOWPOS_UNDEFINED,
                                      SDL_WINDOWPOS_UNDEFINED,
                                      854,
                                      480,
                                      SDL_WINDOW_SHOWN |
                                      SDL_WINDOW_WIIU_GAMEPAD_ONLY);
    if (sTvWindow == NULL || sGamePadWindow == NULL) {
        graphics_shutdown();
        return false;
    }

    sTvRenderer = create_renderer(sTvWindow);
    sGamePadRenderer = create_renderer(sGamePadWindow);
    if (sTvRenderer == NULL || sGamePadRenderer == NULL) {
        graphics_shutdown();
        return false;
    }

    sTvDrawColourValid = false;
    sGamePadDrawColourValid = false;
    graphics_clear(GRAPHICS_TARGET_BOTH, GRAPHICS_RGB(0, 0, 0));
    graphics_present(GRAPHICS_TARGET_BOTH);
    return true;
}

void graphics_shutdown(void)
{
    destroy_texture_cache();

    if (sTvRenderer != NULL) {
        SDL_DestroyRenderer(sTvRenderer);
        sTvRenderer = NULL;
    }
    if (sGamePadRenderer != NULL) {
        SDL_DestroyRenderer(sGamePadRenderer);
        sGamePadRenderer = NULL;
    }
    if (sTvWindow != NULL) {
        SDL_DestroyWindow(sTvWindow);
        sTvWindow = NULL;
    }
    if (sGamePadWindow != NULL) {
        SDL_DestroyWindow(sGamePadWindow);
        sGamePadWindow = NULL;
    }

    sTvDrawColourValid = false;
    sGamePadDrawColourValid = false;
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

void graphics_clear(uint32_t targets, uint32_t colour)
{
    for (uint32_t index = 0u; index < 2u; ++index) {
        const RenderTarget target = get_target(index);
        if ((targets & target.target) == 0u || target.renderer == NULL) {
            continue;
        }
        set_draw_colour(target.renderer, colour);
        SDL_RenderClear(target.renderer);
    }
}

void graphics_present(uint32_t targets)
{
    (void) targets;

    /*
     * The TV window defers the GX2 scan-buffer swap. Presenting the GamePad
     * window second copies its buffer and performs one synchronized swap for
     * both displays.
     */
    if (sTvRenderer != NULL) {
        SDL_RenderPresent(sTvRenderer);
    }
    if (sGamePadRenderer != NULL) {
        SDL_RenderPresent(sGamePadRenderer);
    }
    SDL_PumpEvents();
}

void graphics_draw_rect(uint32_t targets,
                        int x,
                        int y,
                        int width,
                        int height,
                        uint32_t colour)
{
    if (width <= 0 || height <= 0) {
        return;
    }

    const SDL_Rect rectangle = {x, y, width, height};
    fill_rects(targets, &rectangle, 1, colour);
}

void graphics_draw_frame(uint32_t targets,
                         int x,
                         int y,
                         int width,
                         int height,
                         int thickness,
                         uint32_t colour)
{
    if (width <= 0 || height <= 0 || thickness <= 0) {
        return;
    }

    const SDL_Rect rectangles[4] = {
        {x, y, width, thickness},
        {x, y + height - thickness, width, thickness},
        {x, y, thickness, height},
        {x + width - thickness, y, thickness, height},
    };
    fill_rects(targets, rectangles, 4, colour);
}

void graphics_draw_ellipse(uint32_t targets,
                           int centre_x,
                           int centre_y,
                           int radius_x,
                           int radius_y,
                           uint32_t colour)
{
    if (radius_x <= 0 || radius_y <= 0) {
        return;
    }

    const int64_t radius_x_squared = (int64_t) radius_x * radius_x;
    const int64_t radius_y_squared = (int64_t) radius_y * radius_y;
    const int64_t limit = radius_x_squared * radius_y_squared;
    int ellipse_x = radius_x;

    /* The maximum valid X only decreases as Y moves away from the centre.
     * Reusing it makes this O(radius_x + radius_y) instead of scanning the
     * entire bounding rectangle for every ellipse. */
    for (int ellipse_y = 0; ellipse_y <= radius_y; ++ellipse_y) {
        while (ellipse_x > 0) {
            const int64_t value =
                (int64_t) ellipse_x * ellipse_x * radius_y_squared +
                (int64_t) ellipse_y * ellipse_y * radius_x_squared;
            if (value <= limit) {
                break;
            }
            --ellipse_x;
        }

        const int row_width = ellipse_x * 2 + 1;
        graphics_draw_rect(targets,
                           centre_x - ellipse_x,
                           centre_y + ellipse_y,
                           row_width,
                           1,
                           colour);
        if (ellipse_y != 0) {
            graphics_draw_rect(targets,
                               centre_x - ellipse_x,
                               centre_y - ellipse_y,
                               row_width,
                               1,
                               colour);
        }
    }
}

void graphics_draw_text(uint32_t targets,
                        int x,
                        int y,
                        int scale,
                        const char *text,
                        uint32_t colour)
{
    if (text == NULL || scale <= 0) {
        return;
    }

    int cursor_x = x;
    uint8_t rows[7];
    SDL_Rect rectangles[TEXT_RECT_BATCH_CAPACITY];
    int rectangle_count = 0;

    while (*text != '\0') {
        if (*text == '\n') {
            cursor_x = x;
            y += 9 * scale;
            ++text;
            continue;
        }

        if (get_glyph(*text, rows)) {
            for (int row = 0; row < 7; ++row) {
                for (int column = 0; column < 5; ++column) {
                    if ((rows[row] & (1u << (4 - column))) == 0u) {
                        continue;
                    }

                    rectangles[rectangle_count++] = (SDL_Rect) {
                        cursor_x + column * scale,
                        y + row * scale,
                        scale,
                        scale,
                    };
                    if (rectangle_count == TEXT_RECT_BATCH_CAPACITY) {
                        fill_rects(targets,
                                   rectangles,
                                   rectangle_count,
                                   colour);
                        rectangle_count = 0;
                    }
                }
            }
        }

        cursor_x += 6 * scale;
        ++text;
    }

    fill_rects(targets, rectangles, rectangle_count, colour);
}

void graphics_draw_indexed_rle(uint32_t targets,
                               int x,
                               int y,
                               int width,
                               int height,
                               const void *cache_key,
                               uint16_t source_width,
                               uint16_t source_height,
                               uint8_t transparent_index,
                               const uint16_t *row_offsets,
                               const uint8_t *runs,
                               const uint32_t *palette)
{
    if (cache_key == NULL || source_width == 0u || source_height == 0u ||
        row_offsets == NULL || runs == NULL || palette == NULL ||
        width <= 0 || height <= 0) {
        return;
    }

    CachedTexture *cached = find_cached_texture(cache_key,
                                                source_width,
                                                source_height);
    if (cached == NULL) {
        cached = cache_indexed_texture(cache_key,
                                       source_width,
                                       source_height,
                                       transparent_index,
                                       row_offsets,
                                       runs,
                                       palette);
    }

    if (cached == NULL) {
        draw_rle_fallback(targets,
                          x,
                          y,
                          width,
                          height,
                          source_width,
                          source_height,
                          transparent_index,
                          row_offsets,
                          runs,
                          palette);
        return;
    }

    const SDL_Rect destination = {x, y, width, height};
    if ((targets & GRAPHICS_TARGET_TV) != 0u &&
        sTvRenderer != NULL && cached->tv != NULL) {
        SDL_RenderCopy(sTvRenderer, cached->tv, NULL, &destination);
    }
    if ((targets & GRAPHICS_TARGET_GAMEPAD) != 0u &&
        sGamePadRenderer != NULL && cached->gamepad != NULL) {
        SDL_RenderCopy(sGamePadRenderer, cached->gamepad, NULL, &destination);
    }
}

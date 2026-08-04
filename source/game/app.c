#include "game/app.h"

#include <stdio.h>

#include "platform/graphics.h"
#include "renderer/sprite.h"

#define WARNING_DURATION_FRAMES 180u
#define TITLE_GLITCH_INTERVAL 180u
#define OFFICE_EFFECT_INTERVAL 30u

static const uint8_t kFanPixels[] = {
    0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,
    0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,
    0,0,0,1,1,1,0,1,0,1,1,1,0,0,0,
    0,0,1,1,1,0,0,1,0,0,1,1,1,0,0,
    0,1,1,1,0,0,0,1,0,0,0,1,1,1,0,
    0,1,1,0,0,0,0,1,0,0,0,0,1,1,0,
    1,1,0,0,0,0,1,1,1,0,0,0,0,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,0,0,0,0,1,1,1,0,0,0,0,1,1,
    0,1,1,0,0,0,0,1,0,0,0,0,1,1,0,
    0,1,1,1,0,0,0,1,0,0,0,1,1,1,0,
    0,0,1,1,1,0,0,1,0,0,1,1,1,0,0,
    0,0,0,1,1,1,0,1,0,1,1,1,0,0,0,
    0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,
    0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,
};

static const SpriteMask kFanSprite = {
    15u,
    15u,
    kFanPixels,
};

static uint32_t next_random(App *app)
{
    app->effect_seed = app->effect_seed * 1664525u + 1013904223u;
    return app->effect_seed;
}

static int clamp_int(int value, int minimum, int maximum)
{
    if (value < minimum) {
        return minimum;
    }
    if (value > maximum) {
        return maximum;
    }
    return value;
}

static void change_screen(App *app, AppScreen screen)
{
    app->screen = screen;
    app->state_frames = 0u;
    app->needs_redraw = true;
}

static void draw_scanlines(uint32_t targets, uint32_t seed)
{
    for (int y = 0; y < GRAPHICS_LOGICAL_HEIGHT; y += 8) {
        const uint32_t shade = 4u + ((seed + (uint32_t) y) % 8u);
        graphics_draw_rect(targets,
                           0,
                           y,
                           GRAPHICS_LOGICAL_WIDTH,
                           1,
                           GRAPHICS_RGB(shade, shade + 5u, shade));
    }
}

static void draw_warning(const App *app)
{
    uint8_t intensity = 255u;
    const uint32_t fade_start = WARNING_DURATION_FRAMES - 48u;

    if (app->state_frames > fade_start) {
        const uint32_t remaining = WARNING_DURATION_FRAMES - app->state_frames;
        intensity = (uint8_t) ((remaining * 255u) / 48u);
    }

    const uint32_t red = GRAPHICS_RGB(intensity,
                                      intensity / 8u,
                                      intensity / 12u);
    const uint32_t white = GRAPHICS_RGB(intensity, intensity, intensity);

    graphics_clear(GRAPHICS_TARGET_BOTH, GRAPHICS_RGB(0, 0, 0));
    graphics_draw_frame(GRAPHICS_TARGET_BOTH, 74, 76, 706, 326, 4, red);
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 263, 111, 7, "WARNING", red);
    graphics_draw_text(GRAPHICS_TARGET_BOTH,
                       142,
                       212,
                       3,
                       "FLASHING LIGHTS",
                       white);
    graphics_draw_text(GRAPHICS_TARGET_BOTH,
                       214,
                       258,
                       3,
                       "LOUD NOISES",
                       white);
    graphics_draw_text(GRAPHICS_TARGET_BOTH,
                       214,
                       304,
                       3,
                       "JUMPSCARES",
                       white);
    graphics_draw_text(GRAPHICS_TARGET_BOTH,
                       268,
                       365,
                       2,
                       "PRESS A TO SKIP",
                       GRAPHICS_RGB(intensity / 2u,
                                    intensity,
                                    intensity / 2u));
    graphics_present(GRAPHICS_TARGET_BOTH);
}

static void draw_title_character(uint32_t targets, uint32_t seed)
{
    const int glitch = (int) ((seed >> 9) % 9u) - 4;
    const uint32_t dark = GRAPHICS_RGB(14, 35, 17);
    const uint32_t suit = GRAPHICS_RGB(36, 72, 37);
    const uint32_t light = GRAPHICS_RGB(79, 116, 68);
    const uint32_t eye = GRAPHICS_RGB(220, 255, 211);

    graphics_draw_ellipse(targets, 667 + glitch, 260, 145, 183, dark);
    graphics_draw_rect(targets, 573 + glitch, 76, 43, 130, suit);
    graphics_draw_rect(targets, 713 + glitch, 58, 39, 151, suit);
    graphics_draw_ellipse(targets, 663 + glitch, 245, 112, 132, suit);
    graphics_draw_ellipse(targets, 663 + glitch, 299, 86, 58, dark);
    graphics_draw_rect(targets, 590 + glitch, 219, 146, 17, light);
    graphics_draw_rect(targets, 596 + glitch, 335, 137, 16, light);
    graphics_draw_rect(targets, 604 + glitch, 359, 122, 9, dark);
    graphics_draw_rect(targets, 616 + glitch, 231, 36, 17, eye);
    graphics_draw_rect(targets, 689 + glitch, 231, 33, 17, eye);
    graphics_draw_rect(targets, 627 + glitch, 236, 11, 8, GRAPHICS_RGB(6, 8, 5));
    graphics_draw_rect(targets, 700 + glitch, 236, 10, 8, GRAPHICS_RGB(6, 8, 5));
}

static void draw_title(const App *app)
{
    static const char *const menu_items[] = {
        "NEW GAME",
        "LOAD GAME",
        "EXTRAS",
    };

    graphics_clear(GRAPHICS_TARGET_BOTH, GRAPHICS_RGB(1, 4, 2));
    graphics_draw_rect(GRAPHICS_TARGET_BOTH,
                       0,
                       0,
                       410,
                       GRAPHICS_LOGICAL_HEIGHT,
                       GRAPHICS_RGB(2, 7, 4));
    graphics_draw_rect(GRAPHICS_TARGET_BOTH,
                       405,
                       0,
                       4,
                       GRAPHICS_LOGICAL_HEIGHT,
                       GRAPHICS_RGB(18, 44, 24));

    graphics_draw_text(GRAPHICS_TARGET_BOTH, 43, 48, 7, "F", GRAPHICS_RGB(206, 231, 207));
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 43, 120, 7, "N", GRAPHICS_RGB(206, 231, 207));
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 43, 192, 7, "A", GRAPHICS_RGB(206, 231, 207));
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 43, 264, 7, "F", GRAPHICS_RGB(206, 231, 207));
    graphics_draw_text(GRAPHICS_TARGET_BOTH, 43, 350, 8, "3", GRAPHICS_RGB(89, 151, 83));

    graphics_draw_text(GRAPHICS_TARGET_BOTH,
                       122,
                       72,
                       2,
                       "FIVE NIGHTS AT",
                       GRAPHICS_RGB(113, 151, 111));
    graphics_draw_text(GRAPHICS_TARGET_BOTH,
                       122,
                       93,
                       2,
                       "FREDDYS 3",
                       GRAPHICS_RGB(113, 151, 111));
    graphics_draw_text(GRAPHICS_TARGET_BOTH,
                       122,
                       125,
                       1,
                       "WII U PORT - PHASE 1",
                       GRAPHICS_RGB(61, 95, 66));

    for (int item = 0; item < 3; ++item) {
        const int y = 284 + item * 48;
        if (item == app->selected_item) {
            graphics_draw_rect(GRAPHICS_TARGET_BOTH,
                               108,
                               y - 10,
                               255,
                               36,
                               GRAPHICS_RGB(31, 72, 37));
            graphics_draw_rect(GRAPHICS_TARGET_BOTH,
                               108,
                               y - 10,
                               5,
                               36,
                               GRAPHICS_RGB(129, 207, 121));
            graphics_draw_text(GRAPHICS_TARGET_BOTH,
                               126,
                               y,
                               3,
                               menu_items[item],
                               GRAPHICS_RGB(226, 244, 225));
        } else {
            graphics_draw_text(GRAPHICS_TARGET_BOTH,
                               126,
                               y,
                               3,
                               menu_items[item],
                               GRAPHICS_RGB(119, 142, 120));
        }
    }

    graphics_draw_text(GRAPHICS_TARGET_BOTH,
                       116,
                       447,
                       1,
                       "UP DOWN: SELECT   A: CONFIRM",
                       GRAPHICS_RGB(90, 126, 93));

    draw_title_character(GRAPHICS_TARGET_BOTH, app->effect_seed);
    draw_scanlines(GRAPHICS_TARGET_BOTH, app->effect_seed);

    if ((app->effect_seed & 0x07u) == 0u) {
        const int offset_y = 115 + (int) ((app->effect_seed >> 8) % 220u);
        graphics_draw_rect(GRAPHICS_TARGET_BOTH,
                           480,
                           offset_y,
                           330,
                           4,
                           GRAPHICS_RGB(23, 87, 42));
        graphics_draw_rect(GRAPHICS_TARGET_BOTH,
                           523,
                           offset_y + 7,
                           262,
                           2,
                           GRAPHICS_RGB(78, 129, 70));
    }

    graphics_present(GRAPHICS_TARGET_BOTH);
}

static void draw_office_tv(const App *app)
{
    const int offset = app->office_pan;

    graphics_clear(GRAPHICS_TARGET_TV, GRAPHICS_RGB(3, 6, 4));
    graphics_draw_rect(GRAPHICS_TARGET_TV,
                       -170 + offset,
                       0,
                       1194,
                       480,
                       GRAPHICS_RGB(11, 18, 12));
    graphics_draw_rect(GRAPHICS_TARGET_TV,
                       -100 + offset,
                       48,
                       1050,
                       270,
                       GRAPHICS_RGB(19, 27, 19));
    graphics_draw_frame(GRAPHICS_TARGET_TV,
                        185 + offset,
                        75,
                        260,
                        208,
                        8,
                        GRAPHICS_RGB(52, 66, 49));
    graphics_draw_rect(GRAPHICS_TARGET_TV,
                       199 + offset,
                       89,
                       232,
                       180,
                       GRAPHICS_RGB(5, 9, 6));
    graphics_draw_text(GRAPHICS_TARGET_TV,
                       265 + offset,
                       157,
                       3,
                       "HALL",
                       GRAPHICS_RGB(44, 61, 42));
    graphics_draw_rect(GRAPHICS_TARGET_TV,
                       520 + offset,
                       71,
                       196,
                       145,
                       GRAPHICS_RGB(47, 52, 42));
    graphics_draw_frame(GRAPHICS_TARGET_TV,
                        520 + offset,
                        71,
                        196,
                        145,
                        5,
                        GRAPHICS_RGB(78, 86, 67));
    graphics_draw_text(GRAPHICS_TARGET_TV,
                       562 + offset,
                       130,
                       2,
                       "CAM MAP",
                       GRAPHICS_RGB(24, 31, 24));
    graphics_draw_rect(GRAPHICS_TARGET_TV,
                       -80 + offset,
                       324,
                       1110,
                       156,
                       GRAPHICS_RGB(31, 29, 24));
    graphics_draw_rect(GRAPHICS_TARGET_TV,
                       130 + offset,
                       350,
                       650,
                       57,
                       GRAPHICS_RGB(52, 48, 38));
    graphics_draw_rect(GRAPHICS_TARGET_TV,
                       154 + offset,
                       407,
                       602,
                       23,
                       GRAPHICS_RGB(19, 17, 14));

    sprite_draw_mask(GRAPHICS_TARGET_TV,
                     422 + offset,
                     286,
                     4,
                     &kFanSprite,
                     GRAPHICS_RGB(77, 88, 73));
    graphics_draw_rect(GRAPHICS_TARGET_TV,
                       449 + offset,
                       345,
                       7,
                       22,
                       GRAPHICS_RGB(77, 88, 73));
    graphics_draw_rect(GRAPHICS_TARGET_TV,
                       430 + offset,
                       367,
                       45,
                       7,
                       GRAPHICS_RGB(77, 88, 73));
    graphics_draw_text(GRAPHICS_TARGET_TV,
                       26,
                       24,
                       2,
                       "OFFICE SYSTEM TEST",
                       GRAPHICS_RGB(118, 148, 115));
    graphics_draw_text(GRAPHICS_TARGET_TV,
                       26,
                       451,
                       1,
                       "LEFT RIGHT: LOOK   X/Y: CAMERA PANEL   B: MENU",
                       GRAPHICS_RGB(96, 123, 94));
    draw_scanlines(GRAPHICS_TARGET_TV, app->effect_seed);
}

static void draw_camera_map_gamepad(void)
{
    graphics_clear(GRAPHICS_TARGET_GAMEPAD, GRAPHICS_RGB(3, 10, 6));
    graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                       29,
                       24,
                       3,
                       "CAMERA SYSTEM",
                       GRAPHICS_RGB(160, 205, 156));
    graphics_draw_frame(GRAPHICS_TARGET_GAMEPAD,
                        33,
                        77,
                        788,
                        338,
                        4,
                        GRAPHICS_RGB(52, 108, 58));
    graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD,
                       77,
                       122,
                       203,
                       95,
                       GRAPHICS_RGB(18, 43, 24));
    graphics_draw_frame(GRAPHICS_TARGET_GAMEPAD,
                        77,
                        122,
                        203,
                        95,
                        3,
                        GRAPHICS_RGB(76, 145, 78));
    graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                       126,
                       157,
                       3,
                       "CAM 01",
                       GRAPHICS_RGB(209, 242, 205));
    graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD,
                       323,
                       122,
                       203,
                       95,
                       GRAPHICS_RGB(10, 25, 15));
    graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                       372,
                       157,
                       3,
                       "CAM 02",
                       GRAPHICS_RGB(75, 102, 75));
    graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD,
                       569,
                       122,
                       203,
                       95,
                       GRAPHICS_RGB(10, 25, 15));
    graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                       618,
                       157,
                       3,
                       "CAM 03",
                       GRAPHICS_RGB(75, 102, 75));
    graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                       87,
                       281,
                       2,
                       "CAM 01 SIGNAL ONLINE",
                       GRAPHICS_RGB(124, 175, 122));
    graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                       87,
                       319,
                       2,
                       "VIDEO FEED COMING NEXT",
                       GRAPHICS_RGB(83, 122, 83));
    graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                       87,
                       373,
                       2,
                       "X/Y: CLOSE PANEL",
                       GRAPHICS_RGB(142, 190, 139));
    draw_scanlines(GRAPHICS_TARGET_GAMEPAD, 0xCA010001u);
}

static void draw_office_status_gamepad(const App *app)
{
    char pan_text[48];
    snprintf(pan_text,
             sizeof(pan_text),
             "VIEW OFFSET: %d",
             app->office_pan);

    graphics_clear(GRAPHICS_TARGET_GAMEPAD, GRAPHICS_RGB(4, 7, 5));
    graphics_draw_frame(GRAPHICS_TARGET_GAMEPAD,
                        48,
                        53,
                        758,
                        374,
                        4,
                        GRAPHICS_RGB(39, 73, 43));
    graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                       91,
                       97,
                       4,
                       "OFFICE CONTROL",
                       GRAPHICS_RGB(154, 188, 150));
    graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                       91,
                       188,
                       3,
                       pan_text,
                       GRAPHICS_RGB(109, 148, 106));
    graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                       91,
                       250,
                       3,
                       "X/Y: OPEN CAMERAS",
                       GRAPHICS_RGB(133, 178, 129));
    graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                       91,
                       312,
                       2,
                       "THIS IS THE FIRST PLAYABLE LOOP",
                       GRAPHICS_RGB(76, 103, 75));
    draw_scanlines(GRAPHICS_TARGET_GAMEPAD, 0x0FF1CE01u);
}

static void draw_office(const App *app)
{
    draw_office_tv(app);

    if (app->camera_panel_open) {
        draw_camera_map_gamepad();
    } else {
        draw_office_status_gamepad(app);
    }

    graphics_present(GRAPHICS_TARGET_BOTH);
}

static void draw_placeholder(const App *app)
{
    static const char *const headings[] = {
        "NEW GAME",
        "LOAD GAME",
        "EXTRAS",
    };
    static const char *const details[] = {
        "OFFICE TEST AVAILABLE",
        "SAVE SYSTEM NOT PORTED YET",
        "EXTRAS NOT PORTED YET",
    };

    graphics_clear(GRAPHICS_TARGET_BOTH, GRAPHICS_RGB(1, 5, 3));
    graphics_draw_frame(GRAPHICS_TARGET_BOTH,
                        61,
                        73,
                        732,
                        334,
                        3,
                        GRAPHICS_RGB(36, 91, 44));
    graphics_draw_text(GRAPHICS_TARGET_BOTH,
                       128,
                       127,
                       6,
                       headings[app->placeholder_item],
                       GRAPHICS_RGB(203, 235, 202));
    graphics_draw_text(GRAPHICS_TARGET_BOTH,
                       145,
                       243,
                       3,
                       details[app->placeholder_item],
                       GRAPHICS_RGB(98, 151, 100));
    graphics_draw_text(GRAPHICS_TARGET_BOTH,
                       238,
                       335,
                       2,
                       "PRESS B TO RETURN",
                       GRAPHICS_RGB(128, 186, 128));
    draw_scanlines(GRAPHICS_TARGET_BOTH,
                   0xBACC0000u + (uint32_t) app->placeholder_item);
    graphics_present(GRAPHICS_TARGET_BOTH);
}

void app_init(App *app)
{
    if (app == NULL) {
        return;
    }

    app->screen = APP_SCREEN_WARNING;
    app->selected_item = 0;
    app->placeholder_item = 0;
    app->office_pan = 0;
    app->camera_panel_open = false;
    app->needs_redraw = true;
    app->state_frames = 0u;
    app->effect_seed = 0xF3A30001u;
}

void app_update(App *app, const InputState *input)
{
    if (app == NULL || input == NULL) {
        return;
    }

    ++app->state_frames;

    switch (app->screen) {
        case APP_SCREEN_WARNING:
            if (input_was_pressed(input, GAME_BUTTON_CONFIRM) ||
                input_was_pressed(input, GAME_BUTTON_START) ||
                app->state_frames >= WARNING_DURATION_FRAMES) {
                next_random(app);
                change_screen(app, APP_SCREEN_TITLE);
            } else if ((app->state_frames % 4u) == 0u) {
                app->needs_redraw = true;
            }
            break;

        case APP_SCREEN_TITLE:
            if (input_was_pressed(input, GAME_BUTTON_UP)) {
                app->selected_item = (app->selected_item + 2) % 3;
                app->needs_redraw = true;
            }
            if (input_was_pressed(input, GAME_BUTTON_DOWN)) {
                app->selected_item = (app->selected_item + 1) % 3;
                app->needs_redraw = true;
            }
            if (input_was_pressed(input, GAME_BUTTON_CONFIRM) ||
                input_was_pressed(input, GAME_BUTTON_START)) {
                if (app->selected_item == 0) {
                    app->office_pan = 0;
                    app->camera_panel_open = false;
                    change_screen(app, APP_SCREEN_OFFICE_TEST);
                } else {
                    app->placeholder_item = app->selected_item;
                    change_screen(app, APP_SCREEN_PLACEHOLDER);
                }
            } else if (app->state_frames >= TITLE_GLITCH_INTERVAL) {
                next_random(app);
                app->state_frames = 0u;
                app->needs_redraw = true;
            }
            break;

        case APP_SCREEN_OFFICE_TEST: {
            bool changed = false;

            if (input_was_pressed(input, GAME_BUTTON_BACK)) {
                next_random(app);
                change_screen(app, APP_SCREEN_TITLE);
                break;
            }

            if (input_was_pressed(input, GAME_BUTTON_PANEL)) {
                app->camera_panel_open = !app->camera_panel_open;
                changed = true;
            }

            if (!app->camera_panel_open) {
                int new_pan = app->office_pan;
                if (input_is_held(input, GAME_BUTTON_LEFT)) {
                    new_pan -= 4;
                }
                if (input_is_held(input, GAME_BUTTON_RIGHT)) {
                    new_pan += 4;
                }
                new_pan = clamp_int(new_pan, -140, 140);
                if (new_pan != app->office_pan) {
                    app->office_pan = new_pan;
                    changed = true;
                }
            }

            if (changed || (app->state_frames % OFFICE_EFFECT_INTERVAL) == 0u) {
                next_random(app);
                app->needs_redraw = true;
            }
            break;
        }

        case APP_SCREEN_PLACEHOLDER:
            if (input_was_pressed(input, GAME_BUTTON_BACK)) {
                next_random(app);
                change_screen(app, APP_SCREEN_TITLE);
            }
            break;
    }
}

void app_render(App *app)
{
    if (app == NULL || !app->needs_redraw) {
        return;
    }

    switch (app->screen) {
        case APP_SCREEN_WARNING:
            draw_warning(app);
            break;
        case APP_SCREEN_TITLE:
            draw_title(app);
            break;
        case APP_SCREEN_OFFICE_TEST:
            draw_office(app);
            break;
        case APP_SCREEN_PLACEHOLDER:
            draw_placeholder(app);
            break;
    }

    app->needs_redraw = false;
}

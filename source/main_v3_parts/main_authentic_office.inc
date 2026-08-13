#include "assets/pc_hud_visuals.h"

#define PC_OFFICE_SOURCE_TO_WIIU_NUM 9
#define PC_OFFICE_SOURCE_TO_WIIU_DEN 16

static int pc_office_hud_scale(int value)
{
    return (value * PC_OFFICE_SOURCE_TO_WIIU_NUM +
            PC_OFFICE_SOURCE_TO_WIIU_DEN / 2) /
           PC_OFFICE_SOURCE_TO_WIIU_DEN;
}

static int pc_office_hud_width(const TextureRle *texture)
{
    return texture != NULL ? pc_office_hud_scale((int)texture->width) : 0;
}

static int pc_office_hud_height(const TextureRle *texture)
{
    return texture != NULL ? pc_office_hud_scale((int)texture->height) : 0;
}

static void pc_office_hud_draw(uint32_t targets,
                               int x,
                               int y,
                               const TextureRle *texture)
{
    if (texture == NULL) return;
    const int width = pc_office_hud_width(texture);
    const int height = pc_office_hud_height(texture);
    if (width <= 0 || height <= 0) return;
    texture_draw_rle(targets, x, y, width, height, texture);
}

static void draw_authentic_office_hud(const Game *game, uint32_t targets)
{
    /* The PC office source is 768 pixels high while the Wii U panorama is
     * rendered at 430 pixels: 430/768 ~= 9/16. Apply that same uniform scale
     * to the original Clickteam HUD sprites instead of replacing them with a
     * platform font. */
    const int right = 838;
    const int spacing = 2;

    const TextureRle *am = &gPcHudAmLabelTexture;
    const int am_w = pc_office_hud_width(am);
    const int am_h = pc_office_hud_height(am);
    int cursor = right;

    cursor -= am_w;
    pc_office_hud_draw(targets, cursor, 12, am);
    cursor -= spacing;

    const int hour = game->hour;
    if (hour == 12) {
        const TextureRle *one = gPcHudClockDigits[1];
        const TextureRle *two = gPcHudClockDigits[2];
        const int two_w = pc_office_hud_width(two);
        const int one_w = pc_office_hud_width(one);
        cursor -= two_w;
        pc_office_hud_draw(targets, cursor, 10, two);
        cursor -= 1;
        cursor -= one_w;
        pc_office_hud_draw(targets, cursor, 10, one);
    } else if (hour >= 0 && hour <= 9) {
        const TextureRle *digit = gPcHudClockDigits[hour];
        cursor -= pc_office_hud_width(digit);
        pc_office_hud_draw(targets, cursor, 10, digit);
    }
    (void)am_h;

    int night = game->night;
    if (night < 1) night = 1;
    if (night > 6) night = 6;
    const TextureRle *night_digit = gPcHudNightDigits[night];
    const TextureRle *night_label = &gPcHudNightLabelTexture;

    cursor = right;
    cursor -= pc_office_hud_width(night_digit);
    pc_office_hud_draw(targets, cursor, 28, night_digit);
    cursor -= spacing;
    cursor -= pc_office_hud_width(night_label);
    pc_office_hud_draw(targets, cursor, 31, night_label);
}

static void draw_authentic_office_fan(const Game *game,
                                      int texture_x,
                                      uint32_t targets)
{
    const int frame = (int) ((game->night_frames / 4u) % 3u);
    const TextureRle *fan = office_assets_texture(
        (OfficeAssetTextureId) (OFFICE_ASSET_FAN_0 + frame));
    if (fan == NULL) return;

    /* Position is anchored in the original office panorama. */
    const int fan_x = texture_x + 812;
    const int fan_y = 178;
    texture_draw_rle(targets, fan_x, fan_y, 120, 97, fan);
}

static void draw_authentic_office_targets(const Game *game, uint32_t targets)
{
    const int texture_x = -133 + game->office_pan;
    const uint32_t previous_targets = sPcOfficeTargets;
    sPcOfficeTargets = targets;

    graphics_clear(targets, COLOUR_BLACK);

    /*
     * Use the compact Wii U-safe PC texture. It is generated from the same PC
     * General Sprites office image 203 as the former tiled panorama, but stays
     * in one validated TextureRle object and avoids the black-screen regression
     * observed on real Wii U hardware.
     */
    texture_draw_rle(targets,
                     texture_x, 24, 1120, 430,
                     &gOfficeTexture);

    draw_springtrap_office(game);
    draw_authentic_office_fan(game, texture_x, targets);
    draw_phantom_office(game);

    /* No Wii U prototype scanline grid or `VENT ERROR` banner here. Camera
     * static and the MFA ventilation B-counter/red-alarm presentation are owned
     * by their dedicated PC fidelity layers. */
    draw_authentic_office_hud(game, targets);

    sPcOfficeTargets = previous_targets;
}

static void draw_authentic_office_tv(const Game *game)
{
    draw_authentic_office_targets(game, GRAPHICS_TARGET_TV);
}

/* PC sprite presentation for BB's Air Adventure.
 *
 * This deliberately replaces only the old generated drawing layer. The room
 * graph, platform collisions and glitch route will be replaced once the MFA
 * event/frame audit is complete.
 */
static void minigame_draw_pc_texture_centered(const TextureRle *texture,
                                               int centre_x,
                                               int centre_y)
{
    if (texture == NULL) return;
    texture_draw_rle(GRAPHICS_TARGET_BOTH,
                     centre_x - (int) texture->width / 2,
                     centre_y - (int) texture->height / 2,
                     texture->width,
                     texture->height,
                     texture);
}

static void secret_draw_bb_pc(void)
{
    static const int balloons[5][2] = {
        {178, 306}, {320, 154}, {468, 282}, {626, 128}, {754, 326}
    };
    static const TextureRle *const balloon_textures[5] = {
        &gMiniBalloonSmallRed,
        &gMiniBalloonRed,
        &gMiniBalloonPurple,
        &gMiniBalloonGreen,
        &gMiniBalloonOrange,
    };
    static int last_x = 84;
    static bool facing_left = false;

    if (sSecret.frames <= 1u) {
        last_x = sSecret.x;
        facing_left = false;
    } else if (sSecret.x < last_x) {
        facing_left = true;
    } else if (sSecret.x > last_x) {
        facing_left = false;
    }
    last_x = sSecret.x;

    secret_draw_header("BBS AIR ADVENTURE",
                       "COLLECT 5 BALLOONS, THEN REACH THE EXIT CLOUD");

    /* Temporary room shell retained until the MFA frame objects and collision
     * masks are imported. The authentic PC sprites are already used here. */
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 0, 58, 854, 382,
                       GRAPHICS_RGB(48, 172, 178));
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 0, 399, 854, 41,
                       GRAPHICS_RGB(39, 39, 39));
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 610, 190, 31, 209,
                       GRAPHICS_RGB(47, 96, 183));
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 641, 213, 31, 186,
                       GRAPHICS_RGB(47, 96, 183));
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 672, 338, 31, 61,
                       GRAPHICS_RGB(47, 96, 183));
    graphics_draw_rect(GRAPHICS_TARGET_BOTH, 610, 389, 164, 10,
                       GRAPHICS_RGB(47, 96, 183));

    for (int index = 0; index < 5; ++index) {
        if ((sSecret.collected & (uint8_t) (1u << index)) == 0u) {
            minigame_draw_pc_texture_centered(balloon_textures[index],
                                               balloons[index][0],
                                               balloons[index][1]);
        }
    }

    minigame_draw_pc_texture_centered(&gMiniExitCloud, 790, 90);
    minigame_draw_pc_texture_centered(facing_left ? &gMiniBbLeft
                                                  : &gMiniBbRight,
                                      sSecret.x,
                                      sSecret.y);
}

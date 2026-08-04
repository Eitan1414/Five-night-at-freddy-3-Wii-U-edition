                       GRAPHICS_RGB(1, 4, 2));
    graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                       28, 20, 3,
                       "CAMERA SYSTEM",
                       GRAPHICS_RGB(167, 205, 157));
    graphics_draw_frame(GRAPHICS_TARGET_GAMEPAD,
                        35, 61, 784, 326, 4,
                        GRAPHICS_RGB(61, 112, 62));
    graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD,
                       47, 73, 760, 300,
                       GRAPHICS_RGB(0, 0, 0));
    texture_draw_rle(GRAPHICS_TARGET_GAMEPAD,
                     57, 79, 740, 292,
                     camera_texture(selected_camera));

    for (int y = 82; y < 371; y += 10) {
        graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD,
                           57, y, 740, 1,
                           GRAPHICS_RGB(5, 10, 6));
    }
    for (int bar = 0; bar < 4; ++bar) {
        const uint32_t shifted = seed >> (bar * 5);
        const int y = 86 + (int) (shifted % 270u);
        const int x = 57 + (int) ((shifted >> 8) % 130u);
        const int width = 390 + (int) ((shifted >> 16) % 220u);
        graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD,
                           x, y, width, 1 + (bar & 1),
                           GRAPHICS_RGB(25, 61, 30));
    }

    graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                       55, 399, 3,
                       camera_label(selected_camera),
                       GRAPHICS_RGB(216, 239, 209));
    for (int camera = 0; camera < CAMERA_COUNT; ++camera) {
        const int x = 594 + camera * 70;
        const uint32_t colour = camera == selected_camera
            ? GRAPHICS_RGB(151, 210, 143)
            : GRAPHICS_RGB(45, 78, 47);
        graphics_draw_frame(GRAPHICS_TARGET_GAMEPAD,
                            x, 397, 54, 30, 3, colour);
    }
    graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                       55, 449, 1,
                       "LEFT RIGHT: SWITCH CAMERA   X/Y: CLOSE",
                       GRAPHICS_RGB(130, 174, 126));
}

static bool system_failed(const SystemsState *systems, int item)
{
    switch (item) {
        case MAINTENANCE_CAMERA:
            return systems->camera_failed;
        case MAINTENANCE_AUDIO:
            return systems->audio_failed;
        case MAINTENANCE_VENTILATION:
            return systems->ventilation_failed;
        default:
            return false;
    }
}

static void draw_maintenance_panel(const SystemsState *systems)
{
    static const char *const labels[MAINTENANCE_ITEM_COUNT] = {
        "CAMERA SYSTEM",
        "AUDIO DEVICES",
        "VENTILATION",
        "REBOOT ALL",
        "EXIT",
    };

    graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD,
                       0, 0, GRAPHICS_LOGICAL_WIDTH, GRAPHICS_LOGICAL_HEIGHT,
                       GRAPHICS_RGB(1, 5, 3));
    graphics_draw_frame(GRAPHICS_TARGET_GAMEPAD,
                        38, 31, 778, 412, 4,
                        GRAPHICS_RGB(54, 112, 59));
    graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                       81, 58, 4,
                       "MAINTENANCE PANEL",
                       GRAPHICS_RGB(174, 220, 166));

    for (int item = 0; item < MAINTENANCE_ITEM_COUNT; ++item) {
        const int y = 126 + item * 55;
        const bool selected = item == systems->maintenance_selection;
        if (selected) {
            graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD,
                               76, y - 12, 701, 40,
                               GRAPHICS_RGB(24, 60, 29));
            graphics_draw_rect(GRAPHICS_TARGET_GAMEPAD,
                               76, y - 12, 6, 40,
                               GRAPHICS_RGB(131, 211, 124));
        }

        graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                           101, y, 3, labels[item],
                           selected
                               ? GRAPHICS_RGB(222, 242, 216)
                               : GRAPHICS_RGB(111, 148, 108));

        if (item < MAINTENANCE_REBOOT_ALL) {
            const bool failed = system_failed(systems, item);
            graphics_draw_text(GRAPHICS_TARGET_GAMEPAD,
                               602, y, 2,

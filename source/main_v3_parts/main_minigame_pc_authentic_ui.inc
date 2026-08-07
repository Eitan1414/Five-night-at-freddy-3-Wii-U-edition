/* Authentic PC minigame UI shim.
 *
 * The legacy Wii U recreations reserve a black header/footer and print helper
 * text such as the minigame name, controller objective and progress counters.
 * The PC minigames themselves do not draw those port/debug labels, so
 * temporarily redirect the shared header and text helpers while the authentic
 * PC implementations are included. Keep the clear call so the existing
 * gameplay viewport coordinates remain unchanged.
 */
static void secret_draw_header_pc_authentic(const char *name,
                                            const char *objective)
{
    (void) name;
    (void) objective;
    graphics_clear(GRAPHICS_TARGET_BOTH, COLOUR_BLACK);
}

#define secret_draw_header secret_draw_header_pc_authentic
#define graphics_draw_text(...) ((void) 0)

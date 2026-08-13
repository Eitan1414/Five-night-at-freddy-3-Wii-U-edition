#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "game/phantom_ai.h"

#define CAM01 0
#define CAM04 3
#define CAM07 6
#define CAM08 7

static PhantomEvent update(PhantomSystem *system,
                           int hour,
                           bool camera_open,
                           bool maintenance_open,
                           int camera,
                           int office_pan)
{
    return phantoms_update(system, hour, camera_open, maintenance_open,
                           camera, office_pan);
}

static void test_night1_has_no_forced_phantoms(void)
{
    PhantomSystem system;
    phantoms_reset(&system, 1, false, 0x11111111u);

    for (int hour = 1; hour <= 5; ++hour) {
        for (int frame = 0; frame < 65 * 60; ++frame) {
            PhantomEvent event = update(&system, hour, false, false, CAM01, 0);
            assert((event.flags & PHANTOM_EVENT_JUMPSCARE_STARTED) == 0u);
            assert(phantoms_camera_overlay(&system, CAM04) == PHANTOM_NONE);
            assert(phantoms_camera_overlay(&system, CAM07) == PHANTOM_NONE);
            assert(phantoms_camera_overlay(&system, CAM08) == PHANTOM_NONE);
        }
    }
}

static void test_midnight_blocks_random_phantom_cycles(void)
{
    PhantomSystem system;
    phantoms_reset(&system, 4, false, 0x12121212u);

    /* Force every random comparison to succeed if a 20/60-second roll is
     * incorrectly evaluated.  The MFA explicitly excludes time-of-night 12. */
    system.ai_level = 100;
    (void)phantoms_on_hour_changed(&system, 12);
    assert(!system.aggressive_mode);

    for (int frame = 0; frame < 61 * 60; ++frame)
        (void)update(&system, 12, false, false, CAM01, 0);

    assert(!system.aggressive_mode);
    assert(!system.bb_armed);
    assert(!system.bb_camera_visible);
    assert(!system.freddy_armed);
    assert(!system.freddy_walking);
    assert(!system.chica_camera_visible);
    assert(!system.mangle_camera_visible);
    assert(!system.puppet_camera_visible);
}

static void test_forced_hour_availability_by_night(void)
{
    PhantomSystem n2;
    phantoms_reset(&n2, 2, false, 0x22222222u);
    (void)update(&n2, 3, false, false, CAM01, 0);
    assert(n2.bb_armed);
    assert(n2.forced_bb);
    (void)update(&n2, 5, true, false, CAM01, 0);
    assert(n2.mangle_camera_visible);
    assert(n2.forced_mangle);
    assert(!n2.chica_camera_visible);
    assert(!n2.puppet_camera_visible);

    PhantomSystem n3;
    phantoms_reset(&n3, 3, false, 0x33333333u);
    (void)update(&n3, 4, false, false, CAM01, 0);
    assert(n3.freddy_armed);
    assert(n3.forced_freddy);
    (void)update(&n3, 5, false, false, CAM01, 0);
    assert(n3.chica_camera_visible);
    assert(n3.forced_chica);
    assert(!n3.puppet_camera_visible);

    PhantomSystem n4;
    phantoms_reset(&n4, 4, false, 0x44444444u);
    (void)update(&n4, 5, false, false, CAM01, 0);
    assert(n4.puppet_camera_visible);
    assert(n4.forced_puppet);
}

static void test_bb_camera_and_vent_failure(void)
{
    PhantomSystem system;
    phantoms_reset(&system, 2, false, 0x51515151u);
    system.bb_armed = true;

    PhantomEvent event = update(&system, 1, true, false, CAM01, 0);
    assert(system.bb_camera_visible);
    assert(phantoms_camera_overlay(&system, CAM01) == PHANTOM_BALLOON_BOY);
    assert((event.flags & PHANTOM_EVENT_VISUAL_CHANGED) != 0u);

    bool started = false;
    for (uint32_t frame = 0; frame < system.time_limit_frames + 4u; ++frame) {
        event = update(&system, 1, true, false, CAM01, 0);
        if ((event.flags & PHANTOM_EVENT_JUMPSCARE_STARTED) != 0u) {
            started = true;
            break;
        }
    }
    assert(started);
    assert(event.phantom == PHANTOM_BALLOON_BOY);
    assert((event.flags & PHANTOM_EVENT_FORCE_CLOSE_PANEL) != 0u);
    assert((event.flags & PHANTOM_EVENT_RELEASE_SPRINGTRAP) != 0u);

    bool ended = false;
    for (int frame = 0; frame < 120; ++frame) {
        event = update(&system, 1, false, false, CAM01, 0);
        if ((event.flags & PHANTOM_EVENT_JUMPSCARE_ENDED) != 0u) {
            ended = true;
            assert((event.flags & PHANTOM_EVENT_VENTILATION_FAILURE) != 0u);
            break;
        }
    }
    assert(ended);
    assert(system.scare_cooldown_seconds == 10u);
}

static void test_chica_switch_pauses_and_lower_clears(void)
{
    PhantomSystem system;
    phantoms_reset(&system, 3, false, 0x61616161u);
    system.chica_camera_visible = true;

    (void)update(&system, 1, true, false, CAM07, 0);
    const uint32_t seen = system.chica_seen_frames;
    assert(seen > 0u);

    for (int frame = 0; frame < 30; ++frame)
        (void)update(&system, 1, true, false, CAM04, 0);
    assert(system.chica_seen_frames == seen);
    assert(system.chica_camera_visible);

    /* Actual lowering edge clears Chica camera state. */
    (void)update(&system, 1, false, false, CAM04, 0);
    assert(!system.chica_camera_visible);
    assert(system.chica_seen_frames == 0u);
}

static void test_chica_threshold_then_office_scare(void)
{
    PhantomSystem system;
    phantoms_reset(&system, 3, false, 0x71717171u);
    system.chica_camera_visible = true;
    system.chica_seen_frames = system.time_limit_frames - 1u;

    PhantomEvent event = update(&system, 1, true, false, CAM07, 0);
    assert(system.chica_office_waiting);
    assert(!system.chica_camera_visible);
    assert((event.flags & PHANTOM_EVENT_RELEASE_SPRINGTRAP) != 0u);

    event = update(&system, 1, false, false, CAM07, -45);
    assert((event.flags & PHANTOM_EVENT_JUMPSCARE_STARTED) != 0u);
    assert(event.phantom == PHANTOM_CHICA);
}

static void test_mangle_audio_error_occurs_after_office_effect(void)
{
    PhantomSystem system;
    phantoms_reset(&system, 2, false, 0x81818181u);
    system.mangle_camera_visible = true;
    system.mangle_seen_frames = system.time_limit_frames - 1u;

    PhantomEvent event = update(&system, 1, true, false, CAM04, 0);
    assert(system.mangle_office_active);
    assert((event.flags & PHANTOM_EVENT_FORCE_CLOSE_PANEL) != 0u);
    assert((event.flags & PHANTOM_EVENT_GARBLE_STARTED) != 0u);
    assert((event.flags & PHANTOM_EVENT_AUDIO_FAILURE) == 0u);

    bool audio_failed = false;
    for (int frame = 0; frame < 1000; ++frame) {
        event = update(&system, 1, false, false, CAM01, 0);
        if ((event.flags & PHANTOM_EVENT_AUDIO_FAILURE) != 0u) {
            audio_failed = true;
            assert((event.flags & PHANTOM_EVENT_GARBLE_ENDED) != 0u);
            break;
        }
    }
    assert(audio_failed);
    assert(!system.mangle_office_active);
}

static void test_puppet_mask_lifecycle(void)
{
    PhantomSystem system;
    phantoms_reset(&system, 4, false, 0x91919191u);
    system.puppet_camera_visible = true;
    system.puppet_seen_frames = system.time_limit_frames - 1u;

    PhantomEvent event = update(&system, 1, true, false, CAM08, 0);
    assert(system.puppet_office_active);
    assert((event.flags & PHANTOM_EVENT_FORCE_CLOSE_PANEL) != 0u);
    assert((event.flags & PHANTOM_EVENT_MASK_STARTED) != 0u);
    assert((event.flags & PHANTOM_EVENT_RELEASE_SPRINGTRAP) != 0u);

    bool ended = false;
    for (int frame = 0; frame < 1100; ++frame) {
        event = update(&system, 1, false, false, CAM01, 0);
        if ((event.flags & PHANTOM_EVENT_MASK_ENDED) != 0u) {
            ended = true;
            break;
        }
    }
    assert(ended);
    assert(!system.puppet_office_active);
}

static void test_foxy_roll_waits_for_monitor_animation(void)
{
    PhantomSystem system;
    phantoms_reset(&system, 3, false, 0xA1A1A1A1u);

    PhantomEvent event = phantoms_on_panel_opened(&system);
    assert(event.flags == PHANTOM_EVENT_NONE);
    assert(system.foxy_roll_pending);

    (void)phantoms_on_monitor_animation_finished(&system);
    assert(!system.foxy_roll_pending);
}

static void test_foxy_office_collision_starts_scare(void)
{
    PhantomSystem system;
    phantoms_reset(&system, 3, false, 0xB1B1B1B1u);
    system.foxy_present = true;
    system.scare_cooldown_seconds = 0u;

    PhantomEvent event = update(&system, 1, false, false, CAM01, -70);
    assert((event.flags & PHANTOM_EVENT_JUMPSCARE_STARTED) != 0u);
    assert(event.phantom == PHANTOM_FOXY);
    assert(!system.foxy_present);
}

int main(void)
{
    test_night1_has_no_forced_phantoms();
    test_midnight_blocks_random_phantom_cycles();
    test_forced_hour_availability_by_night();
    test_bb_camera_and_vent_failure();
    test_chica_switch_pauses_and_lower_clears();
    test_chica_threshold_then_office_scare();
    test_mangle_audio_error_occurs_after_office_effect();
    test_puppet_mask_lifecycle();
    test_foxy_roll_waits_for_monitor_animation();
    test_foxy_office_collision_starts_scare();
    puts("Phantom AI fidelity tests passed");
    return 0;
}

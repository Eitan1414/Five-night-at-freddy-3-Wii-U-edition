#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "game/springtrap_ai.h"

static void tick(SpringtrapAI *ai,
                 int night,
                 int hour,
                 SpringtrapVent seal,
                 bool blinded)
{
    (void)springtrap_ai_update(ai, night, hour, 0u, seal, false, blinded);
}

static SpringtrapEvent tick_until_event(SpringtrapAI *ai,
                                        int night,
                                        int hour,
                                        SpringtrapVent seal,
                                        uint32_t wanted,
                                        int max_frames)
{
    SpringtrapEvent event = {0};
    for (int frame = 0; frame < max_frames; ++frame) {
        event = springtrap_ai_update(ai, night, hour, 0u, seal, false, false);
        if ((event.flags & wanted) != 0u) return event;
    }
    assert(!"Springtrap event timed out");
    return event;
}

/* Force one MFA movement opportunity without altering the actual action roll.
 * This makes attack-chain tests deterministic in timing while still exercising
 * the same Random(4)+1 branch used by gameplay. */
static SpringtrapEvent force_opportunity(SpringtrapAI *ai,
                                         int night,
                                         bool camera_open,
                                         bool maintenance_open,
                                         bool blinded)
{
    springtrap_ai_set_runtime_state(ai,
                                    camera_open,
                                    maintenance_open,
                                    2,
                                    false,
                                    0u);
    ai->move_counter = 100u;
    ai->one_second_frames = 59u;
    ai->aggressive = true;
    return springtrap_ai_update(ai,
                                night,
                                1,
                                0u,
                                SPRINGTRAP_VENT_NONE,
                                false,
                                blinded);
}

static void test_night1_is_absent(void)
{
    SpringtrapAI ai;
    springtrap_ai_reset(&ai, 1, 0x12345678u);

    for (int frame = 0; frame < 30 * 60; ++frame) {
        springtrap_ai_set_runtime_state(&ai, false, false, 0, true, 0u);
        SpringtrapEvent event =
            springtrap_ai_update(&ai, 1, 5, 0u, SPRINGTRAP_VENT_NONE,
                                 false, true);
        assert(event.flags == SPRINGTRAP_EVENT_NONE);
    }

    assert(springtrap_ai_camera(&ai) == -1);
    assert(!springtrap_ai_is_in_vent(&ai));
    assert(springtrap_ai_office_side(&ai) == SPRINGTRAP_OFFICE_NONE);
    assert(!springtrap_ai_is_danger_near(&ai));
    assert((springtrap_ai_lure(&ai, 5, 1).flags &
            SPRINGTRAP_EVENT_LURE_INVALID) != 0u);
}

static void test_spawn_is_cam06_to_cam10(void)
{
    for (uint32_t seed = 1u; seed < 256u; ++seed) {
        SpringtrapAI ai;
        springtrap_ai_reset(&ai, 2, seed);
        const int camera = springtrap_ai_camera(&ai);
        assert(camera >= 5 && camera <= 9);
    }
}

static void test_no_screen_counter_and_reset(void)
{
    SpringtrapAI ai;
    springtrap_ai_reset(&ai, 2, 0x20406080u);
    ai.aggressive = false;
    ai.aggressive_refresh_frames = 0u;

    for (int frame = 0; frame < 10 * 60; ++frame) {
        springtrap_ai_set_runtime_state(&ai, false, false, 2, false, 0u);
        tick(&ai, 2, 1, SPRINGTRAP_VENT_NONE, false);
    }
    assert(ai.office_idle_frames == 10u * 60u);
    assert(!ai.aggressive);

    for (int frame = 0; frame < 60; ++frame) {
        springtrap_ai_set_runtime_state(&ai, false, false, 2, false, 0u);
        tick(&ai, 2, 1, SPRINGTRAP_VENT_NONE, false);
    }
    assert(ai.office_idle_frames == 11u * 60u);
    assert(ai.aggressive);

    springtrap_ai_set_runtime_state(&ai, true, false, 2, false, 0u);
    assert(ai.office_idle_frames == 0u);
}

static void test_midnight_clears_late_hour_aggression(void)
{
    SpringtrapAI ai;
    springtrap_ai_reset(&ai, 5, 0x30405060u);

    /* Group 744 (>=4) appears before group 758 (==12) in the MFA.  Game::hour
     * stores midnight as 12, so the later clear must win in the same update. */
    ai.aggressive = true;
    ai.aggressive_refresh_frames = 0u;
    springtrap_ai_set_runtime_state(&ai, true, false, 2, false, 0u);
    tick(&ai, 5, 12, SPRINGTRAP_VENT_NONE, false);
    assert(!ai.aggressive);
}

static void test_lure_adjacency(void)
{
    /* The MFA hearing graph is directional. */
    assert(springtrap_ai_cameras_adjacent(2, 1));  /* CAM03 -> CAM02 */
    assert(springtrap_ai_cameras_adjacent(4, 1));  /* CAM05 -> CAM02 */
    assert(springtrap_ai_cameras_adjacent(8, 9));  /* CAM09 -> CAM10 */
    assert(!springtrap_ai_cameras_adjacent(9, 7)); /* CAM10 !-> CAM08 */
    assert(!springtrap_ai_cameras_adjacent(0, 1)); /* CAM01 attack chain */
}

static void test_sealed_vent_returns_to_source(void)
{
    SpringtrapAI ai;
    springtrap_ai_reset(&ai, 5, 0xA5A5A5A5u);
    ai.kind = SPRINGTRAP_LOCATION_VENT;
    ai.camera = -1;
    ai.vent = SPRINGTRAP_VENT_13;
    ai.vent_source_camera = 4; /* CAM05 */
    ai.move_counter = 100u;
    ai.one_second_frames = 59u;
    ai.aggressive = true;

    SpringtrapEvent event =
        tick_until_event(&ai, 5, 1, SPRINGTRAP_VENT_13,
                         SPRINGTRAP_EVENT_VENT_EXIT, 20 * 60);
    assert((event.flags & SPRINGTRAP_EVENT_ATTACK) == 0u);
    assert(springtrap_ai_camera(&ai) == 4);
    assert(!springtrap_ai_is_in_vent(&ai));
}

static void test_unsealed_vent14_attacks_office(void)
{
    SpringtrapAI ai;
    springtrap_ai_reset(&ai, 5, 0xB6B6B6B6u);
    ai.kind = SPRINGTRAP_LOCATION_VENT;
    ai.camera = -1;
    ai.vent = SPRINGTRAP_VENT_14;
    ai.vent_source_camera = 9; /* CAM10 */
    ai.move_counter = 100u;
    ai.one_second_frames = 59u;
    ai.aggressive = true;

    SpringtrapEvent event =
        tick_until_event(&ai, 5, 1, SPRINGTRAP_VENT_NONE,
                         SPRINGTRAP_EVENT_ATTACK, 20 * 60);
    assert((event.flags & SPRINGTRAP_EVENT_VENT_EXIT) != 0u);
    assert(ai.kind == SPRINGTRAP_LOCATION_OFFICE_INSIDE);
}

static void test_phantom_force_move(void)
{
    SpringtrapAI ai;
    springtrap_ai_reset(&ai, 4, 0xC7C7C7C7u);
    ai.kind = SPRINGTRAP_LOCATION_CAMERA;
    ai.camera = 8; /* CAM09 */
    ai.force_move_pending = true;
    ai.force_to = 1u;

    SpringtrapEvent event =
        springtrap_ai_update(&ai, 4, 1, 0u, SPRINGTRAP_VENT_NONE,
                             false, false);
    assert((event.flags & SPRINGTRAP_EVENT_MOVED) != 0u);
    assert(springtrap_ai_camera(&ai) == 4); /* CAM05 */
    assert(!ai.force_move_pending);
}

static void test_window_waits_for_system_screen(void)
{
    SpringtrapAI ai;
    springtrap_ai_reset(&ai, 5, 0xD8123412u);
    ai.kind = SPRINGTRAP_LOCATION_OFFICE_WINDOW;
    ai.camera = -1;

    for (int attempt = 0; attempt < 64; ++attempt) {
        SpringtrapEvent event = force_opportunity(&ai, 5, false, false, false);
        assert((event.flags & SPRINGTRAP_EVENT_ATTACK) == 0u);
        assert(ai.kind == SPRINGTRAP_LOCATION_OFFICE_WINDOW);
    }

    bool advanced = false;
    for (int attempt = 0; attempt < 64; ++attempt) {
        (void)force_opportunity(&ai, 5, true, false, false);
        if (ai.kind == SPRINGTRAP_LOCATION_HALL_RUN) {
            advanced = true;
            break;
        }
    }
    assert(advanced);
}

static void test_hall_run_reaches_hidden_stage(void)
{
    SpringtrapAI ai;
    springtrap_ai_reset(&ai, 5, 0xE9234523u);
    ai.kind = SPRINGTRAP_LOCATION_HALL_RUN;
    ai.camera = -1;

    bool advanced = false;
    for (int attempt = 0; attempt < 64; ++attempt) {
        (void)force_opportunity(&ai, 5, false, false, false);
        if (ai.kind == SPRINGTRAP_LOCATION_HALL_HIDDEN) {
            advanced = true;
            break;
        }
    }
    assert(advanced);
}

static void test_hidden_stage_waits_then_advances(void)
{
    SpringtrapAI ai;
    springtrap_ai_reset(&ai, 5, 0xFA345634u);
    ai.kind = SPRINGTRAP_LOCATION_HALL_HIDDEN;
    ai.camera = -1;

    for (int attempt = 0; attempt < 64; ++attempt) {
        (void)force_opportunity(&ai, 5, false, false, false);
        assert(ai.kind == SPRINGTRAP_LOCATION_HALL_HIDDEN);
    }

    bool advanced = false;
    for (int attempt = 0; attempt < 64; ++attempt) {
        (void)force_opportunity(&ai, 5, false, true, false);
        if (ai.kind == SPRINGTRAP_LOCATION_CAMERA ||
            ai.kind == SPRINGTRAP_LOCATION_OFFICE_LEFT) {
            advanced = true;
            if (ai.kind == SPRINGTRAP_LOCATION_CAMERA)
                assert(ai.camera == 0); /* CAM01 */
            break;
        }
    }
    assert(advanced);
}

static void test_cam01_waits_then_enters_attack_chain(void)
{
    SpringtrapAI ai;
    springtrap_ai_reset(&ai, 5, 0xAB456745u);
    ai.kind = SPRINGTRAP_LOCATION_CAMERA;
    ai.camera = 0; /* CAM01 */

    for (int attempt = 0; attempt < 64; ++attempt) {
        (void)force_opportunity(&ai, 5, false, false, false);
        assert(ai.kind == SPRINGTRAP_LOCATION_CAMERA);
        assert(ai.camera == 0);
    }

    bool advanced = false;
    for (int attempt = 0; attempt < 64; ++attempt) {
        (void)force_opportunity(&ai, 5, true, false, false);
        if (ai.kind == SPRINGTRAP_LOCATION_HALL_HIDDEN ||
            ai.kind == SPRINGTRAP_LOCATION_OFFICE_LEFT) {
            advanced = true;
            break;
        }
    }
    assert(advanced);
}

static void test_left_stage_waits_then_attacks(void)
{
    SpringtrapAI ai;
    springtrap_ai_reset(&ai, 5, 0xBC567856u);
    ai.kind = SPRINGTRAP_LOCATION_OFFICE_LEFT;
    ai.camera = -1;

    for (int attempt = 0; attempt < 64; ++attempt) {
        SpringtrapEvent event = force_opportunity(&ai, 5, false, false, false);
        assert((event.flags & SPRINGTRAP_EVENT_ATTACK) == 0u);
        assert(ai.kind == SPRINGTRAP_LOCATION_OFFICE_LEFT);
    }

    bool attacked = false;
    for (int attempt = 0; attempt < 64; ++attempt) {
        SpringtrapEvent event = force_opportunity(&ai, 5, false, true, false);
        if ((event.flags & SPRINGTRAP_EVENT_ATTACK) != 0u) {
            attacked = true;
            assert(ai.kind == SPRINGTRAP_LOCATION_OFFICE_INSIDE);
            break;
        }
    }
    assert(attacked);
}

static void test_blackout_can_advance_cam01(void)
{
    SpringtrapAI ai;
    springtrap_ai_reset(&ai, 5, 0xCD678967u);
    ai.kind = SPRINGTRAP_LOCATION_CAMERA;
    ai.camera = 0;

    bool advanced = false;
    for (int attempt = 0; attempt < 64; ++attempt) {
        (void)force_opportunity(&ai, 5, false, false, true);
        if (ai.kind != SPRINGTRAP_LOCATION_CAMERA || ai.camera != 0) {
            advanced = true;
            break;
        }
    }
    assert(advanced);
}

int main(void)
{
    test_night1_is_absent();
    test_spawn_is_cam06_to_cam10();
    test_no_screen_counter_and_reset();
    test_midnight_clears_late_hour_aggression();
    test_lure_adjacency();
    test_sealed_vent_returns_to_source();
    test_unsealed_vent14_attacks_office();
    test_phantom_force_move();
    test_window_waits_for_system_screen();
    test_hall_run_reaches_hidden_stage();
    test_hidden_stage_waits_then_advances();
    test_cam01_waits_then_enters_attack_chain();
    test_left_stage_waits_then_attacks();
    test_blackout_can_advance_cam01();
    puts("Springtrap AI fidelity tests passed");
    return 0;
}

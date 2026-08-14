#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "game/phantom_ai.h"

#define CAM01 0

static uint32_t lcg_next(uint32_t state)
{
    return state * 1664525u + 1013904223u;
}

static uint32_t state_for_roll(uint32_t denominator, uint32_t result)
{
    for (uint32_t state = 0u; state < 2000000u; ++state) {
        if ((lcg_next(state) % denominator) == result)
            return state;
    }
    assert(!"unable to find deterministic RNG state");
    return 0u;
}

static void test_20_and_60_second_random_clocks(void)
{
    PhantomSystem system;
    phantoms_reset(&system, 4, false, 0xAABBCCDDu);

    /* Make every Random(range)+1 <= AI comparison succeed. This isolates the
     * event-clock boundaries from the actual night AI probability. */
    system.ai_level = 100;

    for (int frame = 0; frame < 20 * 60 - 1; ++frame)
        (void)phantoms_update(&system, 1, false, false, CAM01, 0);

    assert(!system.bb_armed);
    assert(!system.mangle_camera_visible);
    assert(!system.chica_camera_visible);
    assert(!system.puppet_camera_visible);
    assert(!system.freddy_armed);

    (void)phantoms_update(&system, 1, false, false, CAM01, 0);
    assert(system.bb_armed);
    assert(system.mangle_camera_visible);
    assert(system.chica_camera_visible);
    assert(system.puppet_camera_visible);
    assert(!system.freddy_armed);

    /* Reset only the random Phantom states so the same instance can reach the
     * 60-second Freddy boundary without a forced-hour event. */
    system.bb_armed = false;
    system.mangle_camera_visible = false;
    system.chica_camera_visible = false;
    system.puppet_camera_visible = false;

    for (int frame = 20 * 60; frame < 60 * 60 - 1; ++frame)
        (void)phantoms_update(&system, 1, false, false, CAM01, 0);

    assert(!system.freddy_armed);
    (void)phantoms_update(&system, 1, false, false, CAM01, 0);
    assert(system.freddy_armed);
}

static void test_midnight_never_runs_random_cycles(void)
{
    PhantomSystem system;
    phantoms_reset(&system, 6, false, 0x11223344u);
    system.ai_level = 100;

    for (int frame = 0; frame < 2 * 60 * 60; ++frame)
        (void)phantoms_update(&system, 0, false, false, CAM01, 0);

    assert(!system.bb_armed);
    assert(!system.freddy_armed);
    assert(!system.freddy_walking);
    assert(!system.mangle_camera_visible);
    assert(!system.chica_camera_visible);
    assert(!system.puppet_camera_visible);
}

static void test_foxy_denominator(int night, uint32_t denominator)
{
    PhantomSystem system;

    phantoms_reset(&system, night, false, 0x1234u);
    system.rng = state_for_roll(denominator, 1u);
    (void)phantoms_on_panel_opened(&system);
    (void)phantoms_on_monitor_animation_finished(&system);
    assert(system.foxy_present);

    phantoms_reset(&system, night, false, 0x5678u);
    system.rng = state_for_roll(denominator, 0u);
    (void)phantoms_on_panel_opened(&system);
    (void)phantoms_on_monitor_animation_finished(&system);
    assert(!system.foxy_present);
}

static void test_foxy_night_odds_routes(void)
{
    test_foxy_denominator(2, 1000u);
    test_foxy_denominator(3, 50u);
    test_foxy_denominator(4, 25u);
    test_foxy_denominator(5, 10u);
    test_foxy_denominator(6, 10u);
}

static void test_forced_hour_routes(void)
{
    PhantomSystem system;

    phantoms_reset(&system, 2, false, 1u);
    (void)phantoms_update(&system, 3, false, false, CAM01, 0);
    assert(system.bb_armed && system.forced_bb);

    phantoms_reset(&system, 3, false, 2u);
    (void)phantoms_update(&system, 4, false, false, CAM01, 0);
    assert(system.freddy_armed && system.forced_freddy);

    phantoms_reset(&system, 4, false, 3u);
    (void)phantoms_update(&system, 5, true, false, CAM01, 0);
    assert(system.mangle_camera_visible && system.forced_mangle);
    assert(system.chica_camera_visible && system.forced_chica);
    assert(system.puppet_camera_visible && system.forced_puppet);
}

int main(void)
{
    test_20_and_60_second_random_clocks();
    test_midnight_never_runs_random_cycles();
    test_foxy_night_odds_routes();
    test_forced_hour_routes();
    puts("Rare Phantom event fidelity tests passed");
    return 0;
}

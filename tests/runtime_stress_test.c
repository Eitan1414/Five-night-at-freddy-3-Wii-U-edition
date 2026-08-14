#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "game/phantom_ai.h"
#include "game/springtrap_ai.h"

#define CAMERA_COUNT 10
#define FRAMES_PER_HOUR 3600
#define HOURS_PER_NIGHT 6
#define STRESS_SEEDS 32

static uint32_t movement_interval(int night)
{
    static const uint32_t intervals[6] = {1u, 360u, 300u, 240u, 180u, 120u};
    assert(night >= 1 && night <= 6);
    return intervals[night - 1];
}

static void assert_springtrap_valid(const SpringtrapAI *ai)
{
    assert(ai != NULL);
    assert(ai->kind >= SPRINGTRAP_LOCATION_CAMERA);
    assert(ai->kind <= SPRINGTRAP_LOCATION_OFFICE_INSIDE);
    assert(ai->vent >= SPRINGTRAP_VENT_NONE);
    assert(ai->vent < SPRINGTRAP_VENT_COUNT);
    assert(ai->selected_camera >= 0 && ai->selected_camera < CAMERA_COUNT);
    if (ai->kind == SPRINGTRAP_LOCATION_CAMERA)
        assert(ai->camera >= 0 && ai->camera < CAMERA_COUNT);
    if (ai->kind == SPRINGTRAP_LOCATION_VENT)
        assert(ai->vent >= 0 && ai->vent < SPRINGTRAP_VENT_COUNT);
}

static void assert_phantoms_valid(const PhantomSystem *system)
{
    assert(system != NULL);
    assert(system->night >= 1 && system->night <= 6);
    assert(system->jumpscare >= PHANTOM_NONE && system->jumpscare < PHANTOM_COUNT);
    assert(system->bb_camera >= -1 && system->bb_camera < CAMERA_COUNT);
    assert(system->time_limit_frames > 0u);
    assert(system->scare_cooldown_seconds <= 10u);
    assert(system->mangle_office_frames < 100000u);
    assert(system->puppet_office_frames < 100000u);
}

static void run_stress_night(int night, uint32_t seed)
{
    SpringtrapAI springtrap;
    PhantomSystem phantoms;
    springtrap_ai_reset(&springtrap, night, seed ^ 0x13579BDFu);
    phantoms_reset(&phantoms, night, false, seed ^ 0x2468ACE0u);

    bool camera_open = false;
    bool maintenance_open = false;
    bool ventilation_failed = false;
    SpringtrapVent sealed_vent = SPRINGTRAP_VENT_NONE;
    uint32_t idle_frames = 0u;
    bool previous_camera_open = false;

    for (int frame = 0; frame < FRAMES_PER_HOUR * HOURS_PER_NIGHT; ++frame) {
        const int hour = frame / FRAMES_PER_HOUR; /* 0 == 12 AM, then 1..5. */
        const int selected_camera = (frame / 7 + (int)(seed % CAMERA_COUNT)) % CAMERA_COUNT;
        const int panel_phase = (frame + (int)(seed & 31u)) % 180;
        camera_open = panel_phase < 92;
        maintenance_open = panel_phase >= 92 && panel_phase < 128;
        const int office_pan = ((frame / 5 + (int)(seed & 15u)) % 161) - 80;

        if (camera_open != previous_camera_open) {
            if (camera_open) {
                (void)phantoms_on_panel_opened(&phantoms);
                /* Real game rolls Foxy after the eleven-frame monitor raise.
                 * Stress the completed-animation callback at every opening. */
                (void)phantoms_on_monitor_animation_finished(&phantoms);
            }
            previous_camera_open = camera_open;
        }

        if ((frame % FRAMES_PER_HOUR) == 0)
            (void)phantoms_on_hour_changed(&phantoms, hour);

        PhantomEvent phantom_event = phantoms_update(
            &phantoms, hour, camera_open, maintenance_open,
            selected_camera, office_pan);

        if ((phantom_event.flags & PHANTOM_EVENT_FORCE_CLOSE_PANEL) != 0u) {
            camera_open = false;
            maintenance_open = false;
        }
        if ((phantom_event.flags & PHANTOM_EVENT_RELEASE_SPRINGTRAP) != 0u)
            springtrap_ai_release_observation(&springtrap);
        if ((phantom_event.flags & PHANTOM_EVENT_VENTILATION_FAILURE) != 0u)
            ventilation_failed = true;

        /* Simulate successful maintenance repairs periodically so the stress
         * path repeatedly enters/exits blackout and Hallucinationtrap pressure
         * rather than staying in one terminal state forever. */
        if (ventilation_failed && (frame % 1800) == 1799)
            ventilation_failed = false;

        if (!camera_open && !maintenance_open)
            ++idle_frames;
        else
            idle_frames = 0u;

        if ((frame % 420) == 0) {
            const int raw = (frame / 420) % (SPRINGTRAP_VENT_COUNT + 1);
            sealed_vent = raw == SPRINGTRAP_VENT_COUNT
                ? SPRINGTRAP_VENT_NONE
                : (SpringtrapVent)raw;
        }

        springtrap_ai_set_runtime_state(
            &springtrap, camera_open, maintenance_open, selected_camera,
            ventilation_failed, idle_frames);

        const bool observed = camera_open &&
            springtrap_ai_is_on_camera(&springtrap, selected_camera);
        const bool blinded = ventilation_failed && ((frame / 60) & 1) != 0;
        SpringtrapEvent spring_event = springtrap_ai_update(
            &springtrap, night, hour, movement_interval(night), sealed_vent,
            observed, blinded);

        /* Hammer Play Audio at approximately the retail recharge cadence. */
        if (night >= 2 && camera_open && (frame % 630) == 0) {
            const int target = (selected_camera + 1) % CAMERA_COUNT;
            (void)springtrap_ai_lure(&springtrap, target, night);
        }

        /* An attack ends a real night. For stress testing, reset only the AI so
         * the same process can continue hammering thousands more transitions. */
        if ((spring_event.flags & SPRINGTRAP_EVENT_ATTACK) != 0u) {
            springtrap_ai_reset(&springtrap, night,
                                seed ^ (uint32_t)frame ^ 0xA5A5A5A5u);
            springtrap_ai_set_runtime_state(
                &springtrap, camera_open, maintenance_open, selected_camera,
                ventilation_failed, idle_frames);
        }

        assert_springtrap_valid(&springtrap);
        assert_phantoms_valid(&phantoms);
    }
}

int main(void)
{
    uint64_t simulated_frames = 0u;
    for (int night = 1; night <= 6; ++night) {
        for (uint32_t seed_index = 0u; seed_index < STRESS_SEEDS; ++seed_index) {
            const uint32_t seed =
                0x9E3779B9u * (seed_index + 1u) ^ (uint32_t)(night * 0x10203);
            run_stress_night(night, seed);
            simulated_frames += (uint64_t)FRAMES_PER_HOUR * HOURS_PER_NIGHT;
        }
    }

    printf("Runtime stress test passed: %llu synthetic 60Hz frames across Nights 1-6\n",
           (unsigned long long)simulated_frames);
    return 0;
}

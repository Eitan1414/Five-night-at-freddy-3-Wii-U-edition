                           PHANTOM_EVENT_AUDIO_FAILURE |
                           PHANTOM_EVENT_RELEASE_SPRINGTRAP |
                           PHANTOM_EVENT_GARBLE_STARTED |
                           PHANTOM_EVENT_VISUAL_CHANGED;
            event.phantom = PHANTOM_MANGLE;
        }
    }

    if (system->puppet_camera_visible) {
        if (!camera_open || selected_camera != 7) {
            if (system->puppet_seen_frames > 0u) {
                dismiss_camera_phantom(system, PHANTOM_PUPPET);
                event.flags |= PHANTOM_EVENT_VISUAL_CHANGED;
                event.phantom = PHANTOM_PUPPET;
            }
        } else if (++system->puppet_seen_frames >= CAMERA_REACTION_FRAMES) {
            system->puppet_camera_visible = false;
            system->puppet_seen_frames = 0u;
            system->puppet_office_active = true;
            system->puppet_office_frames = puppet_duration(system);
            /* Match the requested 10 / 13 / 15 / 17 second encounter curve. */
            if (system->night == 4 || system->night == 5) {
                system->puppet_office_frames += FRAMES_PER_SECOND;
            }
            event.flags |= PHANTOM_EVENT_FORCE_CLOSE_PANEL |
                           PHANTOM_EVENT_RELEASE_SPRINGTRAP |
                           PHANTOM_EVENT_VENTILATION_FAILURE |
                           PHANTOM_EVENT_MASK_STARTED |
                           PHANTOM_EVENT_VISUAL_CHANGED;
            event.phantom = PHANTOM_PUPPET;
        }
    }

    return event;
}

PhantomId phantoms_camera_overlay(const PhantomSystem *system,
                                  int selected_camera)
{
    /*
     * Phantom camera visuals are already baked into the user's camera pack.
     * Keep the AI/timers above fully active, but never draw a second Phantom
     * sprite on top of those camera images.
     */
    (void) system;
    (void) selected_camera;
    return PHANTOM_NONE;
}

PhantomId phantoms_office_overlay(const PhantomSystem *system)
{
    if (system == NULL) {
        return PHANTOM_NONE;
    }
    if (system->puppet_office_active) {
        return PHANTOM_PUPPET;
    }
    if (system->mangle_office_active) {
        return PHANTOM_MANGLE;
    }
    if (system->chica_office_waiting) {
        return PHANTOM_CHICA;
    }
    if (system->foxy_present) {
        return PHANTOM_FOXY;
    }
    if (system->freddy_walking) {
        return PHANTOM_FREDDY;
    }
    return PHANTOM_NONE;
}

PhantomId phantoms_active_jumpscare(const PhantomSystem *system)
{
    return system != NULL ? system->jumpscare : PHANTOM_NONE;
}

uint32_t phantoms_jumpscare_frames(const PhantomSystem *system)
{
    return system != NULL ? system->jumpscare_frames : 0u;
}

bool phantoms_blocks_input(const PhantomSystem *system)
{
    return system != NULL &&
           (system->jumpscare != PHANTOM_NONE || system->puppet_office_active);
}

bool phantoms_foxy_present(const PhantomSystem *system)
{
    return system != NULL && system->foxy_present;
}

bool phantoms_freddy_walking(const PhantomSystem *system)
{
    return system != NULL && system->freddy_walking;
}

bool phantoms_mangle_active(const PhantomSystem *system)
{
    return system != NULL && system->mangle_office_active;
}

bool phantoms_puppet_active(const PhantomSystem *system)
{
    return system != NULL && system->puppet_office_active;
}

uint32_t phantoms_mangle_remaining(const PhantomSystem *system)
{
    return system != NULL ? system->mangle_office_frames : 0u;
}

uint32_t phantoms_puppet_remaining(const PhantomSystem *system)
{
    return system != NULL ? system->puppet_office_frames : 0u;
}

const char *phantom_name(PhantomId phantom)
{
    static const char *const names[PHANTOM_COUNT] = {
        "PHANTOM FOXY",
        "PHANTOM BALLOON BOY",
        "PHANTOM FREDDY",
        "PHANTOM CHICA",
        "PHANTOM MANGLE",
        "PHANTOM PUPPET",
    };
    return phantom >= 0 && phantom < PHANTOM_COUNT
        ? names[phantom]
        : "NO PHANTOM";
}

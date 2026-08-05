        system->chica_camera_visible = true;
        system->chica_seen_frames = 0u;
    }
}

static void maybe_spawn_mangle(PhantomSystem *system, int hour)
{
    if (system->night < 3 || hour < 1 ||
        attacked(system, PHANTOM_MANGLE) ||
        system->mangle_camera_visible || system->mangle_office_active) {
        return;
    }
    ++system->mangle_spawn_frames;
    if (system->mangle_spawn_frames < timed_interval(system, 35u, 20u)) {
        return;
    }
    system->mangle_spawn_frames = 0u;
    if ((int) (next_random(system) % 10u) <
        chance_tenths(system, 4, 7)) {
        system->mangle_camera_visible = true;
        system->mangle_seen_frames = 0u;
    }
}

static void maybe_spawn_puppet(PhantomSystem *system, int hour)
{
    if (system->night < 3 || hour < 1 ||
        attacked(system, PHANTOM_PUPPET) ||
        system->puppet_camera_visible || system->puppet_office_active) {
        return;
    }
    ++system->puppet_spawn_frames;
    if (system->puppet_spawn_frames < timed_interval(system, 30u, 20u)) {
        return;
    }
    system->puppet_spawn_frames = 0u;
    if ((int) (next_random(system) % 10u) <
        chance_tenths(system, 5, 7)) {
        system->puppet_camera_visible = true;
        system->puppet_seen_frames = 0u;
    }
}

static uint32_t jumpscare_duration_frames(PhantomId phantom)
{
    switch (phantom) {
        case PHANTOM_FOXY: return 11u * 4u;
        case PHANTOM_BALLOON_BOY: return 9u * 4u;
        case PHANTOM_FREDDY: return 7u * 4u;
        case PHANTOM_CHICA: return 6u * 4u;
        default: return JUMPSCARE_DURATION_FRAMES;
    }
}

static PhantomEvent update_jumpscare(PhantomSystem *system)
{
    PhantomEvent event = empty_event();
    if (system->jumpscare == PHANTOM_NONE) {
        return event;
    }
    ++system->jumpscare_frames;
    if (system->jumpscare_frames >=
        jumpscare_duration_frames(system->jumpscare)) {
        const PhantomId finished = system->jumpscare;
        mark_attacked(system, finished);
        system->jumpscare = PHANTOM_NONE;
        system->jumpscare_frames = 0u;
        event.flags = PHANTOM_EVENT_VENTILATION_FAILURE |
                      PHANTOM_EVENT_JUMPSCARE_ENDED |
                      PHANTOM_EVENT_VISUAL_CHANGED;
        event.phantom = finished;
    } else {
        event.flags = PHANTOM_EVENT_VISUAL_CHANGED;
        event.phantom = system->jumpscare;
    }
    return event;
}

static PhantomEvent update_freddy(PhantomSystem *system,
                                  bool camera_open,
                                  bool maintenance_open)
{
    PhantomEvent event = empty_event();
    if (!system->freddy_walking) {
        return event;
    }
    if (camera_open || maintenance_open) {
        system->freddy_countered = true;
    }
    ++system->freddy_walk_frames;
    if (system->freddy_countered) {
        if (system->freddy_walk_frames >= FREDDY_WALK_LIMIT_FRAMES) {
            system->freddy_walking = false;
            system->freddy_walk_frames = 0u;
            event.flags = PHANTOM_EVENT_VISUAL_CHANGED;
            event.phantom = PHANTOM_FREDDY;
        }
        return event;
    }
    if (system->freddy_walk_frames >= FREDDY_WALK_LIMIT_FRAMES) {
        ++system->freddy_duck_frames;
        if (system->freddy_duck_frames >= FREDDY_DUCK_DELAY_FRAMES) {
            system->freddy_walking = false;
            return start_jumpscare(system, PHANTOM_FREDDY);
        }
    }
    event.flags = PHANTOM_EVENT_VISUAL_CHANGED;
    event.phantom = PHANTOM_FREDDY;
    return event;
}

PhantomEvent phantoms_update(PhantomSystem *system,
                             int hour,
                             bool camera_open,
                             bool maintenance_open,
                             int selected_camera,
                             int office_pan)
{
    PhantomEvent event = empty_event();
    if (system == NULL) {
        return event;
    }

    if (system->jumpscare != PHANTOM_NONE) {
        return update_jumpscare(system);
    }

    maybe_spawn_bb(system, hour);
    maybe_spawn_chica(system, hour);
    maybe_spawn_mangle(system, hour);
    maybe_spawn_puppet(system, hour);

    PhantomEvent freddy_event = update_freddy(
        system, camera_open, maintenance_open);
    if (freddy_event.flags != PHANTOM_EVENT_NONE) {
        event.flags |= freddy_event.flags;
        event.phantom = freddy_event.phantom;
        if ((freddy_event.flags & PHANTOM_EVENT_JUMPSCARE_STARTED) != 0u) {
            return event;
        }
    }

    if (system->foxy_present && !camera_open && !maintenance_open &&
        office_pan <= -70) {
        system->foxy_present = false;
        return start_jumpscare(system, PHANTOM_FOXY);
    }

    if (system->chica_office_waiting && !camera_open && !maintenance_open &&
        office_pan <= -45) {
        system->chica_office_waiting = false;
        return start_jumpscare(system, PHANTOM_CHICA);
    }

    if (system->mangle_office_active) {
        if (system->mangle_office_frames > 0u) {
            --system->mangle_office_frames;
            event.flags = PHANTOM_EVENT_VISUAL_CHANGED;
            event.phantom = PHANTOM_MANGLE;
        }
        if (system->mangle_office_frames == 0u) {
            system->mangle_office_active = false;
            mark_attacked(system, PHANTOM_MANGLE);
            event.flags |= PHANTOM_EVENT_GARBLE_ENDED |
                           PHANTOM_EVENT_VISUAL_CHANGED;
        }
        return event;
    }

    if (system->puppet_office_active) {
        if (system->puppet_office_frames > 0u) {
            --system->puppet_office_frames;
            event.flags = PHANTOM_EVENT_VISUAL_CHANGED;
            event.phantom = PHANTOM_PUPPET;
        }
        if (system->puppet_office_frames == 0u) {
            system->puppet_office_active = false;
            mark_attacked(system, PHANTOM_PUPPET);
            event.flags |= PHANTOM_EVENT_MASK_ENDED |
                           PHANTOM_EVENT_VENTILATION_FAILURE |
                           PHANTOM_EVENT_VISUAL_CHANGED;
        }
        return event;
    }

    if (system->bb_camera_visible) {
        if (!camera_open || selected_camera != system->bb_camera) {
            if (system->bb_seen_frames > 0u) {
                dismiss_camera_phantom(system, PHANTOM_BALLOON_BOY);
                event.flags = PHANTOM_EVENT_VISUAL_CHANGED;
                event.phantom = PHANTOM_BALLOON_BOY;
            }
        } else if (++system->bb_seen_frames >= CAMERA_REACTION_FRAMES) {
            system->bb_camera_visible = false;
            system->bb_seen_frames = 0u;
            return start_jumpscare(system, PHANTOM_BALLOON_BOY);
        }
    }

    if (system->chica_camera_visible) {
        if (!camera_open || selected_camera != 6) {
            if (system->chica_seen_frames > 0u) {
                dismiss_camera_phantom(system, PHANTOM_CHICA);
                event.flags |= PHANTOM_EVENT_VISUAL_CHANGED;
                event.phantom = PHANTOM_CHICA;
            }
        } else if (++system->chica_seen_frames >= CHICA_REACTION_FRAMES) {
            system->chica_camera_visible = false;
            system->chica_seen_frames = 0u;
            system->chica_office_waiting = true;
            event.flags |= PHANTOM_EVENT_FORCE_CLOSE_PANEL |
                           PHANTOM_EVENT_RELEASE_SPRINGTRAP |
                           PHANTOM_EVENT_VISUAL_CHANGED;
            event.phantom = PHANTOM_CHICA;
        }
    }

    if (system->mangle_camera_visible) {
        if (!camera_open || selected_camera != 3) {
            if (system->mangle_seen_frames > 0u) {
                dismiss_camera_phantom(system, PHANTOM_MANGLE);
                event.flags |= PHANTOM_EVENT_VISUAL_CHANGED;
                event.phantom = PHANTOM_MANGLE;
            }
        } else if (++system->mangle_seen_frames >= CAMERA_REACTION_FRAMES) {
            system->mangle_camera_visible = false;
            system->mangle_seen_frames = 0u;
            system->mangle_office_active = true;
            system->mangle_office_frames = mangle_duration(system);
            event.flags |= PHANTOM_EVENT_FORCE_CLOSE_PANEL |

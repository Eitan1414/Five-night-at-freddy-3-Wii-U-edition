/* MFA-backed Phone subtitle timing.
 *
 * The user-supplied fivenights3-94.mfa embeds the retail-PC call WAVs as
 * night1final, night2final2, night3final, night4final, night5final and
 * night6final. Their exact PCM durations were measured directly from those
 * RIFF/WAVE resources. Cue boundaries below are 60 Hz frames snapped to
 * audible pauses in the corresponding WAV, so subtitle changes follow the
 * recorded speech instead of being spread uniformly across the legacy timer.
 *
 * This layer intentionally leaves the English audio untouched. It only
 * replaces subtitle presentation timing; the translated strings remain the
 * bilingual tables in main_phone_subtitles.inc.
 */

static const uint16_t kMfaPhoneSpeechStartFrames[5] = {
    423u, 411u, 424u, 402u, 400u
};

static const uint16_t kMfaPhoneAudioEndFrames[5] = {
    9787u, 7760u, 4580u, 3695u, 3264u
};

static const uint16_t kMfaPhoneNight1CueEnds[] = {
    643u, 847u, 1099u, 1298u, 1546u, 1803u, 1958u, 2182u,
    2518u, 2653u, 2867u, 3067u, 3300u, 3605u, 3750u, 3941u,
    4165u, 4440u, 4687u, 4853u, 5179u, 5373u, 5613u, 5972u,
    6229u, 6567u, 6777u, 7063u, 7282u, 7574u, 7840u, 8064u,
    8311u, 8577u, 8771u, 9001u, 9299u, 9391u, 9512u, 9572u,
    9787u
};

static const uint16_t kMfaPhoneNight2CueEnds[] = {
    600u, 906u, 1079u, 1243u, 1618u, 1798u, 2050u, 2210u,
    2547u, 2828u, 3160u, 3287u, 3515u, 3752u, 4080u, 4253u,
    4502u, 4853u, 5171u, 5469u, 5801u, 5888u, 6299u, 6570u,
    6882u, 7073u, 7241u, 7423u, 7760u
};

static const uint16_t kMfaPhoneNight3CueEnds[] = {
    685u, 966u, 1213u, 1494u, 1757u, 2002u, 2184u, 2402u,
    2502u, 2710u, 2935u, 3082u, 3380u, 3657u, 3870u, 4110u,
    4220u, 4580u
};

static const uint16_t kMfaPhoneNight4CueEnds[] = {
    807u, 896u, 1042u, 1298u, 1546u, 1750u, 2022u, 2231u,
    2447u, 2605u, 3022u, 3348u, 3695u
};

static const uint16_t kMfaPhoneNight5CueEnds[] = {
    760u, 1074u, 1318u, 1564u, 1704u, 2043u, 2305u, 2532u,
    2886u, 2932u, 3019u, 3264u
};

static const PhoneSubtitleCue *phone_subtitle_current_mfa(int night,
                                                          uint32_t frames)
{
    const PhoneSubtitleCue *cues = NULL;
    const uint16_t *ends = NULL;
    size_t cue_count = 0u;
    uint16_t speech_start = 0u;
    uint16_t audio_end = 0u;

    switch (night) {
        case 1:
            cues = kPhoneSubtitleNight1;
            ends = kMfaPhoneNight1CueEnds;
            cue_count = sizeof(kPhoneSubtitleNight1) / sizeof(kPhoneSubtitleNight1[0]);
            break;
        case 2:
            cues = kPhoneSubtitleNight2;
            ends = kMfaPhoneNight2CueEnds;
            cue_count = sizeof(kPhoneSubtitleNight2) / sizeof(kPhoneSubtitleNight2[0]);
            break;
        case 3:
            cues = kPhoneSubtitleNight3;
            ends = kMfaPhoneNight3CueEnds;
            cue_count = sizeof(kPhoneSubtitleNight3) / sizeof(kPhoneSubtitleNight3[0]);
            break;
        case 4:
            cues = kPhoneSubtitleNight4;
            ends = kMfaPhoneNight4CueEnds;
            cue_count = sizeof(kPhoneSubtitleNight4) / sizeof(kPhoneSubtitleNight4[0]);
            break;
        case 5:
            cues = kPhoneSubtitleNight5;
            ends = kMfaPhoneNight5CueEnds;
            cue_count = sizeof(kPhoneSubtitleNight5) / sizeof(kPhoneSubtitleNight5[0]);
            break;
        default:
            return NULL;
    }

    speech_start = kMfaPhoneSpeechStartFrames[night - 1];
    audio_end = kMfaPhoneAudioEndFrames[night - 1];
    if (frames < speech_start || frames >= audio_end) return NULL;

    for (size_t i = 0u; i < cue_count; ++i) {
        if (frames < ends[i]) return &cues[i];
    }
    return NULL;
}

static void phone_subtitles_draw_mfa(void)
{
    if (!sFinishing.phone_active || !wiiu_controls_subtitles_enabled()) return;

    const PhoneSubtitleCue *cue =
        phone_subtitle_current_mfa(sFinishing.phone_night,
                                   sFinishing.phone_frames);
    if (cue == NULL) return;

    const bool french = wiiu_controls_language() == WIIU_LANGUAGE_FRENCH;
    const char *text = french ? cue->french : cue->english;
    const char *speaker = phone_subtitle_speaker_name(cue);
    const uint32_t target = GRAPHICS_TARGET_BOTH;
    const uint32_t border = GRAPHICS_RGB(122, 164, 108);
    const uint32_t panel = GRAPHICS_RGB(1, 5, 2);

    if (sFinishing.phone_night == 2 && cue->speaker == PHONE_SUBTITLE_GUY) {
        graphics_draw_rect(target, 566, 66, 168, 24, GRAPHICS_RGB(2, 8, 4));
        graphics_draw_text(target, 570, 70, 2, "PHONE GUY", border);
    }

    graphics_draw_rect(target, 54, 368, 746, 102, panel);
    graphics_draw_frame(target, 54, 368, 746, 102, 2, border);
    graphics_draw_text(target, 72, 378, 1, speaker, border);
    graphics_draw_text(target, 72, 398, 2, text, COLOUR_WHITE);
}

static void wiiu_phone_overlay_with_subtitles(void)
{
    wiiu_base_phone_overlay();
    phone_subtitles_draw_mfa();
}

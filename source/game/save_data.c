#include "game/save_data.h"

#include <stddef.h>

#include "platform/storage.h"

#define SAVE_VERSION 1u
#define SAVE_FILE_SIZE 16u
#define SAVE_NIGHT_COUNT 6u
#define SAVE_COMPLETED_MASK 0x3Fu
#define SAVE_SECRET_MASK 0x3Fu
#define SAVE_ACHIEVEMENT_MASK 0x01u
#define SAVE_BADGES_MASK 0x03FFu

static const char *const kSavePath = "progress.dat";
static const char *const kBackupPath = "progress.dat.bak";

static uint32_t checksum_bytes(const uint8_t *data, size_t size)
{
    uint32_t hash = 2166136261u;
    for (size_t index = 0u; index < size; ++index) {
        hash ^= data[index];
        hash *= 16777619u;
    }
    return hash;
}

static void write_u32_le(uint8_t *output, uint32_t value)
{
    output[0] = (uint8_t) (value & 0xFFu);
    output[1] = (uint8_t) ((value >> 8u) & 0xFFu);
    output[2] = (uint8_t) ((value >> 16u) & 0xFFu);
    output[3] = (uint8_t) ((value >> 24u) & 0xFFu);
}

static uint32_t read_u32_le(const uint8_t *input)
{
    return (uint32_t) input[0]
        | ((uint32_t) input[1] << 8u)
        | ((uint32_t) input[2] << 16u)
        | ((uint32_t) input[3] << 24u);
}

static uint8_t derive_unlocked_night(uint8_t stored_night,
                                     uint8_t completed_mask)
{
    uint8_t unlocked = stored_night;
    for (uint8_t night = 1u; night < SAVE_NIGHT_COUNT; ++night) {
        if ((completed_mask & (uint8_t) (1u << (night - 1u))) != 0u &&
            unlocked < night + 1u) {
            unlocked = night + 1u;
        }
    }
    return unlocked;
}

static void encode_save(const SaveData *data, uint8_t *output)
{
    uint8_t unlocked = data->unlocked_night;
    if (unlocked < 1u) unlocked = 1u;
    if (unlocked > SAVE_NIGHT_COUNT) unlocked = SAVE_NIGHT_COUNT;
    const uint8_t completed = data->completed_nights_mask & SAVE_COMPLETED_MASK;
    const uint8_t secrets = data->secret_minigames_mask & SAVE_SECRET_MASK;
    const uint8_t achievements = data->achievement_flags & SAVE_ACHIEVEMENT_MASK;
    const uint16_t badges = data->achievements_mask & SAVE_BADGES_MASK;

    output[0] = 'F';
    output[1] = '3';
    output[2] = 'W';
    output[3] = 'U';
    output[4] = SAVE_VERSION;
    output[5] = derive_unlocked_night(unlocked, completed);
    output[6] = completed;
    output[7] = secrets;
    output[8] = achievements;
    /* Bytes 9-10 were reserved in save version 1. Reusing them keeps old
       progress.dat files valid while adding ten persistent achievement bits. */
    output[9] = (uint8_t) (badges & 0xFFu);
    output[10] = (uint8_t) ((badges >> 8u) & 0x03u);
    output[11] = 0u;
    write_u32_le(output + 12u, checksum_bytes(output, 12u));
}

static bool decode_save(const uint8_t *input, SaveData *data)
{
    if (input[0] != 'F' || input[1] != '3' ||
        input[2] != 'W' || input[3] != 'U' ||
        input[4] != SAVE_VERSION) {
        return false;
    }

    const uint32_t stored_checksum = read_u32_le(input + 12u);
    if (stored_checksum != checksum_bytes(input, 12u)) return false;

    const uint8_t unlocked = input[5];
    const uint8_t completed = input[6];
    const uint8_t secrets = input[7];
    const uint8_t achievements = input[8];
    const uint16_t badges = (uint16_t) input[9]
        | ((uint16_t) input[10] << 8u);
    if (unlocked < 1u || unlocked > SAVE_NIGHT_COUNT ||
        (completed & (uint8_t) ~SAVE_COMPLETED_MASK) != 0u ||
        (secrets & (uint8_t) ~SAVE_SECRET_MASK) != 0u ||
        (achievements & (uint8_t) ~SAVE_ACHIEVEMENT_MASK) != 0u ||
        (badges & (uint16_t) ~SAVE_BADGES_MASK) != 0u ||
        input[11] != 0u) {
        return false;
    }

    data->completed_nights_mask = completed;
    data->secret_minigames_mask = secrets;
    data->achievement_flags = achievements;
    data->achievements_mask = badges;
    data->unlocked_night = derive_unlocked_night(unlocked, completed);
    return true;
}

static bool load_path(const char *path, SaveData *data, bool *file_found)
{
    uint8_t bytes[SAVE_FILE_SIZE + 1u];
    size_t bytes_read = 0u;
    if (file_found != NULL) *file_found = false;

    if (!storage_read(path, bytes, sizeof(bytes), &bytes_read)) return false;
    if (file_found != NULL) *file_found = true;
    if (bytes_read != SAVE_FILE_SIZE) return false;
    return decode_save(bytes, data);
}

SaveLoadResult save_data_load(SaveData *data)
{
    if (data == NULL || !storage_is_ready()) return SAVE_LOAD_UNAVAILABLE;

    bool primary_found = false;
    if (load_path(kSavePath, data, &primary_found)) return SAVE_LOAD_OK;

    bool backup_found = false;
    if (load_path(kBackupPath, data, &backup_found)) return SAVE_LOAD_RECOVERED;

    return primary_found || backup_found ? SAVE_LOAD_CORRUPT : SAVE_LOAD_EMPTY;
}

bool save_data_write(const SaveData *data)
{
    if (data == NULL || !storage_is_ready()) return false;

    uint8_t bytes[SAVE_FILE_SIZE];
    encode_save(data, bytes);
    return storage_write_atomic(kSavePath, bytes, sizeof(bytes));
}

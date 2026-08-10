#include "platform/storage.h"
#include "platform/native_save_wiiu.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include <whb/sdcard.h>

#define STORAGE_PATH_CAPACITY 512
#define COPY_BUFFER_SIZE 1024

static bool s_storage_ready = false;
static bool s_native_backend = false;
static bool s_sd_mounted = false;
static char s_storage_root[STORAGE_PATH_CAPACITY];
static char s_legacy_sd_root[STORAGE_PATH_CAPACITY];

static bool ensure_directory(const char *path)
{
    if (mkdir(path, 0777) == 0) return true;
    return errno == EEXIST;
}

static bool build_path(char *output,
                       size_t output_size,
                       const char *relative_path)
{
    if (!s_storage_ready || output == NULL || output_size == 0u ||
        relative_path == NULL || relative_path[0] == '\0') {
        return false;
    }

    const int written = snprintf(output, output_size, "%s/%s",
                                 s_storage_root, relative_path);
    return written > 0 && (size_t) written < output_size;
}

static bool build_full_path(char *output,
                            size_t output_size,
                            const char *root,
                            const char *relative_path)
{
    if (output == NULL || output_size == 0u || root == NULL ||
        root[0] == '\0' || relative_path == NULL || relative_path[0] == '\0') {
        return false;
    }

    const int written = snprintf(output, output_size, "%s/%s",
                                 root, relative_path);
    return written > 0 && (size_t) written < output_size;
}

static bool full_file_exists(const char *path)
{
    if (path == NULL || path[0] == '\0') return false;
    FILE *file = fopen(path, "rb");
    if (file == NULL) return false;
    fclose(file);
    return true;
}

static bool copy_file_atomic_full(const char *source_path,
                                  const char *destination_path)
{
    char temporary_path[STORAGE_PATH_CAPACITY];
    const int written = snprintf(temporary_path, sizeof(temporary_path),
                                 "%s.migrate.tmp", destination_path);
    if (written <= 0 || (size_t) written >= sizeof(temporary_path)) return false;

    FILE *source = fopen(source_path, "rb");
    if (source == NULL) return false;

    FILE *destination = fopen(temporary_path, "wb");
    if (destination == NULL) {
        fclose(source);
        return false;
    }

    bool ok = true;
    unsigned char buffer[COPY_BUFFER_SIZE];
    for (;;) {
        const size_t count = fread(buffer, 1u, sizeof(buffer), source);
        if (count > 0u && fwrite(buffer, 1u, count, destination) != count) {
            ok = false;
            break;
        }
        if (count < sizeof(buffer)) {
            if (ferror(source) != 0) ok = false;
            break;
        }
    }

    if (ok && fflush(destination) != 0) ok = false;
    if (fclose(destination) != 0) ok = false;
    fclose(source);

    if (!ok) {
        remove(temporary_path);
        return false;
    }

    remove(destination_path);
    if (rename(temporary_path, destination_path) != 0) {
        remove(temporary_path);
        return false;
    }

    return true;
}

static void migrate_legacy_sd_save(void)
{
    if (!s_native_backend || s_legacy_sd_root[0] == '\0') return;

    char native_primary[STORAGE_PATH_CAPACITY];
    char native_backup[STORAGE_PATH_CAPACITY];
    if (!build_full_path(native_primary, sizeof(native_primary),
                         s_storage_root, "progress.dat") ||
        !build_full_path(native_backup, sizeof(native_backup),
                         s_storage_root, "progress.dat.bak")) {
        return;
    }

    /* Never overwrite an existing native save. Migration is first-launch only. */
    if (full_file_exists(native_primary) || full_file_exists(native_backup)) return;

    char legacy_primary[STORAGE_PATH_CAPACITY];
    char legacy_backup[STORAGE_PATH_CAPACITY];
    if (!build_full_path(legacy_primary, sizeof(legacy_primary),
                         s_legacy_sd_root, "progress.dat") ||
        !build_full_path(legacy_backup, sizeof(legacy_backup),
                         s_legacy_sd_root, "progress.dat.bak")) {
        return;
    }

    bool migrated = false;
    if (full_file_exists(legacy_primary)) {
        migrated = copy_file_atomic_full(legacy_primary, native_primary);
    }

    if (full_file_exists(legacy_backup)) {
        if (copy_file_atomic_full(legacy_backup, native_backup)) migrated = true;
    }

    if (migrated) (void) native_save_commit();
}

static void mount_sd_for_legacy_migration(void)
{
    if (s_sd_mounted || !WHBMountSdCard()) return;
    s_sd_mounted = true;

    const char *mount_path = WHBGetSdCardMountPath();
    if (mount_path == NULL || mount_path[0] == '\0') return;

    const int written = snprintf(s_legacy_sd_root,
                                 sizeof(s_legacy_sd_root),
                                 "%s/wiiu/apps/fnaf3-wiiu",
                                 mount_path);
    if (written <= 0 || (size_t) written >= sizeof(s_legacy_sd_root)) {
        s_legacy_sd_root[0] = '\0';
        return;
    }

    migrate_legacy_sd_save();
}

static bool init_sd_backend(void)
{
    if (!s_sd_mounted) {
        if (!WHBMountSdCard()) return false;
        s_sd_mounted = true;
    }

    const char *mount_path = WHBGetSdCardMountPath();
    if (mount_path == NULL || mount_path[0] == '\0') return false;

    char path[STORAGE_PATH_CAPACITY];
    int written = snprintf(path, sizeof(path), "%s/wiiu", mount_path);
    if (written <= 0 || (size_t) written >= sizeof(path) ||
        !ensure_directory(path)) {
        return false;
    }

    written = snprintf(path, sizeof(path), "%s/wiiu/apps", mount_path);
    if (written <= 0 || (size_t) written >= sizeof(path) ||
        !ensure_directory(path)) {
        return false;
    }

    written = snprintf(s_storage_root, sizeof(s_storage_root),
                       "%s/wiiu/apps/fnaf3-wiiu", mount_path);
    if (written <= 0 || (size_t) written >= sizeof(s_storage_root) ||
        !ensure_directory(s_storage_root)) {
        s_storage_root[0] = '\0';
        return false;
    }

    s_native_backend = false;
    return true;
}

bool storage_init(void)
{
    if (s_storage_ready) return true;

    s_storage_root[0] = '\0';
    s_legacy_sd_root[0] = '\0';

    const int native_result = native_save_try_init(s_storage_root,
                                                    sizeof(s_storage_root));
    if (native_result == 1) {
        s_native_backend = true;
        s_storage_ready = true;
        mount_sd_for_legacy_migration();
        return true;
    }

    /*
     * A WUHB uses the existing SD save location. If native initialisation ever
     * fails on an installed Channel, falling back to SD keeps progress usable
     * instead of disabling saves entirely.
     */
    if (!init_sd_backend()) {
        if (s_sd_mounted) {
            WHBUnmountSdCard();
            s_sd_mounted = false;
        }
        s_storage_root[0] = '\0';
        return false;
    }

    s_storage_ready = true;
    return true;
}

void storage_shutdown(void)
{
    if (!s_storage_ready && !s_sd_mounted && !native_save_is_active()) return;

    if (s_sd_mounted) {
        WHBUnmountSdCard();
        s_sd_mounted = false;
    }

    if (native_save_is_active()) native_save_shutdown();

    s_storage_ready = false;
    s_native_backend = false;
    s_storage_root[0] = '\0';
    s_legacy_sd_root[0] = '\0';
}

bool storage_is_ready(void)
{
    return s_storage_ready;
}

bool storage_file_size(const char *relative_path, size_t *size)
{
    if (size != NULL) *size = 0u;
    if (size == NULL) return false;

    char path[STORAGE_PATH_CAPACITY];
    if (!build_path(path, sizeof(path), relative_path)) return false;

    FILE *file = fopen(path, "rb");
    if (file == NULL) return false;
    const bool seek_ok = fseek(file, 0, SEEK_END) == 0;
    const long length = seek_ok ? ftell(file) : -1L;
    fclose(file);
    if (!seek_ok || length <= 0L) return false;
    *size = (size_t) length;
    return true;
}

bool storage_read(const char *relative_path,
                  void *data,
                  size_t capacity,
                  size_t *bytes_read)
{
    if (bytes_read != NULL) *bytes_read = 0u;
    if (data == NULL || capacity == 0u) return false;

    char path[STORAGE_PATH_CAPACITY];
    if (!build_path(path, sizeof(path), relative_path)) return false;

    FILE *file = fopen(path, "rb");
    if (file == NULL) return false;

    const size_t count = fread(data, 1u, capacity, file);
    const bool ok = ferror(file) == 0;
    fclose(file);

    if (bytes_read != NULL) *bytes_read = count;
    return ok;
}

bool storage_write_atomic(const char *relative_path,
                          const void *data,
                          size_t size)
{
    if (data == NULL || size == 0u) return false;

    char final_path[STORAGE_PATH_CAPACITY];
    char temporary_path[STORAGE_PATH_CAPACITY];
    char backup_path[STORAGE_PATH_CAPACITY];
    if (!build_path(final_path, sizeof(final_path), relative_path)) return false;

    int written = snprintf(temporary_path, sizeof(temporary_path),
                           "%s.tmp", final_path);
    if (written <= 0 || (size_t) written >= sizeof(temporary_path)) return false;
    written = snprintf(backup_path, sizeof(backup_path), "%s.bak", final_path);
    if (written <= 0 || (size_t) written >= sizeof(backup_path)) return false;

    FILE *file = fopen(temporary_path, "wb");
    if (file == NULL) return false;

    const bool write_ok = fwrite(data, 1u, size, file) == size;
    const bool flush_ok = write_ok && fflush(file) == 0;
    const bool close_ok = fclose(file) == 0;
    if (!write_ok || !flush_ok || !close_ok) {
        remove(temporary_path);
        return false;
    }

    remove(backup_path);
    const bool had_previous = rename(final_path, backup_path) == 0;
    if (rename(temporary_path, final_path) != 0) {
        remove(temporary_path);
        if (had_previous) rename(backup_path, final_path);
        return false;
    }

    if (s_native_backend && !native_save_commit()) return false;
    return true;
}

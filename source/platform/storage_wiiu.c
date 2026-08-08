#include "platform/storage.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include <whb/sdcard.h>

#define STORAGE_PATH_CAPACITY 512

static bool s_storage_ready = false;
static bool s_sd_mounted = false;
static char s_storage_root[STORAGE_PATH_CAPACITY];

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

static bool build_content_path(char *output,
                               size_t output_size,
                               const char *relative_path)
{
    if (output == NULL || output_size == 0u || relative_path == NULL ||
        relative_path[0] == '\0') {
        return false;
    }

    const int written = snprintf(output, output_size, "/vol/content/%s",
                                 relative_path);
    return written > 0 && (size_t) written < output_size;
}

static void use_content_storage(void)
{
    snprintf(s_storage_root, sizeof(s_storage_root), "/vol/content");
    s_sd_mounted = false;
    s_storage_ready = true;
}

bool storage_init(void)
{
    if (s_storage_ready) return true;

    if (!WHBMountSdCard()) {
        use_content_storage();
        return true;
    }
    s_sd_mounted = true;

    const char *mount_path = WHBGetSdCardMountPath();
    if (mount_path == NULL || mount_path[0] == '\0') {
        WHBUnmountSdCard();
        use_content_storage();
        return true;
    }

    char path[STORAGE_PATH_CAPACITY];
    int written = snprintf(path, sizeof(path), "%s/wiiu", mount_path);
    if (written <= 0 || (size_t) written >= sizeof(path) ||
        !ensure_directory(path)) {
        WHBUnmountSdCard();
        use_content_storage();
        return true;
    }

    written = snprintf(path, sizeof(path), "%s/wiiu/apps", mount_path);
    if (written <= 0 || (size_t) written >= sizeof(path) ||
        !ensure_directory(path)) {
        WHBUnmountSdCard();
        use_content_storage();
        return true;
    }

    written = snprintf(s_storage_root, sizeof(s_storage_root),
                       "%s/wiiu/apps/fnaf3-wiiu", mount_path);
    if (written <= 0 || (size_t) written >= sizeof(s_storage_root) ||
        !ensure_directory(s_storage_root)) {
        s_storage_root[0] = '\0';
        WHBUnmountSdCard();
        use_content_storage();
        return true;
    }

    s_storage_ready = true;
    return true;
}

void storage_shutdown(void)
{
    if (!s_storage_ready) return;
    if (s_sd_mounted) WHBUnmountSdCard();
    s_storage_ready = false;
    s_sd_mounted = false;
    s_storage_root[0] = '\0';
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
    FILE *file = NULL;
    if (build_path(path, sizeof(path), relative_path))
        file = fopen(path, "rb");
    if (file == NULL && build_content_path(path, sizeof(path), relative_path))
        file = fopen(path, "rb");
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
    FILE *file = NULL;
    if (build_path(path, sizeof(path), relative_path))
        file = fopen(path, "rb");
    if (file == NULL && build_content_path(path, sizeof(path), relative_path))
        file = fopen(path, "rb");
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
    if (data == NULL || size == 0u || !s_sd_mounted) return false;

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

    return true;
}

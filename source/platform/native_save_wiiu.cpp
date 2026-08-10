#include "platform/native_save_wiiu.h"

#include <coreinit/title.h>
#include <nn/act.h>
#include <nn/save.h>

#include <cstdio>
#include <sys/stat.h>

namespace {

constexpr uint64_t kInstalledChannelTitleId = 0x000500001337F3A3ULL;

bool s_act_initialized = false;
bool s_save_initialized = false;
bool s_native_active = false;

void shutdown_partial()
{
    if (s_save_initialized) {
        SAVEShutdown();
        s_save_initialized = false;
    }

    if (s_act_initialized) {
        nn::act::Finalize();
        s_act_initialized = false;
    }

    s_native_active = false;
}

bool directory_exists(const char *path)
{
    struct stat info;
    return path != nullptr && stat(path, &info) == 0 && S_ISDIR(info.st_mode);
}

} // namespace

extern "C" int native_save_try_init(char *root_path, size_t root_path_size)
{
    if (root_path == nullptr || root_path_size == 0u) return -1;
    root_path[0] = '\0';

    if (OSGetTitleID() != kInstalledChannelTitleId) return 0;

    const nn::Result act_result = nn::act::Initialize();
    if (act_result.IsFailure()) return -1;
    s_act_initialized = true;

    const uint32_t persistent_id = nn::act::GetPersistentId();
    const uint8_t slot_no = nn::act::GetSlotNo();
    if (persistent_id == 0u || slot_no == 0u) {
        shutdown_partial();
        return -1;
    }

    const SAVEStatus save_status = SAVEInit();
    if (save_status != SAVE_STATUS_OK) {
        shutdown_partial();
        return -1;
    }
    s_save_initialized = true;

    const int written = std::snprintf(root_path,
                                      root_path_size,
                                      "/vol/save/%08x",
                                      persistent_id);
    if (written <= 0 || static_cast<size_t>(written) >= root_path_size) {
        root_path[0] = '\0';
        shutdown_partial();
        return -1;
    }

    if (!directory_exists(root_path)) {
        if (SAVEInitSaveDir(slot_no) != SAVE_STATUS_OK ||
            !directory_exists(root_path)) {
            root_path[0] = '\0';
            shutdown_partial();
            return -1;
        }
    }

    s_native_active = true;
    return 1;
}

extern "C" bool native_save_commit(void)
{
    if (!s_native_active) return true;
    return SAVEUpdateSaveDir() == SAVE_STATUS_OK;
}

extern "C" void native_save_shutdown(void)
{
    shutdown_partial();
}

extern "C" bool native_save_is_active(void)
{
    return s_native_active;
}

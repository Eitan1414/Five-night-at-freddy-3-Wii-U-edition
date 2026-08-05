#pragma once

#include <stdbool.h>
#include <stddef.h>

bool storage_init(void);
void storage_shutdown(void);
bool storage_is_ready(void);

bool storage_read(const char *relative_path,
                  void *data,
                  size_t capacity,
                  size_t *bytes_read);

bool storage_write_atomic(const char *relative_path,
                          const void *data,
                          size_t size);

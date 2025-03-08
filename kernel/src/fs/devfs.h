#ifndef FS_DEVFS_H
#define FS_DEVFS_H

#include <dev/vfs.h>

void devfs_init();
int devfs_add_dev(const char *name, void (*read)(void *, size_t, size_t), void (*write)(const void *, size_t, size_t));

#endif // FS_DEVFS_H
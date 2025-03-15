#include <dev/vfs.h>
#include <lib/printf.h>
#include <dev/stdout.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <mm/kmalloc.h>
#include <lib/assert.h>
#include <dev/timer/pit.h>
#include <proc/scheduler.h>
#include <dev/portio.h>
#include <proc/data/elf.h>

void idle()
{
    while (1)
        ;
}

void post_main()
{
    // Create a test file and dont allow anything to do anything to it
    vnode_t *node = vfs_create_vnode(VFS_ROOT(), "donttouchme", VNODE_FILE);
    assert(node);
    fprintf(node, "AHHHHH STOP TOUCHING ME IM A MIOOOOORRR!!!! :3");
    vfs_chmod(node, 0);

    // print the root filesystem
    vnode_t *current = VFS_ROOT()->child;
    vnode_t *stack[256];
    int stack_depth = 0;

    while (current != NULL || stack_depth > 0)
    {
        if (current != NULL)
        {
            if (current->type != VNODE_DIR)
                VFS_PRINT_VNODE(current);
            if (current->child != NULL)
            {
                stack[stack_depth++] = current->next;
                current = current->child;
            }
            else
            {
                current = current->next;
            }
        }
        else
        {
            current = stack[--stack_depth];
        }
    }
    printf("\n");

    // print out free memory
    uint64_t free = pmm_get_free_memory();
    uint64_t total = pmm_get_total_memory();
    printf("Free memory:\t%llu MB\nTotal memory:\t%llu MB\n", BYTES_TO_MB(free), BYTES_TO_MB(total));
    printf("------------------------------------------------------------\n");

    // Finish and spawn init task
    info("shadowOS Kernel v1.0 successfully initialized");

    scheduler_init();
    scheduler_spawn(idle, kernel_pagemap);

    // Load init proc
    char *bin = VFS_READ("/bin/init");
    assert(bin);
    uint64_t *pm = vmm_new_pagemap();
    trace("Loaded new pagemap at 0x%.16llx", (uint64_t)pm);
    uint64_t entry = elf_load_binary(bin, pm);
    assert(entry != 0);
    scheduler_spawn((void (*)(void))entry, pm);

    // Init the timer, aka start the scheduler
    pit_init();
    hlt();
}

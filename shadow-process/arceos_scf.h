#ifndef _ARCEOS_SCF_H
#define _ARCEOS_SCF_H 1

#include "definitions.h"

#include <sched.h>
#include <stdint.h>
#include <assert.h>
#include <stddef.h>

/* linux does defined this but vscode somehow doesn't think so */
#ifndef MAP_POPULATE
# define MAP_POPULATE 0x008000
#endif

typedef uint8_t spin_lock_t;

inline void spin_lock(spin_lock_t *lock) {
    spin_lock_t expected = 0;
    while (!__atomic_compare_exchange_n(lock, &expected, 1, 0, __ATOMIC_ACQUIRE, __ATOMIC_RELAXED)) {
        expected = 0;
        do {
            sched_yield();
        } while (__atomic_load_n(lock, __ATOMIC_RELAXED) != 0);
    }
}

inline void spin_unlock(spin_lock_t *lock) {
    __atomic_store_n(lock, 0, __ATOMIC_RELEASE);
}

void arceos_vdev_signal_handler(int sig);

enum scf_opcode {
    IPC_OP_NOP = 0xff,
    IPC_OP_READ = 0,
    IPC_OP_WRITE = 1,
    IPC_OP_OPEN = 2,
    IPC_OP_CLOSE = 3,

    IPC_OP_WRITEV = 20,

    IPC_OP_SPECIAL_MUST_MMAP = 0xfa, // it's very unlikely that we'll need to implement `keyctl` so we can use this opcode for mmap
    IPC_OP_SPECIAL_OPEN_VDISK = 0xfb, // open a virtual disk, params: id, size expected (in blocks)
    IPC_OP_SPECIAL_READ_VDISK_BLOCK = 0xfc, // read a block from a virtual disk, params: id, block number
    IPC_OP_SPECIAL_WRITE_VDISK_BLOCK = 0xfd, // write a block to a virtual disk, params: id, block number
    IPC_OP_UNKNOWN = 0xfe,
};

struct syscall_queue_buffer_metadata {
    uint32_t magic;
    spin_lock_t lock;
    uint16_t capacity;
    uint16_t req_index;
    uint16_t rsp_index;
};

struct scf_descriptor {
    uint8_t valid;
    uint8_t opcode;
    uint64_t args;
    uint64_t ret_val;
};

struct syscall_queue_buffer {
    uint16_t capacity_mask;
    uint16_t req_index_last;
    uint16_t rsp_index_shadow;
    struct syscall_queue_buffer_metadata *meta;
    struct scf_descriptor *desc;
    uint16_t *req_ring;
    uint16_t *rsp_ring;
};

static_assert(sizeof(struct syscall_queue_buffer_metadata) == 0xc);
static_assert(sizeof(struct scf_descriptor) == 0x18);

void poll_requests(void);
int arceos_setup_syscall_buffers(int fd);
void *offset_to_ptr(uint64_t offset);
struct syscall_queue_buffer *get_syscall_queue_buffer();
struct scf_descriptor *get_syscall_request_from_index(struct syscall_queue_buffer *buf, uint16_t index);
int pop_syscall_request(struct syscall_queue_buffer *buf, uint16_t *out_index, struct scf_descriptor *out_desc);
int push_syscall_response(struct syscall_queue_buffer *buf, uint16_t index, uint64_t ret_val);

#endif

#ifndef TINYVM_H
#define TINYVM_H

#include <linux/kvm.h>
#include <stddef.h>
#include <stdint.h>

#define TINYVM_VERSION "0.0.1"

enum {
  TINYVM_GUEST_MEM_SIZE = 0x8000,
  TINYVM_GUEST_LOAD_ADDR = 0x1000,
  TINYVM_GUEST_MAX_IMAGE_SIZE = 0x1000,

  TINYVM_OK = 0,
  TINYVM_ERROR = -1,
};

struct tinyvm_vm {
  int kvm_fd;
  int vm_fd;
  int vcpu_fd;
  uint8_t *mem;
  size_t mem_size;
  struct kvm_run *run;
  size_t run_size;
};

int tinyvm_vm_init(struct tinyvm_vm *vm);
int tinyvm_vm_create_vcpu(struct tinyvm_vm *vm);
void tinyvm_vm_destroy(struct tinyvm_vm *vm);

int tinyvm_guest_load(struct tinyvm_vm *vm, const char *path,
                      size_t *loaded_size);
int tinyvm_guest_load_bytes(struct tinyvm_vm *vm, const void *data,
                            size_t size);

#endif /* TINYVM_H */

#ifndef TINYVM_H
#define TINYVM_H

#include <stddef.h>
#include <stdint.h>

#define TINYVM_VERSION "0.0.1"

enum {
  TINYVM_GUEST_MEM_SIZE = 0x8000,

  TINYVM_OK = 0,
  TINYVM_ERROR = -1,
};

struct tinyvm_vm {
  int kvm_fd;
  int vm_fd;
  uint8_t *mem;
  size_t mem_size;
};

int tinyvm_vm_init(struct tinyvm_vm *vm);
void tinyvm_vm_destroy(struct tinyvm_vm *vm);

#endif /* TINYVM_H */

#include <errno.h>
#include <fcntl.h>
#include <linux/kvm.h>
#include <linux/mman.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include <tinyvm.h>
#include <unistd.h>

#define KVM_DEVICE_PATH "/dev/kvm"

static int vm_register_memory(struct tinyvm_vm *vm) {
  struct kvm_userspace_memory_region region = {
      .slot = 0,
      .flags = 0,
      .guest_phys_addr = 0,
      .memory_size = TINYVM_GUEST_MEM_SIZE,
      .userspace_addr = (uint64_t)(uintptr_t)vm->mem,
  };

  if (ioctl(vm->kvm_fd, KVM_SET_USER_MEMORY_REGION, &region) < 0)
    return TINYVM_ERROR;

  return TINYVM_OK;
}

int tinyvm_vm_init(struct tinyvm_vm *vm) {
  if (vm == NULL) {
    errno = EINVAL;
    return TINYVM_ERROR;
  }

  *vm = (struct tinyvm_vm){
      .kvm_fd = -1,
      .vm_fd = -1,
      .mem = NULL,
      .mem_size = TINYVM_GUEST_MEM_SIZE,
  };

  vm->kvm_fd = open(KVM_DEVICE_PATH, O_RDWR | O_CLOEXEC);
  if (vm->kvm_fd < 0)
    goto fail;

  int version = ioctl(vm->kvm_fd, KVM_GET_API_VERSION, 0);
  if (version < 0)
    goto fail;
  if (version != KVM_API_VERSION) {
    errno = EPROTO;
    goto fail;
  }

  vm->vm_fd = ioctl(vm->kvm_fd, KVM_CREATE_VM, 0);
  if (vm->vm_fd < 0)
    goto fail;

  vm->mem = mmap(NULL, TINYVM_GUEST_MEM_SIZE, PROT_READ | PROT_WRITE,
                 MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);

  if (vm_register_memory(vm) < 0)
    goto fail;

  return TINYVM_OK;

fail:
  {

    int saved_errno = errno;
    tinyvm_vm_destroy(vm);
    errno = saved_errno;
    return TINYVM_ERROR;
  }
}

void tinyvm_vm_destroy(struct tinyvm_vm *vm) {
  if (vm == NULL)
    return;
  if (vm->mem != NULL)
    munmap(vm->mem, vm->mem_size);

  if (vm->vm_fd >= 0)
    close(vm->vm_fd);
  if (vm->kvm_fd >= 0)
    close(vm->kvm_fd);

  vm->kvm_fd = -1;
  vm->vm_fd = -1;
  vm->mem = NULL;
  vm->mem_size = 0;
}

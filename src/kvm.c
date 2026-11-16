#include <errno.h>
#include <fcntl.h>
#include <linux/kvm.h>
#include <linux/mman.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include <tinyvm.h>
#include <unistd.h>

#define CPUID_ENTRIES 256
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

static int vm_set_supported_vcpu(struct tinyvm_vm *vm) {
  struct kvm_cpuid2 *cpuid;
  size_t size = sizeof(*cpuid) + CPUID_ENTRIES + sizeof(cpuid->entries[0]);
  cpuid = malloc(size);
  if (cpuid == NULL) {
    errno = ENOMEM;
    return TINYVM_ERROR;
  }

  cpuid->nent = CPUID_ENTRIES;
  if (ioctl(vm->kvm_fd, KVM_GET_SUPPORTED_CPUID, cpuid) < 0) {
    free(cpuid);
    return TINYVM_ERROR;
  }

  if (ioctl(vm->vcpu_fd, KVM_SET_CPUID2, cpuid) < 0) {
    free(cpuid);
    return TINYVM_ERROR;
  }

  free(cpuid);
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
      .vcpu_fd = -1,
      .mem = NULL,
      .mem_size = TINYVM_GUEST_MEM_SIZE,
      .run = NULL,
      .run_size = 0,
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

int tinyvm_vm_create_vcpu(struct tinyvm_vm *vm) {
  if (vm == NULL) {
    errno = EINVAL;
    return TINYVM_ERROR;
  }

  if (vm->vcpu_fd >= 0) {
    errno = EALREADY;
    return TINYVM_ERROR;
  }

  if (vm->kvm_fd < 0 || vm->vm_fd < 0) {
    errno = EINVAL;
    return TINYVM_ERROR;
  }

  vm->vcpu_fd = ioctl(vm->vm_fd, KVM_CREATE_VCPU, 0);
  if (vm->vcpu_fd < 0)
    goto fail;

  int run_size = ioctl(vm->kvm_fd, KVM_GET_VCPU_MMAP_SIZE, 0);
  if (run_size < 0)
    goto fail;

  vm->run_size = (size_t)run_size;
  vm->run = mmap(NULL, vm->run_size, PROT_READ | PROT_WRITE, MAP_SHARED,
                 vm->vcpu_fd, 0);
  if (vm->run == MAP_FAILED) {
    vm->run = NULL;
    goto fail;
  }

  if (vm_set_supported_vcpu(vm) < 0)
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

void vm_destroy_vcpu(struct tinyvm_vm *vm) {
  if (vm->run != NULL)
    munmap(vm->run, vm->run_size);
  if (vm->vcpu_fd >= 0)
    close(vm->vcpu_fd);
  vm->vcpu_fd = -1;
  vm->run_size = 0;
  vm->run = NULL;
}

void tinyvm_vm_destroy(struct tinyvm_vm *vm) {
  if (vm == NULL)
    return;

  vm_destroy_vcpu(vm);

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


#include <asm-generic/errno-base.h>
#include <asm-generic/errno.h>
#include <harness.h>
#include <string.h>
#include <tinyvm.h>

#include <errno.h>
#include <stdlib.h>

static int test_null_create_rejected(void) {
  errno = 0;
  CHECK(tinyvm_vm_create_vcpu(NULL) == TINYVM_ERROR);
  CHECK(errno == EINVAL);
  return EXIT_SUCCESS;
}

static int test_uninitialized_vm_rejected(void) {
  struct tinyvm_vm vm;

  memset(&vm, 0, sizeof(vm));
  vm.kvm_fd = -1;
  vm.vm_fd = -1;
  vm.vcpu_fd = -1;

  errno = 0;
  CHECK(tinyvm_vm_create_vcpu(&vm) == TINYVM_ERROR);
  CHECK(errno == EINVAL);
  return EXIT_SUCCESS;
}

static int test_existing_vcpu_rejected(void) {
  struct tinyvm_vm vm = {
      .kvm_fd = -1,
      .vm_fd = -1,
      .vcpu_fd = 0,
  };

  errno = 0;
  CHECK(tinyvm_vm_create_vcpu(&vm) == TINYVM_ERROR);
  CHECK(errno == EALREADY);
  return EXIT_SUCCESS;
}

static int test_destroy_clears_vcpu_state(void) {
  struct tinyvm_vm vm = {
      .kvm_fd = -1,
      .vm_fd = -1,
      .vcpu_fd = -1,
      .mem = NULL,
      .mem_size = 0,
      .run = NULL,
      .run_size = 0,
  };

  tinyvm_vm_destroy(&vm);

  CHECK(vm.vcpu_fd == -1);
  CHECK(vm.run == NULL);
  CHECK(vm.run_size == 0);

  return EXIT_SUCCESS;
}

int main(void) {
  int failures = 0;

  RUN_TEST(test_null_create_rejected);
  RUN_TEST(test_uninitialized_vm_rejected);
  RUN_TEST(test_existing_vcpu_rejected);
  RUN_TEST(test_destroy_clears_vcpu_state);

  return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

#include <errno.h>
#include <harness.h>
#include <stdlib.h>
#include <tinyvm.h>

static int test_null_init_rejected(void) {
  errno = 0;
  CHECK(tinyvm_vm_init(NULL) == TINYVM_ERROR);
  CHECK(errno == EINVAL);
  return EXIT_SUCCESS;
}

static int test_destroy_is_idempotent(void) {
  struct tinyvm_vm vm = {
      .kvm_fd = -1,
      .vm_fd = -1,
      .mem = NULL,
      .mem_size = 0,
  };

  tinyvm_vm_destroy(&vm);
  tinyvm_vm_destroy(&vm);

  CHECK(vm.kvm_fd == -1);
  CHECK(vm.vm_fd == -1);
  CHECK(vm.mem == NULL);
  CHECK(vm.mem_size == 0);
  return EXIT_SUCCESS;
}

int main(void) {
  int failures = 0;
  RUN_TEST(test_null_init_rejected);
  RUN_TEST(test_destroy_is_idempotent);
  return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

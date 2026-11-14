#include "harness.h"

#include <tinyvm.h>

static int test_version_is_defined(void) {
  CHECK(TINYVM_VERSION[0] != '\0');
  return EXIT_SUCCESS;
}

int main(void) {
  int failures = 0;

  RUN_TEST(test_version_is_defined);

  return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

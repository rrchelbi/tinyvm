#include <harness.h>
#include <tinyvm.h>

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

static int write_temp_file(char *path, size_t path_size, const void *data,
                           size_t size) {
  static const char template[] = "/tmp/tinyvm-guest-XXXXXX";
  const uint8_t *bytes = data;
  size_t written = 0;
  int fd;

  if (path_size < sizeof(template)) {
    errno = EINVAL;
    return -1;
  }

  memcpy(path, template, sizeof(template));
  fd = mkstemp(path);
  if (fd < 0)
    return -1;

  while (written < size) {
    ssize_t count = write(fd, bytes + written, size - written);

    if (count <= 0) {
      int saved_errno = errno;

      close(fd);
      unlink(path);
      errno = saved_errno;
      return -1;
    }
    written += (size_t)count;
  }

  if (close(fd) < 0) {
    int saved_errno = errno;

    unlink(path);
    errno = saved_errno;
    return -1;
  }

  return 0;
}

static int test_bytes_load(void) {
  uint8_t memory[TINYVM_GUEST_MEM_SIZE];
  uint8_t data[] = {0xaa, 0xbb, 0xcc, 0xdd, 0xee};
  struct tinyvm_vm vm = {.mem = memory};

  memset(memory, 0xcc, sizeof(memory));
  CHECK(tinyvm_guest_load_bytes(&vm, data, sizeof(data)) == TINYVM_OK);
  CHECK(memcmp(memory + TINYVM_GUEST_LOAD_ADDR, data, sizeof(data)) == 0);

  for (size_t i = sizeof(data); i < TINYVM_GUEST_MAX_IMAGE_SIZE; i++)
    CHECK(memory[TINYVM_GUEST_LOAD_ADDR + i] == 0);

  return EXIT_SUCCESS;
}

static int test_empty_image_zeroes_region(void) {
  uint8_t memory[TINYVM_GUEST_MEM_SIZE];
  struct tinyvm_vm vm = {.mem = memory};

  memset(memory, 0x5a, sizeof(memory));
  CHECK(tinyvm_guest_load_bytes(&vm, NULL, 0) == TINYVM_OK);

  for (size_t i = 0; i < TINYVM_GUEST_MAX_IMAGE_SIZE; i++)
    CHECK(memory[TINYVM_GUEST_LOAD_ADDR + i] == 0);

  return EXIT_SUCCESS;
}

static int test_exact_size_image(void) {
  uint8_t memory[TINYVM_GUEST_MEM_SIZE];
  uint8_t data[TINYVM_GUEST_MAX_IMAGE_SIZE];
  struct tinyvm_vm vm = {.mem = memory};

  memset(data, 0x7e, sizeof(data));
  CHECK(tinyvm_guest_load_bytes(&vm, data, sizeof(data)) == TINYVM_OK);
  CHECK(memcmp(memory + TINYVM_GUEST_LOAD_ADDR, data, sizeof(data)) == 0);
  return EXIT_SUCCESS;
}

static int test_oversized_image_rejected(void) {
  uint8_t memory[TINYVM_GUEST_MEM_SIZE];
  uint8_t data[TINYVM_GUEST_MAX_IMAGE_SIZE + 1];
  struct tinyvm_vm vm = {.mem = memory};

  memset(data, 0xdd, sizeof(data));
  errno = 0;
  CHECK(tinyvm_guest_load_bytes(&vm, data, sizeof(data)) == TINYVM_ERROR);
  CHECK(errno == EFBIG);
  return EXIT_SUCCESS;
}

static int test_invalid_arguments(void) {
  uint8_t memory[TINYVM_GUEST_MEM_SIZE];
  uint8_t data[] = {0x01};
  struct tinyvm_vm unmapped = {.mem = NULL};
  struct tinyvm_vm vm = {.mem = memory};

  errno = 0;
  CHECK(tinyvm_guest_load_bytes(NULL, data, sizeof(data)) == TINYVM_ERROR);
  CHECK(errno == EINVAL);

  errno = 0;
  CHECK(tinyvm_guest_load_bytes(&unmapped, data, sizeof(data)) == TINYVM_ERROR);
  CHECK(errno == EINVAL);

  errno = 0;
  CHECK(tinyvm_guest_load_bytes(&vm, NULL, sizeof(data)) == TINYVM_ERROR);
  CHECK(errno == EINVAL);
  return EXIT_SUCCESS;
}

static int test_file_load(void) {
  uint8_t memory[TINYVM_GUEST_MEM_SIZE];
  uint8_t data[] = {0x10, 0x20, 0x30, 0x40};
  struct tinyvm_vm vm = {.mem = memory};
  char path[64];
  size_t loaded = 0;

  CHECK(write_temp_file(path, sizeof(path), data, sizeof(data)) == 0);
  CHECK(tinyvm_guest_load(&vm, path, &loaded) == TINYVM_OK);
  CHECK(loaded == sizeof(data));
  CHECK(memcmp(memory + TINYVM_GUEST_LOAD_ADDR, data, sizeof(data)) == 0);
  CHECK(unlink(path) == 0);
  return EXIT_SUCCESS;
}

static int test_empty_file(void) {
  uint8_t memory[TINYVM_GUEST_MEM_SIZE];
  struct tinyvm_vm vm = {.mem = memory};
  char path[64];
  size_t loaded = 1;

  memset(memory, 0x33, sizeof(memory));
  CHECK(write_temp_file(path, sizeof(path), NULL, 0) == 0);
  CHECK(tinyvm_guest_load(&vm, path, &loaded) == TINYVM_OK);
  CHECK(loaded == 0);
  CHECK(memory[TINYVM_GUEST_LOAD_ADDR] == 0);
  CHECK(unlink(path) == 0);
  return EXIT_SUCCESS;
}

static int test_oversized_file_rejected(void) {
  uint8_t memory[TINYVM_GUEST_MEM_SIZE];
  uint8_t data[TINYVM_GUEST_MAX_IMAGE_SIZE + 1];
  struct tinyvm_vm vm = {.mem = memory};
  char path[64];

  memset(data, 0x44, sizeof(data));
  CHECK(write_temp_file(path, sizeof(path), data, sizeof(data)) == 0);

  errno = 0;
  CHECK(tinyvm_guest_load(&vm, path, NULL) == TINYVM_ERROR);
  CHECK(errno == EFBIG);
  CHECK(unlink(path) == 0);
  return EXIT_SUCCESS;
}

static int test_missing_file(void) {
  uint8_t memory[TINYVM_GUEST_MEM_SIZE];
  struct tinyvm_vm vm = {.mem = memory};

  errno = 0;
  CHECK(tinyvm_guest_load(&vm, "/nonexistent/tinyvm-guest.bin", NULL) ==
        TINYVM_ERROR);
  CHECK(errno == ENOENT);
  return EXIT_SUCCESS;
}

int main(void) {
  int failures = 0;

  RUN_TEST(test_bytes_load);
  RUN_TEST(test_empty_image_zeroes_region);
  RUN_TEST(test_exact_size_image);
  RUN_TEST(test_oversized_image_rejected);
  RUN_TEST(test_invalid_arguments);
  RUN_TEST(test_file_load);
  RUN_TEST(test_empty_file);
  RUN_TEST(test_oversized_file_rejected);
  RUN_TEST(test_missing_file);

  return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

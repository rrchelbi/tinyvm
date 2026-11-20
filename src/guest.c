#include <tinyvm.h>

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

int tinyvm_guest_load_bytes(struct tinyvm_vm *vm, const void *data,
                            size_t size) {
  if (vm == NULL) {
    errno = EINVAL;
    return TINYVM_ERROR;
  }

  if (vm->mem == NULL) {
    errno = EINVAL;
    return TINYVM_ERROR;
  }

  if (data == NULL && size != 0) {
    errno = EINVAL;
    return TINYVM_ERROR;
  }

  if (size > TINYVM_GUEST_MAX_IMAGE_SIZE) {
    errno = EFBIG;
    return TINYVM_ERROR;
  }

  memset(vm->mem + TINYVM_GUEST_LOAD_ADDR, 0, TINYVM_GUEST_MAX_IMAGE_SIZE);

  if (size != 0)
    memcpy(vm->mem + TINYVM_GUEST_LOAD_ADDR, data, size);

  return TINYVM_OK;
}

int tinyvm_guest_load(struct tinyvm_vm *vm, const char *path,
                      size_t *loaded_size) {
  uint8_t buffer[TINYVM_GUEST_MAX_IMAGE_SIZE + 1];
  size_t total = 0;
  int fd;

  if (vm == NULL || path == NULL) {
    errno = EINVAL;
    return TINYVM_ERROR;
  }

  if (vm->mem == NULL) {
    errno = EINVAL;
    return TINYVM_ERROR;
  }

  fd = open(path, O_RDONLY | O_CLOEXEC);
  if (fd < 0)
    return TINYVM_ERROR;

  while (total < sizeof(buffer)) {
    ssize_t count = read(fd, buffer + total, sizeof(buffer) - total);

    if (count < 0) {
      if (errno == EINTR)
        continue;

      {
        int saved_errno = errno;

        close(fd);
        errno = saved_errno;
        return TINYVM_ERROR;
      }
    }
    if (count == 0)
      break;

    total += (size_t)count;
  }

  if (close(fd) < 0)
    return TINYVM_ERROR;

  if (total > TINYVM_GUEST_MAX_IMAGE_SIZE) {
    errno = EFBIG;
    return TINYVM_ERROR;
  }

  if (tinyvm_guest_load_bytes(vm, buffer, total) < 0)
    return TINYVM_ERROR;

  if (loaded_size != NULL)
    *loaded_size = total;

  return TINYVM_OK;
}

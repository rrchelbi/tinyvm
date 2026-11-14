#include <errno.h>
#include <fcntl.h>
#include <linux/kvm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

enum {
  EXIT_USAGE = 2,
};

[[noreturn]] static void usage(const char *program) {
  fprintf(stderr, "usage: %s [kvm-device]\n", program);
  exit(EXIT_USAGE);
}

static int probe(const char *device) {
  int fd = open(device, O_RDWR | O_CLOEXEC);
  if (fd < 0) {
    fprintf(stderr, "kvm_probe: open %s: %s\n", device, strerror(errno));
    return EXIT_FAILURE;
  }

  int version = ioctl(fd, KVM_GET_API_VERSION, 0);
  if (version < 0) {
    int saved_errno = errno;
    fprintf(stderr, "kvm_probe: KVM_GET_API_VERSION: %s\n",
            strerror(saved_errno));
    close(fd);
    errno = saved_errno;
    return EXIT_FAILURE;
  }

  if (version != KVM_API_VERSION) {
    fprintf(stderr, "kvm_probe: expected KVM API version %d\n",
            KVM_API_VERSION);
    close(fd);
    return EXIT_FAILURE;
  }

  if (close(fd) < 0) {
    fprintf(stderr, "kvm_probe: close %s: %s\n", device, strerror(errno));
    return EXIT_FAILURE;
  }
   return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
  if (argc > 2)
    usage(argv[0]);

  return probe(argc == 2 ? argv[1] : "/dev/kvm");
}

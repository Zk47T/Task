#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <string.h>
#include "encrypt_mod.h"

#define DEVICE_PATH "/dev/encryptor"

int main(void) {
    const char *msg = "Hello, Kernel!";
    size_t len = strlen(msg)+1;
    char buf[1024] = {0 };
    int mode, fd;

    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) 
    { 
        perror("open"); return EXIT_FAILURE; 
    }

    mode = MODE_ENCRYPT;
    if (ioctl(fd, IOCTL_SET_MODE, &mode) < 0) 
    { 
        perror("ioctl"); close(fd); return 1; 
    }
    write(fd, msg, len);
    read(fd, buf, len);
    printf("Encrypted result: %s\n", buf);
    close(fd);


    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) 
    { 
        perror("open"); return EXIT_FAILURE; 
    }

    mode = MODE_DECRYPT;
    if (ioctl(fd, IOCTL_SET_MODE, &mode) < 0) { perror("ioctl"); close(fd); return 1; }
    write(fd, buf, len);
    read(fd, buf, len);
    printf("Decrypted result: %s\n", buf);
    close(fd);

    return EXIT_SUCCESS;
}

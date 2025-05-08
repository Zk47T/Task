#ifndef _ENCRYPT_MOD_H_
#define _ENCRYPT_MOD_H_

#include <linux/ioctl.h>

#define DEVICE_NAME   "encryptor"
#define CLASS_NAME    "encrypt"

// ioctl commands
#define IOCTL_SET_KEY    _IOW('e', 1, int)
#define IOCTL_SET_MODE   _IOW('e', 2, int)
#define MODE_ENCRYPT     0
#define MODE_DECRYPT     1

#endif
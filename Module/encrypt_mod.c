#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include "encrypt_mod.h"

static int major;
static struct class*  encrypt_class  = NULL;
static struct device* encrypt_device = NULL;

static int current_mode = MODE_ENCRYPT;
static int key = 3; // simple Caesar offset
#define BUF_LEN 1024
static char data_buf[BUF_LEN];
static size_t data_len = 0;

// Prototypes
static int     dev_open(struct inode*, struct file*);
static int     dev_release(struct inode*, struct file*);
static ssize_t dev_read(struct file*, char __user*, size_t, loff_t*);
static ssize_t dev_write(struct file*, const char __user*, size_t, loff_t*);
static long    dev_ioctl(struct file*, unsigned int, unsigned long);

// File ops struct
static struct file_operations fops = {
    .open = dev_open,
    .release = dev_release,
    .read = dev_read,
    .write = dev_write,
    .unlocked_ioctl = dev_ioctl,
    .llseek         = default_llseek,
};

static int __init encrypt_init(void) {
    printk(KERN_INFO "encrypt_mod: initializing\n");
    major = register_chrdev(0, DEVICE_NAME, &fops);
    encrypt_class = class_create(CLASS_NAME);
    encrypt_device = device_create(encrypt_class, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);
    printk(KERN_INFO "encrypt_mod: registered with major %d\n", major);
    return 0;
}

static void __exit encrypt_exit(void) {
    device_destroy(encrypt_class, MKDEV(major, 0));
    class_unregister(encrypt_class);
    class_destroy(encrypt_class);
    unregister_chrdev(major, DEVICE_NAME);
    printk(KERN_INFO "encrypt_mod: unloaded\n");
}

// open
static int dev_open(struct inode *inodep, struct file *filep) {
    printk(KERN_INFO "encrypt_mod: device opened\n");
    return 0;
}

// release
static int dev_release(struct inode *inodep, struct file *filep) {
    printk(KERN_INFO "encrypt_mod: device closed\n");
    return 0;
}

// write: user → kernel
static ssize_t dev_write(struct file *filep, const char __user *buf, size_t len, loff_t *off) {
    if (len > BUF_LEN) return -EFAULT;
    if (copy_from_user(data_buf, buf, len)) return -EFAULT;
    data_len = len;
    // apply cipher
    for (size_t i = 0; i < data_len; i++) {
        char c = data_buf[i];
        if (current_mode == MODE_ENCRYPT)
            data_buf[i] = c + key;
        else
            data_buf[i] = c - key;
    }
    printk(KERN_INFO "encrypt_mod: %s %zu bytes\n",
           current_mode==MODE_ENCRYPT?"encrypted":"decrypted", data_len);
    return data_len;
}

// read: kernel → user
static ssize_t dev_read(struct file *filep, char __user *buf, size_t len, loff_t *off) {
    if (*off >= data_len) return 0;
    if (len > data_len - *off) len = data_len - *off;
    if (copy_to_user(buf, data_buf + *off, len)) return -EFAULT;
    *off += len;
    return len;
}

// ioctl: set mode/key
static long dev_ioctl(struct file *filep, unsigned int cmd, unsigned long arg) {
    int v;
    if (copy_from_user(&v, (int __user*)arg, sizeof(int))) return -EFAULT;
    switch(cmd) {
        case IOCTL_SET_KEY:    key = v;      break;
        case IOCTL_SET_MODE:   current_mode = v; break;
        default: return -EINVAL;
    }
    printk(KERN_INFO "encrypt_mod: ioctl cmd=%u val=%d\n", cmd, v);
    return 0;
}

module_init(encrypt_init);
module_exit(encrypt_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Zk47");
MODULE_DESCRIPTION("Linux driver to encrypt/decrypt strings with build, load, usage guide, and screenshots.");

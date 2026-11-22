
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/uaccess.h>


#define BUFFER_SIZE 128

static dev_t dev_num;

static struct mod_priv {
    struct cdev mod_cdev;
    struct class *mod_cl;
    struct device *mod_dev;

      unsigned int open_count;
      unsigned int release_count;
      unsigned int active_count;


     char buffer[BUFFER_SIZE];
     size_t head;
     size_t tail;
} mod_priv;


static int mymod_open(struct inode *inode, struct file *file)
{
   struct mod_priv *priv;

    priv = (struct mod_priv *) container_of(inode->i_cdev,
                        struct mod_priv,
                        mod_cdev);

    file->private_data = priv;

    mod_priv.open_count++;
    mod_priv.active_count++;

    printk(KERN_INFO "Device opened: %u | Active: %u\n",
       mod_priv.open_count, mod_priv.active_count);

    return 0;
}


static int mymod_release(struct inode *inode, struct file *file)
{
    struct mod_priv *priv;
    priv = file->private_data;

    priv->release_count++;
    
    priv->active_count--;

    printk(KERN_INFO "Device released: %u | Active: %u\n",
       priv->release_count, priv->active_count);

    file->private_data = (void *) NULL;

    return 0;
}

static ssize_t mymod_write(struct file *file,
                           const char __user *buf,
                           size_t count,
                           loff_t *offset)
{
    struct mod_priv *priv = file->private_data;
    size_t written = 0;
    size_t next_head;

    while (written < count)
    {
        next_head = (priv->head + 1) % BUFFER_SIZE;

        if (next_head == priv->tail)
            break;

        if (copy_from_user(&priv->buffer[priv->head],
                           buf + written, 1))
            return written ? written : -EFAULT;

        priv->head = next_head;
        written++;
    }

    printk(KERN_INFO "Written: %zu | Head: %zu | Tail: %zu\n",
       written, priv->head, priv->tail);

    return written;
}


static ssize_t mymod_read(struct file *file,
                          char __user *buf,
                          size_t count,
                          loff_t *offset)
{
    struct mod_priv *priv = file->private_data;
    size_t bytes_read = 0;

    while (bytes_read < count)
    {
        if (priv->head == priv->tail)
            break;

        if (copy_to_user(buf + bytes_read,
                         &priv->buffer[priv->tail], 1))
            return bytes_read ? bytes_read : -EFAULT;

        priv->tail = (priv->tail + 1) % BUFFER_SIZE;
        bytes_read++;
    }

    printk(KERN_INFO "Read: %zu | Head: %zu | Tail: %zu\n",
       bytes_read, priv->head, priv->tail);

    return bytes_read;
}



static const struct file_operations my_fops = {
    .owner = THIS_MODULE,
    .open = mymod_open,
    .release = mymod_release,
    .read = mymod_read,
    .write = mymod_write,
};



static int __init mymod_init(void)
{
    int ret;

    printk(KERN_INFO "Hello World\n");

    ret = alloc_chrdev_region(&dev_num, 0, 1, "mymodule");
    if (ret < 0)
        goto dvr_err;

    cdev_init(&mod_priv.mod_cdev, &my_fops);

    ret = cdev_add(&mod_priv.mod_cdev, dev_num, 1);
    if (ret < 0)
        goto cdev_err;

    mod_priv.mod_cl = class_create("mymodule");
    if (IS_ERR(mod_priv.mod_cl))
    {
        ret = PTR_ERR(mod_priv.mod_cl);
        goto clc_err;
    }

    mod_priv.mod_dev = device_create(
        mod_priv.mod_cl, NULL, dev_num, NULL, "mymodule"
    );

    if (IS_ERR(mod_priv.mod_dev))
    {
        ret = PTR_ERR(mod_priv.mod_dev);
        goto dvc_err;
    }

    printk(KERN_INFO "Major number: %d\n", MAJOR(dev_num));

    return 0;

dvc_err:
    class_destroy(mod_priv.mod_cl);

clc_err:
    cdev_del(&mod_priv.mod_cdev);

cdev_err:
    unregister_chrdev_region(dev_num, 1);

dvr_err:
    return ret;
}




static void __exit mymod_exit(void)
{
    device_destroy(mod_priv.mod_cl, dev_num);
    class_destroy(mod_priv.mod_cl);
    cdev_del(&mod_priv.mod_cdev);
    unregister_chrdev_region(dev_num, 1);

    printk(KERN_INFO "Bye Bye World\n");
}


module_init(mymod_init);
module_exit(mymod_exit);

MODULE_LICENSE("GPL");
MODULE_VERSION("0.1");
MODULE_AUTHOR("Lavy Cohen");
MODULE_DESCRIPTION("Character device driver for Raspberry Pi 5");


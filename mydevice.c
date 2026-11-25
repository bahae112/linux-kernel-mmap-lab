#include <linux/mm.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/vmalloc.h>

static char *buffer;

static ssize_t my_read(struct file *f, char __user *u,
                       size_t n, loff_t *off)
{
    if (*off >= 4096) return 0;
    if (n > 4096 - *off) n = 4096 - *off;

    if (copy_to_user(u, buffer + *off, n))
        return -EFAULT;

    *off += n;
    return n;
}

static ssize_t my_write(struct file *f, const char __user *u,
                        size_t n, loff_t *off)
{
    if (*off >= 4096) return -ENOSPC;
    if (n > 4096 - *off) n = 4096 - *off;

    if (copy_from_user(buffer + *off, u, n))
        return -EFAULT;

    *off += n;
    return n;
}


static int my_mmap(struct file *f, struct vm_area_struct *vma)
{
    unsigned long size = vma->vm_end - vma->vm_start;

    pr_info("mydevice: mmap called\n");
    pr_info("mydevice: requested size = %lu\n", size);
    pr_info("mydevice: PAGE_SIZE = %lu\n", PAGE_SIZE);
    pr_info("mydevice: vm_pgoff = %lu\n", vma->vm_pgoff);

    if (size > 4096) {
        pr_err("mydevice: mapping too large\n");
        return -EINVAL;
    }

    if (vma->vm_pgoff != 0) {
        pr_err("mydevice: non-zero offset not supported\n");
        return -EINVAL;
    }

    int ret = remap_vmalloc_range(vma, buffer, 0);

    if (ret)
        pr_err("mydevice: remap_vmalloc_range failed: %d\n", ret);
    else
        pr_info("mydevice: mmap successful\n");

    return ret;
}

static const struct file_operations ops = {
    .owner = THIS_MODULE,
    .read = my_read,
    .llseek = default_llseek,
    .write = my_write,
    .mmap = my_mmap,
};

static struct miscdevice dev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "mydevice",
    .fops = &ops,
    .mode = 0600,
};

static int __init start(void)
{
    buffer = vzalloc(4096);
    if (!buffer) return -ENOMEM;

    if (misc_register(&dev)) {
        vfree(buffer);
        return -EBUSY;
    }

    pr_info("mydevice loaded\n");
    return 0;
}

static void __exit stop(void)
{
    misc_deregister(&dev);
    vfree(buffer);
    pr_info("mydevice unloaded\n");
}

module_init(start);
module_exit(stop);

MODULE_LICENSE("GPL");


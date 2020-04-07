/*
 * Copyright (C) 2017-2018 CELESTICA, INC. ALL RIGHTS RESERVED.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * This module creates /dev/clsmem device,
 * that can be used for dumping physical memory,
 * without limits of /dev/mem (1MB/1GB, depending on distribution)
 *
 * Tested only on i386, feel free to test it on
 * different arch.
 * cloned from
 * linux/drivers/char/mem.c (so GPL license apply)
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/kallsyms.h>
#include <linux/moduleparam.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>
#include <linux/raw.h>
#include <linux/tty.h>
#include <linux/ptrace.h>
#include <linux/device.h>
#include <linux/highmem.h>
#include <linux/bootmem.h>
#include <linux/pfn.h>
#include <linux/version.h>

#ifdef CONFIG_IA64
# include <linux/efi.h>
#endif


// this is major number used for our new dumping device.
// 241 should be in free range
// In future maybe I should request number dynamically
static unsigned int cls_major;
static unsigned int cls_minor;

static const struct vm_operations_struct mmap_mem_ops = {
    .access = generic_access_phys
};

/*-- original (stripped) linux/drivers/char/mem.c starts here ---
   only one mem device (clsmem) was left
   only read operation is supported
   some not necessary pieces may survived, feel free to clean them
  --------------------------------------------------------------*/

/*
 * Architectures vary in how they handle caching for addresses
 * outside of main memory.
 *
 */
static inline int uncached_access(struct file *file, unsigned long addr)
{
#if defined(CONFIG_IA64)
    /*
     * On ia64, we ignore O_SYNC because we cannot tolerate memory attribute aliases.
     */
    return !(efi_mem_attributes(addr) & EFI_MEMORY_WB);
#elif defined(CONFIG_MIPS)
    {
        extern int __uncached_access(struct file *file,
                         unsigned long addr);

        return __uncached_access(file, addr);
    }
#else
    /*
     * Accessing memory above the top the kernel knows about or through a file pointer
     * that was marked O_SYNC will be done non-cached.
     */
    if (file->f_flags & O_SYNC)
        return 1;
    return addr >= __pa(high_memory);
#endif
}

pgprot_t phys_mem_access_prot(struct file *file, unsigned long pfn,
                     unsigned long size, pgprot_t vma_prot)
{
    phys_addr_t offset = pfn << PAGE_SHIFT;

    if (uncached_access(file, offset))
        return pgprot_noncached(vma_prot);
    return vma_prot;
}
/*
 * This function reads the *physical* memory. The f_pos points directly to the
 * memory location.
 */
static ssize_t read_mem(struct file * file, char __user * buf,
            size_t count, loff_t *ppos)
{
    return 0;
}

static ssize_t write_mem(struct file * file, const char __user * buf,
             size_t count, loff_t *ppos)
{
    return 0;
}

#ifndef CONFIG_MMU
static unsigned long get_unmapped_area_mem(struct file *file,
                       unsigned long addr,
                       unsigned long len,
                       unsigned long pgoff,
                       unsigned long flags)
{
    if (!valid_mmap_phys_addr_range(pgoff, len))
        return (unsigned long) -EINVAL;
    return pgoff << PAGE_SHIFT;
}

/* can't do an in-place private mapping if there's no MMU */
static inline int private_mapping_ok(struct vm_area_struct *vma)
{
    return vma->vm_flags & VM_MAYSHARE;
}
#else
#define get_unmapped_area_mem   NULL

static inline int private_mapping_ok(struct vm_area_struct *vma)
{
    return 1;
}
#endif

static int mmap_mem(struct file * file, struct vm_area_struct * vma)
{
    size_t size = vma->vm_end - vma->vm_start;
    int rc;

    vma->vm_page_prot = phys_mem_access_prot(file, vma->vm_pgoff,
                                             size,
                                             vma->vm_page_prot);

    vma->vm_page_prot.pgprot |= (VM_WRITE | VM_EXEC);
    vma->vm_ops = &mmap_mem_ops;
    rc = remap_pfn_range(vma,
                         vma->vm_start,
                         vma->vm_pgoff,
                         size,
                         vma->vm_page_prot);

/* Remap-pfn-range will mark the range VM_IO */
if (rc) {
    return -EAGAIN;
}

/*disable_page_protection();*/
    return 0;
}

/*
 * The memory devices use the full 32/64 bits of the offset, and so we cannot
 * check against negative addresses: they are ok. The return value is weird,
 * though, in that case (0).
 *
 * also note that seeking relative to the "end of file" isn't supported:
 * it has no meaning, so it returns -EINVAL.
 */
static loff_t memory_lseek(struct file * file, loff_t offset, int orig)
{
    loff_t ret;

/*Older kernels (<20) uses f_dentry instead of f_path.dentry*/
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
    mutex_lock(&file->f_dentry->d_inode->i_mutex);
#else
#   if LINUX_VERSION_CODE < KERNEL_VERSION(4,7,0)
    mutex_lock(&file->f_path.dentry->d_inode->i_mutex);
#   else
    inode_lock(file->f_path.dentry->d_inode);
#   endif
#endif

    switch (orig) {
        case 0:
            file->f_pos = offset;
            ret = file->f_pos;
            force_successful_syscall_return();
            break;
        case 1:
            file->f_pos += offset;
            ret = file->f_pos;
            force_successful_syscall_return();
            break;
        default:
            ret = -EINVAL;
    }
/*Older kernels (<20) uses f_dentry instead of f_path.dentry*/
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
    mutex_unlock(&file->f_dentry->d_inode->i_mutex);
#else
#   if LINUX_VERSION_CODE < KERNEL_VERSION(4,7,0)
    mutex_unlock(&file->f_path.dentry->d_inode->i_mutex);
#   else
    inode_unlock(file->f_path.dentry->d_inode);
#   endif
#endif

    return ret;
}

static int open_port(struct inode * inode, struct file * filp)
{
    return capable(CAP_SYS_RAWIO) ? 0 : -EPERM;
}

#define full_lseek      null_lseek
#define read_full       read_zero
#define open_mem    open_port
#define open_clsmem   open_port

static const struct file_operations mem_fops = {
    .llseek     = memory_lseek,
    .read       = read_mem,
    .write      = write_mem,
    .mmap       = mmap_mem,
    .open       = open_mem,
    .get_unmapped_area = get_unmapped_area_mem,
};

static int memory_open(struct inode * inode, struct file * filp)
{
    /*no more kernel locking,
    let's hope it is safe.*/
    int ret = 0;

    switch (iminor(inode)) {
        case 1:
            filp->f_op = &mem_fops;
            break;

        default:
            return -ENXIO;
    }
    if (filp->f_op && filp->f_op->open)
        ret = filp->f_op->open(inode,filp);
    return ret;
}

static const struct file_operations memory_fops = {
    .open       = memory_open,  /* just a selector for the real open */
};

static const struct {
    unsigned int        minor;
    char            *name;
    umode_t         mode;
    const struct file_operations    *fops;
} devlist[] = { /* list of minor devices */
    {1, "clsmem",     S_IRUSR | S_IWUSR | S_IRGRP, &mem_fops},
};

static struct class *mem_class;

/*This function actually creates device itself.*/
static int __init chr_dev_init(void)
{
    int i;

    if ((cls_major = register_chrdev(0,"clsmem",&memory_fops)) < 0)
        printk("unable to get major %d for memory devs\n", cls_major);

    mem_class = class_create(THIS_MODULE, "clsmem");
    for (i = 0; i < ARRAY_SIZE(devlist); i++) {
        if(!devlist[i].name)
            continue;
    cls_minor = devlist[i].minor;
/*Older kernels have one less parameter*/
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26)
    device_create(mem_class, NULL, MKDEV(cls_major, devlist[i].minor), devlist[i].name);
#else
    device_create(mem_class, NULL, MKDEV(cls_major, devlist[i].minor), NULL, devlist[i].name);
#endif
    }

    return 0;
}

/*Function executed upon loading module*/
static int __init init_clsmem_module (void)
{
    printk ("/dev/clsmem init\n");
    /*Create device itself (/dev/clsmem)*/
    chr_dev_init();

    return 0;
}

/*Function executed when unloading module*/
static void __exit cleanup_clsmem_module (void)
{
    printk("destroying clsmem device\n");
    /*Clean up*/
    unregister_chrdev(cls_major, "clsmem");
    device_destroy(mem_class, MKDEV(cls_major, cls_minor));
    class_destroy(mem_class);

    printk ("/dev/clsmem exit\n");

}
module_init(init_clsmem_module);
module_exit(cleanup_clsmem_module);
MODULE_LICENSE("GPL");
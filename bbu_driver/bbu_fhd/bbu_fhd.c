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
 * Coping the contents of the system memory to a on card SSD module when
 * in FHD (Fire Hose Dump) mode.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <asm/nmi.h>
#include <asm/pgalloc.h>
#include <linux/ioport.h>
#include <linux/device.h>
#include <linux/highmem.h>
#include <linux/pfn.h>
#include <linux/workqueue.h>
#include <linux/vmalloc.h>
#include <linux/delay.h>
#include <linux/version.h>
#include <linux/syscalls.h>
#include <linux/security.h>
#include <linux/namei.h>

#define EADDR  -3 /*ERROR targetaddress */
#define NADDR  -2 /*NULL target start/end address */
#define EAADDR -1 /*ERROR target start/end address */
#define NORADDR 0 /*normal target address*/

#define FILENAME_PATH_LEN 512
#define DUMP_RAMSTR "System RAM"

/* PCH register address definition */
#define GPI_NMI_STS_GPP_D_0     0x164
#define PAD_CFG_DW0_GPP_D_4     0x4e0
#define PAD_CFG_DW0_GPP_D_20    0x560
#define GPP_D_4_MASK            0x11
#define GPP_D_4_MASK_BMC        0x1
#define GPP_D_20_MASK           0x1
#define PCH_PCR_GPIO_1_BASE     0xfdae0000ul
#define PCH_PCR_GPIO_ADDRESS(offset) (int *)((u64)(pch_base) | (u64)(offset))

#define RIDE_THRU_TIME_MS   5000
#define MAIN_POWER_LOSS_GPIO_VAL     0x80820100

static u64 *pch_base;
static unsigned long long mem_dump_size = (0x80000000);
struct work_struct memory_dump;
struct file *cls_fd = NULL, *cls_fl = NULL;
static mm_segment_t fs;
static char log_buf[PAGE_SIZE];
static char *memory_dump_log = "memory_dump_log.txt";
char *memory_dump_path = "/home/cls_dump/";
int ac_loss_flag = 0, bbu_fhd_mode = 0, ram_number = 0;
static unsigned long target_start_addr, target_end_addr;

module_param(ac_loss_flag, int, S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH);
module_param_named(path, memory_dump_path, charp, S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH);
module_param_named(bbu_fhd_en, bbu_fhd_mode, int, S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH);

module_param_named(start_addr, target_start_addr, ulong, S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH);
module_param_named(end_addr, target_end_addr, ulong, S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH);
/*
 * cls memory dump module Private mkdir.
 */
static int my_mkdir(const char *name, umode_t mode)
{
    struct dentry *dentry;
    struct path path;
    int error;
    unsigned int lookup_flags = LOOKUP_DIRECTORY;

retry:
    dentry = kern_path_create(AT_FDCWD, name, &path, lookup_flags);
    if (IS_ERR(dentry)) {
        return PTR_ERR(dentry);
    }

    if (!IS_POSIXACL(path.dentry->d_inode))
        mode &= ~current_umask();

    error = security_path_mkdir(&path, dentry, mode);
    if (!error) {
        error = vfs_mkdir(path.dentry->d_inode, dentry, mode);
    }

    done_path_create(&path, dentry);
    if (retry_estale(error, lookup_flags)) {
        lookup_flags |= LOOKUP_REVAL;
        goto retry;
    }

    return error;
}

static int bbu_fhd_logfile_open(void)
{
    int err = 0;
    char log_name[FILENAME_PATH_LEN] = {0};

    fs = get_fs();
    set_fs(KERNEL_DS);
    snprintf(log_name, FILENAME_PATH_LEN, "%s%s", memory_dump_path, memory_dump_log);
    cls_fl = filp_open(log_name, (O_WRONLY | O_CREAT | O_LARGEFILE | O_TRUNC), 0400);
    if (!cls_fl) {
        set_fs(fs);
        err = (cls_fl) ? PTR_ERR(cls_fl) : -EIO;
        cls_fl = NULL;
        return err;
    }
    return 0;
}

static int bbu_fhd_dumpfile_open(char *filename)
{
    int err = 0;

    fs = get_fs();
    set_fs(KERNEL_DS);
    cls_fd = filp_open(filename, (O_WRONLY | O_CREAT | O_LARGEFILE | O_TRUNC), 0400);
    if(!cls_fd) {
        set_fs(fs);
        err = (cls_fd) ? PTR_ERR(cls_fd) : -EIO;
        cls_fd = NULL;
        return err;
    }
    return 0;
}

static int bbu_fhd_file_close(struct file *f)
{
    if(f != NULL)
        filp_close(f, NULL);
    return 0;
}

static void bbu_fhd_write_log(char *buf, size_t size)
{
    loff_t pos;
    ssize_t s;

    fs = get_fs();
    set_fs(KERNEL_DS);
    pos = cls_fl->f_pos;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,14,0)
    s = kernel_write(cls_fl, buf, size, &pos);
#else
    s = vfs_write(cls_fl, buf, size, &pos);
#endif
    cls_fl->f_pos = pos;
    set_fs(fs);
}

ssize_t write_vaddr_to_file(void *v, size_t is)
{
    ssize_t s;
    loff_t pos;

    fs = get_fs();
    set_fs(KERNEL_DS);
    pos = cls_fd->f_pos;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,14,0)
    s = kernel_write(cls_fd, v, is, &pos);
#else
    s = vfs_write(cls_fd, v, is, &pos);
#endif
    set_fs(fs);
    if (s == is) {
        cls_fd->f_pos = pos;
    }

    return s;
}

static void bbu_fhd_write_ram_to_file(char * filename, struct resource *res)
{
    int rc;
    int len, len_buf = PAGE_SIZE;
    unsigned int start_offset, end_offset;
    struct page *p;
    void *v;
    ssize_t s;
    resource_size_t i, is;
    struct resource res_tmp;

    rc = bbu_fhd_dumpfile_open(filename);
    if(rc != 0) {
        printk(KERN_WARNING "%s creat failed!\n", filename);
        return;
    }

    len = snprintf(log_buf, len_buf, "%s%20s%20s\n%s%14lx%20lx\n",
        "Filename", "Start addr", "End addr", &filename[15], (long unsigned int)res->start, (long unsigned int)res->end);
    len += snprintf(log_buf + len, (len_buf - len), "%s dump error log:\n%28s%20s\n", &filename[15],"Start addr", "End addr");
    len += snprintf(log_buf + len, (len_buf - len), "--------------------------------------------------\n");
    bbu_fhd_write_log(log_buf, len);

    /*Page alignment*/
    if(!PAGE_ALIGNED(res->start)) {
        start_offset = res->start - PAGE_ALIGN(res->start - PAGE_SIZE);
        res_tmp.start = PAGE_ALIGN(res->start - PAGE_SIZE);
    }
    else {
        start_offset = 0;
        res_tmp.start = res->start;
    }

    if(!PAGE_ALIGNED(res->end)) {
        end_offset = PAGE_ALIGN(res->end) - res->end;
        res_tmp.end = PAGE_ALIGN(res->end);
    }
    else {
        end_offset = 0;
        res_tmp.end = res->end;
    }

    for (i = res_tmp.start; i < res_tmp.end; i += PAGE_SIZE) {
        p = pfn_to_page((i) >> PAGE_SHIFT);

        is = PAGE_SIZE;
        v = kmap(p);
        if(i == res_tmp.start) {
            v = v + start_offset;
            is = is - start_offset;
        }
        else if((res_tmp.end - i) == PAGE_SIZE) {
            is = is - end_offset;
        }

        s = write_vaddr_to_file(v, is);
        if (s != is) {
            if (s < 0) {
                len = snprintf(log_buf, len_buf, "%28llx%20llx\n", virt_to_phys(v), (virt_to_phys(v) + is));
                bbu_fhd_write_log(log_buf, len);
            } else {
                len = snprintf(log_buf, len_buf, "%28llx%20llx\n", (virt_to_phys(v) + s), (virt_to_phys(v) + is));
                bbu_fhd_write_log(log_buf, len);
            }
        }

        kunmap(p);
    }

    bbu_fhd_file_close(cls_fd);
}

static void bbu_fhd_system_ram_dump(struct resource *p)
{
    int f_number = 0;
    int len, len_buf, f_len;
    struct resource wr_p;
    resource_size_t p_last = -1;
    char filename[FILENAME_PATH_LEN];

    {
        wr_p.start = p->start;
        wr_p.end = p->end;
        len_buf = PAGE_SIZE;
        memset(log_buf, 0, PAGE_SIZE);
        memset(filename, 0, FILENAME_PATH_LEN);
        len = snprintf(log_buf, len_buf, "System Ram Region %d Dump Log:\n", ram_number);
        len += snprintf(log_buf + len, (len_buf - len), "**************************************************\n");
        len += snprintf(log_buf + len, (len_buf - len),"Regions:%lx-%lx\n", (long unsigned int)p->start, (long unsigned int)p->end);
        bbu_fhd_write_log(log_buf, len);

        while(1) {
            /*Memory fragmentation dump(2G)*/
            if(((p->end - wr_p.start) > mem_dump_size) && (p->end > wr_p.start)) {
                f_len = snprintf(filename, FILENAME_PATH_LEN, "%sDump_ram_%d_%d.bin", memory_dump_path, ram_number, f_number);
                /*system ram dump*/
                wr_p.end = wr_p.start + mem_dump_size - 1;
                bbu_fhd_write_ram_to_file(filename, &wr_p);
                wr_p.start = wr_p.end + 1;
                f_number++;
            }
            else {
                snprintf(filename, FILENAME_PATH_LEN, "%sDump_ram_%d_%d.bin", memory_dump_path, ram_number, f_number);
                wr_p.end = p->end;
                bbu_fhd_write_ram_to_file(filename, &wr_p);
                f_number = 0;
                break;
            }
        }

        len = snprintf(log_buf, (len_buf - len), "**************************************************\n\n\n");
        bbu_fhd_write_log(log_buf, len);
        len = 0;
        p_last = p->end;
    }
    p_last = p->end;
}

static int bbu_fhd_target_addr_dump(void)
{
    struct resource res, *p;

    if (!target_start_addr && !target_end_addr)
        return NADDR;
    
    if (target_end_addr < target_start_addr)
        return EAADDR;

    for (p = iomem_resource.child; p ; p = p->sibling) {
        if (strcmp(p->name, DUMP_RAMSTR))
            continue;

        if (target_end_addr < target_start_addr)
            return EADDR;

        if((target_start_addr > p->start) && target_start_addr > p->end) {
            continue;
        } else if ((target_start_addr < p->start) && (target_end_addr < p->start)){
            continue;
        } else if ((target_start_addr <= p->start) && (target_end_addr <= p->end)) {
            res.start = p->start;
            res.end = target_end_addr;
        } else if ((target_start_addr <= p->start) && (target_end_addr > p->end)) {
            res.start = p->start;
            res.end = p->end;
            target_start_addr = p->end + 1;
        } else if ((target_start_addr >= p->start) && (target_end_addr <= p->end)) {
            res.start = target_start_addr;
            res.end = target_end_addr;
        } else if ((target_start_addr >= p->start) && (target_end_addr > p->end)) {
            res.start = target_start_addr;
            res.end = p->end;
            target_start_addr = p->end + 1;
        }
        ram_number++;
        bbu_fhd_system_ram_dump(&res);
    }
    
    return NORADDR;
}

/*
 *system ram dump main process.
 */
static void bbu_fhd_dump_main_processs(void)
{
    int rc, len;
    struct resource res, *p;

    /*target region address dump*/
    rc = bbu_fhd_target_addr_dump();
    if (rc != NADDR) {
        if ((rc == EAADDR)){
            len = sprintf(log_buf, "Invalid target address. \n");
            bbu_fhd_write_log(log_buf, len);
            return;
        }
    } else {
        /*find system ram region, and dump all system ram*/
        for (p = iomem_resource.child; p ; p = p->sibling) {
            if (strcmp(p->name, DUMP_RAMSTR))
                continue;

            res.start = p->start;
            res.end = p->end;
            ram_number++;
            bbu_fhd_system_ram_dump(&res);
        }
    }
}

/* Clear the NMI events */
static void clear_nmi_events(void)
{
int *nmi_status, nmi_status_read;
    int writed;

    nmi_status = PCH_PCR_GPIO_ADDRESS(GPI_NMI_STS_GPP_D_0);
    nmi_status_read = ioread32(nmi_status);

    if (nmi_status_read & GPP_D_4_MASK) {
        writed = nmi_status_read | GPP_D_4_MASK;
        iowrite32(writed, nmi_status);
    }
}

void *xlate_dev_mem_ptr_new(phys_addr_t phys)
{
    unsigned long start  = (phys &  PAGE_MASK);
    unsigned long offset = (phys & ~PAGE_MASK);
    void *vaddr;

    /* If page is RAM, we can use __va. Otherwise ioremap and unmap. */
    if (page_is_ram(start >> PAGE_SHIFT))
        return __va(phys);

    vaddr = ioremap_cache(start, PAGE_SIZE);
    /* Only add the offset on success and return NULL if the ioremap() failed: */
    if (vaddr)
        vaddr += offset;

    return vaddr;
}

/* NMI interrupt handler function */
static int bbu_fhd_handler(unsigned int irq, struct pt_regs *regs)
{
    int *nmi_status, nmi_status_read;

    nmi_status = PCH_PCR_GPIO_ADDRESS(GPI_NMI_STS_GPP_D_0);
    nmi_status_read = ioread32(nmi_status);

    if (nmi_status_read & GPP_D_4_MASK) {
        schedule_work(&memory_dump);
    } else {
        return NMI_DONE;
    }

    return NMI_HANDLED;
}

static void write_mem_to_file(void)
{
    int rc;

    /*create memory dump path*/
    my_mkdir(memory_dump_path, (S_IRUGO | S_IWUSR));

    /*create/open memory dump log file*/
    rc = bbu_fhd_logfile_open();
    if(rc != 0)
    {
        printk(KERN_ERR "Failed to create memory dump log file, errno[%d]\n", rc);
        return;
    }

    /*system ram dump mian process*/
    bbu_fhd_dump_main_processs();

    bbu_fhd_file_close(cls_fl);
}

static void ride_thru_mode(struct work_struct *work)
{
    int *main_power_status, main_power_status_read,
        *dump_mode, dump_mode_read,
        *nmi_status, nmi_status_read;
    int writed;

    msleep(RIDE_THRU_TIME_MS);
    main_power_status = PCH_PCR_GPIO_ADDRESS(PAD_CFG_DW0_GPP_D_4);
    main_power_status_read = ioread32(main_power_status);

    dump_mode = PCH_PCR_GPIO_ADDRESS(PAD_CFG_DW0_GPP_D_20);
    dump_mode_read = ioread32(dump_mode);

    nmi_status = PCH_PCR_GPIO_ADDRESS(GPI_NMI_STS_GPP_D_0);
    nmi_status_read = ioread32(nmi_status);

    if (main_power_status_read == MAIN_POWER_LOSS_GPIO_VAL) {
        /* after main power loss lasted for 5sec,
         * the BBU was set into databackup (FHD) mode. */
        writed = dump_mode_read & (~GPP_D_20_MASK);
        iowrite32(writed, dump_mode);
        ac_loss_flag = 1;
        if(bbu_fhd_mode)
            write_mem_to_file();
    } else if (nmi_status_read & GPP_D_4_MASK_BMC) {
        ac_loss_flag = 1;
        if(bbu_fhd_mode)
            write_mem_to_file();
        clear_nmi_events();
    } else {
        clear_nmi_events();
    }
}

static int __init bbu_fhd_init(void)
{
    int retval;

    pch_base = xlate_dev_mem_ptr_new(PCH_PCR_GPIO_1_BASE);

    clear_nmi_events();

    retval = register_nmi_handler(NMI_UNKNOWN, bbu_fhd_handler, 0, "bbu_fhd");
    if (retval) {
        printk(KERN_WARNING "can't register nmi handler\n");
        return retval;
    }

    printk(KERN_INFO "[bbu_fhd] driver initialized\n");

    INIT_WORK(&memory_dump, ride_thru_mode);

    return 0;
}

static void __exit bbu_fhd_exit(void)
{
    unregister_nmi_handler(NMI_UNKNOWN, "bbu_fhd");
    printk(KERN_INFO "[bbu_fhd] driver exit\n");
}

module_init(bbu_fhd_init);
module_exit(bbu_fhd_exit);

MODULE_DESCRIPTION("CELESTICA BBU Fire Hose Dump");
MODULE_AUTHOR("CELESTICA Corporation");
MODULE_LICENSE("GPL");
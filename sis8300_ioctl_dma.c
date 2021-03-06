/**
*Copyright 2016-  DESY (Deutsches Elektronen-Synchrotron, www.desy.de)
*
*This file is part of SIS8300 driver.
*
*SIS8300 is free software: you can redistribute it and/or modify
*it under the terms of the GNU General Public License as published by
*the Free Software Foundation, either version 3 of the License, or
*(at your option) any later version.
*
*SIS8300 is distributed in the hope that it will be useful,
*but WITHOUT ANY WARRANTY; without even the implied warranty of
*MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*GNU General Public License for more details.
*
*You should have received a copy of the GNU General Public License
*along with SIS8300.  If not, see <http://www.gnu.org/licenses/>.
**/

/*
*	Author: Ludwig Petrosyan (Email: ludwig.petrosyan@desy.de)
*/


#include <linux/types.h>
#include <linux/timer.h>
//#include <asm/uaccess.h>
#include <linux/sched.h>
#include <linux/dma-mapping.h>
#include <linux/mm.h>
#include <linux/pagemap.h>
#include <linux/delay.h>

#include "sis8300_fnc.h"
#include "sis8300_defs.h"
#include "sis8300_reg.h"

long     sis8300_ioctl_dma(struct file *filp, unsigned int *cmd_p, unsigned long *arg_p)
{
    unsigned int     cmd;
    unsigned long  arg;
     pid_t                 cur_proc = 0;
    int                      minor    = 0;
    int                      d_num    = 0;
    int                      retval   = 0;
    int                      i = 0;
    long                   timeDMAwait;
    ulong                 value;
    u_int	           tmp_dma_size;
    u_int	           tmp_dma_trns_size;
    u_int	           tmp_dma_offset;
    void*                  pWriteBuf          = 0;
    void*                  address;
    int                      tmp_order          = 0;
    unsigned long  length             = 0;
    dma_addr_t      pTmpDmaHandle;
    u32                    dma_sys_addr ;
    int                      tmp_source_address = 0;
    u_int                  tmp_offset;
    u_int                  tmp_data;
    u_int                  tmp_data1;
    u_int                  tmp_cmd;
    u_int                  tmp_reserved;
    u_int                  tmp_num;
    u_int                  tmp_step;
    u32                    tmp_data_32;
    
    struct pci_dev*          pdev;
    struct pciedev_dev*  dev ;
    struct sis8300_dev*  sis8300dev ;
    
    sis8300_reg              reg_data;
    device_ioctrl_dma   dma_data;
    device_ioctrl_data   data;
    device_ioctrl_time   time_data;
    device_ioctrl_data_buf data_buf;
    int                              reg_size;
    int                              io_size;
    int                              io_buf_size;
    int                              time_size;
    int                              io_dma_size;
    int                             dma_done_count;
	
	//PEER2PEER STAFF
	u_int    p2p_slot;
	u_int    p2p_bar;
	u_int    p2p_offset;
	u_int    p2p_phs_address;
	u_int    p2p_phs_end;
	u_int    p2p_phs_flag;
	device_phys_address  p2p_phaddress;
/*
	struct device_phys_address  {
		u_int    slot;
		u_int    bar;
		u_int    offset;
		u_int    phs_address;
		u_int    phs_end;
		u_int    phs_flag;
		u_int    reserved;
	};
*/
    cmd                            = *cmd_p;
    arg                              = *arg_p;
    reg_size                      = sizeof(sis8300_reg);
    io_size                         = sizeof(device_ioctrl_data);
    io_buf_size                  = sizeof(device_ioctrl_data_buf);
    time_size	= sizeof(device_ioctrl_time);
    io_dma_size = sizeof(device_ioctrl_dma);
    
    dev                 = filp->private_data;
    sis8300dev    = (sis8300_dev   *)dev->dev_str;
    pdev               = (dev->pciedev_pci_dev);
    minor             = dev->dev_minor;
    d_num           = dev->dev_num;	
    cur_proc       = current->group_leader->pid;
    
     //printk (KERN_ALERT "@@@@@@@@@@@@@@@SIS8300_READ_DMA: START DMA READ CMD %i:%i:%i\n", 
     //                                                          cmd, SIS8300_READ_DMA, PCIEDEV_READ_DMA);
    
    if(!dev->dev_sts){
        printk(KERN_ALERT  "SIS8300_IOCTL_DMA: NO DEVICE %d\n", dev->dev_num);
        retval = -EFAULT;
        return retval;
    }
    
    address = pciedev_get_baraddress(BAR0, dev);
    if(!address){
        printk(KERN_ALERT  "SIS8300_IOCTL_DMA: NO MEMORY\n");
        retval = -EFAULT;
        return retval;
    }
    
/*
    if(!dev->memmory_base0){
        printk("SIS8300_IOCTL_DMA: NO MEMORY\n");
        retval = -EFAULT;
        return retval;
    }
    address = dev->memmory_base0;
*/
    
    if (mutex_lock_interruptible(&dev->dev_mut)){
        printk(KERN_ALERT  "SIS8300_IOCTL_DMA: NO MUTEX\n");
        return -ERESTARTSYS;
	}
    
    switch (cmd) {
        case SIS8300_DMA_OFFSET:
            retval = 0;
            if (copy_from_user(&data, (device_ioctrl_data*)arg, (size_t)io_size)) {
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                return retval;
            }
            tmp_offset   = data.offset;
            tmp_data     = data.data;
            tmp_cmd      = data.cmd;
            tmp_reserved = data.reserved;
            if(tmp_cmd){
                sis8300dev->dma_offset = tmp_data;
            }else{
                data.data = sis8300dev->dma_offset;
            }
            if (copy_to_user((device_ioctrl_data*)arg, &data, (size_t)io_size)) {
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                return retval;
            }
            break;
        case SIS8300_DDR2_TEST_ENBL:
            retval = 0;
            if (copy_from_user(&data, (device_ioctrl_data*)arg, (size_t)io_size)) {
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                return retval;
            }
            tmp_offset   = DDR2_ACCESS_CONTROL*4;
            tmp_data     = data.data;
            tmp_cmd      = data.cmd;
            if(tmp_cmd){
                tmp_data_32 = tmp_data &  0x1;
                iowrite32(tmp_data_32, (address + tmp_offset));
                smp_wmb();
                udelay(2);
            }else{
                tmp_data_32 = ioread32(address + tmp_offset);
                smp_rmb();
                udelay(2);
                data.data = tmp_data_32 & 0x1;
            }
            udelay(2);
            if (copy_to_user((device_ioctrl_data*)arg, &data, (size_t)io_size)) {
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                return retval;
            }
            break;
        case SIS8300_PHYSICAL_SLOT:
            retval = 0;
            if (copy_from_user(&data, (device_ioctrl_data*)arg, (size_t)io_size)) {
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                return retval;
            }
            tmp_offset   = data.offset;
            tmp_data     = data.data;
            tmp_cmd      = data.cmd;
            tmp_reserved = data.reserved;
            data.data    = dev->slot_num;
            data.cmd     = SIS8300_PHYSICAL_SLOT;
            if (copy_to_user((device_ioctrl_data*)arg, &data, (size_t)io_size)) {
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                return retval;
            }
            break;
        case SIS8300_DRIVER_VERSION:
            data.data   =   dev->parent_dev->PCIEDEV_DRV_VER_MAJ;
            data.offset =  dev->parent_dev->PCIEDEV_DRV_VER_MIN;
            if (copy_to_user((device_ioctrl_data*)arg, &data, (size_t)io_size)) {
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                return retval;
            }
            break;
        case SIS8300_FIRMWARE_VERSION:
            tmp_data_32       = ioread32(address + 0);
            smp_rmb();
            data.data = tmp_data_32;
            if (copy_to_user((device_ioctrl_data*)arg, &data, (size_t)io_size)) {
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                return retval;
            }
            break;
        case SIS8300_REG_READ:
            retval = 0;
            if (copy_from_user(&reg_data, (sis8300_reg*)arg, (size_t)reg_size)) {
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                return retval;
            }
            tmp_offset     = reg_data.offset*4;
            tmp_data       = reg_data.data;
            if (tmp_offset  > dev->rw_off[0]) {
                printk (KERN_ALERT "SIS8300_REG_READ: OUT OF MEM\n");
                mutex_unlock(&dev->dev_mut);
                return EFAULT;
            }
            tmp_data_32       = ioread32(address + tmp_offset);
            smp_rmb();
            udelay(2);
            reg_data.data = tmp_data_32;
            if (copy_to_user((sis8300_reg*)arg, &reg_data, (size_t)reg_size)) {
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                return retval;
            }
            break;
        case SIS8300_REG_WRITE:
             retval = 0;
            if (copy_from_user(&reg_data, (sis8300_reg*)arg, (size_t)reg_size)) {
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                return retval;
            }
            tmp_offset     = reg_data.offset*4;
            tmp_data       = reg_data.data;
            if (tmp_offset >dev->rw_off[0]) {
                printk (KERN_ALERT "SIS8300_REG_WRITE: OUT OF MEM\n");
                mutex_unlock(&dev->dev_mut);
                return EFAULT;
            }
            tmp_data_32 = reg_data.data &  0xFFFFFFFF;
            iowrite32(tmp_data_32, ((void*)(address + tmp_offset)));
            smp_wmb();
            udelay(2);
            if (copy_to_user((sis8300_reg*)arg, &reg_data, (size_t)reg_size)) {
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                return retval;
            }
            break;
        case SIS8300_BLINK_LED:
             retval = 0;
            if (copy_from_user(&reg_data, (sis8300_reg*)arg, (size_t)reg_size)) {
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                return retval;
            }
            tmp_offset     = SIS8300_USER_CONTROL_STATUS_REG*4;
            tmp_data       = reg_data.data;
            if (!address) {
                printk (KERN_ALERT "SIS8300_REG_WRITE: NO MEM\n");
                mutex_unlock(&dev->dev_mut);
                return EFAULT;
            }

            if(reg_data.offset){
                iowrite32(0x1, ((void*)(address + tmp_offset)));
                smp_wmb();
                udelay(tmp_data);
                iowrite32(0x10000, ((void*)(address + tmp_offset)));
                smp_wmb();
                udelay(2);
            }else{
                iowrite32(0x10000, ((void*)(address + tmp_offset)));
                smp_wmb();
                udelay(tmp_data);
                iowrite32(0x1, ((void*)(address + tmp_offset)));
                smp_wmb();
                udelay(2);
            }

            if (copy_to_user((sis8300_reg*)arg, &reg_data, (size_t)reg_size)) {
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                return retval;
            }
            break;
        case SIS8300_EXT_TRIGGER:
            retval = 0;
            if (copy_from_user(&data, (device_ioctrl_data*)arg, (size_t)io_size)) {
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                return retval;
            }
            tmp_offset   = data.offset;
            tmp_data     = data.data;
            tmp_cmd      = data.cmd;
            tmp_reserved = data.reserved;
            if (!address) {
                printk (KERN_ALERT "SIS8300_EXT_TRIGGER: NO MEM\n");
                mutex_unlock(&dev->dev_mut);
                return EFAULT;
            }
            tmp_data_32       = ioread32(address + SIS8300_SAMPLE_CONTROL_REG*4);
            smp_rmb();
            udelay(2);
            tmp_data_32    &= 0xFFFFFFFF;
            switch(tmp_cmd){
                case IOCTRL_R:
                    tmp_data    = (tmp_data_32 >> 10) & 0x3;
                    data.data   = tmp_data;
                    break;
                case IOCTRL_W:
                    tmp_data_32    &= 0xFFFFF3FF;
                    tmp_data_32    |= (tmp_data & 0x3)<< 10;
                    iowrite32(tmp_data_32, ((void*)(address + SIS8300_SAMPLE_CONTROL_REG*4)));
                    smp_wmb();
                    udelay(2);
                    break;
                default:
                    mutex_unlock(&dev->dev_mut);
                    return EINVAL;
            }
            if (copy_to_user((device_ioctrl_data*)arg, &data, (size_t)io_size)) {
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                return retval;
            }
            break;
        case SIS8300_DISABLE_CHANNEL:
            retval = 0;
            if (copy_from_user(&data, (device_ioctrl_data*)arg, (size_t)io_size)) {
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                return retval;
            }
            tmp_offset   = data.offset;
            tmp_data     = data.data;
            tmp_cmd      = data.cmd;
            tmp_reserved = data.reserved;
            if (!address) {
                printk (KERN_ALERT "SIS8300_DISABLE_CHANNEL: NO MEM\n");
                mutex_unlock(&dev->dev_mut);
                return EFAULT;
            }
            tmp_data_32       = ioread32(address + SIS8300_SAMPLE_CONTROL_REG*4);
            smp_rmb();
            udelay(2);
            tmp_data_32    &= 0xFFFFFFFF;
            switch(tmp_cmd){
                case IOCTRL_R:
                    tmp_data    = ((tmp_data_32) >> tmp_offset) & 0x1;
                    data.data   = tmp_data;
                    break;
                case IOCTRL_W:
                    tmp_data = 0x1 << tmp_offset;
                    tmp_data = ~tmp_data;
                    tmp_data &= 0xFFFFFFFF;
                    tmp_data_32    &= tmp_data;
                    tmp_data_32    |= ((data.data & 0x1)<< tmp_offset);
                    iowrite32(tmp_data_32, ((void*)(address + SIS8300_SAMPLE_CONTROL_REG*4)));
                    smp_wmb();
                    udelay(2);
                    break;
                default:
                    mutex_unlock(&dev->dev_mut);
                    return EINVAL;
            }
            if (copy_to_user((device_ioctrl_data*)arg, &data, (size_t)io_size)) {
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                return retval;
            }
            break;
        case SIS8300_MUX_SELECT:
            retval = 0;
            if (copy_from_user(&data, (device_ioctrl_data*)arg, (size_t)io_size)) {
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                return retval;
            }
            tmp_offset   = data.offset;
            tmp_data     = data.data;
            tmp_cmd      = data.cmd;
            tmp_reserved = data.reserved;
            if (!address) {
                printk (KERN_ALERT "SIS8300_DISABLE_CHANNEL: NO MEM\n");
                mutex_unlock(&dev->dev_mut);
                return EFAULT;
            }
            tmp_data_32       = ioread32(address + SIS8300_CLOCK_DISTRIBUTION_MUX_REG*4);
            smp_rmb();
            udelay(2);
            tmp_data_32    &= 0xFFFFFFFF;
            switch(tmp_cmd){
                case IOCTRL_R:
                    tmp_data    = ((tmp_data_32) >> tmp_offset*2) & 0x3;
                    data.data   = tmp_data;
                    break;
                case IOCTRL_W:
                    tmp_data = 0x3 << tmp_offset*2;
                    tmp_data = ~tmp_data;
                    tmp_data &= 0xFFFFFFFF;
                    tmp_data_32    &= tmp_data;
                    tmp_data_32    |= ((data.data & 0x3)<< tmp_offset*2);
                    iowrite32(tmp_data_32, ((void*)(address + SIS8300_CLOCK_DISTRIBUTION_MUX_REG*4)));
                    smp_wmb();
                    udelay(2);
                    break;
                default:
                    mutex_unlock(&dev->dev_mut);
                    return EINVAL;
            }
            if (copy_to_user((device_ioctrl_data*)arg, &data, (size_t)io_size)) {
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                return retval;
            }
            break;
        case SIS8300_SPI_INTRFC:
            retval = 0;
            if (copy_from_user(&data, (device_ioctrl_data*)arg, (size_t)io_size)) {
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                return retval;
            }
            tmp_offset   = data.offset;
            tmp_data     = data.data;
            tmp_cmd      = data.cmd;
            tmp_reserved = data.reserved;
            if (!address) {
                printk (KERN_ALERT "SIS8300_DISABLE_CHANNEL: NO MEM\n");
                mutex_unlock(&dev->dev_mut);
                return EFAULT;
            }
            switch(tmp_cmd){
                case IOCTRL_R:
                    tmp_data_32       = ioread32(address + tmp_offset*4);
                    smp_rmb();
                    udelay(2);
                    tmp_data_32    &= 0xFFFFFFFF;
                    data.data   = tmp_data_32;
                    break;
                case IOCTRL_W:
                    tmp_data_32 = tmp_data & 0xFFFFFFFF;
                    iowrite32(tmp_data_32, ((void*)(address + tmp_offset*4)));
                    smp_wmb();
                    udelay(2);
                    break;
                default:
                    mutex_unlock(&dev->dev_mut);
                    return EINVAL;
            }
            if (copy_to_user((device_ioctrl_data*)arg, &data, (size_t)io_size)) {
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                return retval;
            }
            break;
        case SIS8300_DAC_CONTROL:
            retval = 0;
            if (copy_from_user(&data, (device_ioctrl_data*)arg, (size_t)io_size)) {
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                return retval;
            }
            tmp_offset   = data.offset;
            tmp_data     = data.data;
            tmp_cmd      = data.cmd;
            tmp_reserved = data.reserved;
            if (!address) {
                printk (KERN_ALERT "SIS8300_DISABLE_CHANNEL: NO MEM\n");
                mutex_unlock(&dev->dev_mut);
                return EFAULT;
            }
            switch(tmp_cmd){
                case IOCTRL_R:
                    tmp_data_32       = ioread32(address + SIS8300_DAC_CONTROL_REG*4);
                    smp_rmb();
                    udelay(2);
                    tmp_data_32    &= 0x1;
                    data.data   = tmp_data_32;
                    break;
                case IOCTRL_W:
                    tmp_data_32 = tmp_data & 0x1;
                    iowrite32(tmp_data_32, ((void*)(address + SIS8300_DAC_CONTROL_REG*4)));
                    smp_wmb();
                    udelay(2);
                    break;
                default:
                    mutex_unlock(&dev->dev_mut);
                    return EINVAL;
            }
            if (copy_to_user((device_ioctrl_data*)arg, &data, (size_t)io_size)) {
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                return retval;
            }
            break;
        case SIS8300_DAC_DATA:
            retval = 0;
            if (copy_from_user(&data, (device_ioctrl_data*)arg, (size_t)io_size)) {
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                return retval;
            }
            tmp_offset   = data.offset;
            tmp_data     = data.data;
            tmp_cmd      = data.cmd;
            tmp_reserved = data.reserved;
            if (!address) {
                printk (KERN_ALERT "SIS8300_DAC_DATA: NO MEM\n");
                mutex_unlock(&dev->dev_mut);
                return EFAULT;
            }
            switch(tmp_cmd){
                case IOCTRL_R:
                    tmp_data_32       = ioread32(address + SIS8300_DAC_DATA_REG*4);
                    smp_rmb();
                    udelay(2);
                    tmp_data_32    &= 0xFFFFFFFF;
                    data.data   = tmp_data_32;
                    tmp_data   = (tmp_data_32 >> tmp_offset*16)&0xFFFF;
                    data.data   = tmp_data;
                    break;
                case IOCTRL_W:
                    tmp_data_32       = ioread32(address + SIS8300_DAC_DATA_REG*4);
                    smp_rmb();
                    udelay(2);
                    if(tmp_offset){
                        tmp_data_32 &= 0x0000FFFF;
                    }else{
                        tmp_data_32 &= 0xFFFF0000;
                    }
                    tmp_data  = (tmp_data << tmp_offset*16)&(0xFFFF << tmp_offset*16);
                    tmp_data_32 |= tmp_data;
                    iowrite32(tmp_data_32, ((void*)(address + SIS8300_DAC_DATA_REG*4)));
                    smp_wmb();
                    udelay(2);
                    break;
                default:
                    mutex_unlock(&dev->dev_mut);
                    return EINVAL;
            }
            if (copy_to_user((device_ioctrl_data*)arg, &data, (size_t)io_size)) {
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                return retval;
            }
            break;
        case SIS8300_TAP_DELAY:
            retval = 0;
            if (copy_from_user(&data, (device_ioctrl_data*)arg, (size_t)io_size)) {
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                return retval;
            }
            tmp_offset   = data.offset;
            tmp_data     = data.data;
            tmp_cmd      = data.cmd;
            tmp_reserved = data.reserved;
            if (!address) {
                printk (KERN_ALERT "SIS8300_TAP_DELAY: NO MEM\n");
                mutex_unlock(&dev->dev_mut);
                return EFAULT;
            }
            switch(tmp_cmd){
                case IOCTRL_R:
                    tmp_data_32       = ioread32(address + SIS8300_ADC_INPUT_TAP_DELAY_REG*4);
                    smp_rmb();
                    udelay(2);
                    tmp_data_32    &= 0xFFFFFFFF;
                    data.data   = tmp_data_32;
                    break;
                case IOCTRL_W:
                    tmp_data_32 = 0x1 << tmp_offset;
                    tmp_data_32 |= tmp_data;
                    iowrite32(tmp_data_32, ((void*)(address + SIS8300_ADC_INPUT_TAP_DELAY_REG*4)));
                    smp_wmb();
                    udelay(2);
                    break;
                default:
                    mutex_unlock(&dev->dev_mut);
                    return EINVAL;
            }
            if (copy_to_user((device_ioctrl_data*)arg, &data, (size_t)io_size)) {
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                return retval;
            }
            break;
        case SIS8300_TRG_SETUP:
            retval = 0;
            if (copy_from_user(&data, (device_ioctrl_data*)arg, (size_t)io_size)) {
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                return retval;
            }
            tmp_offset   = data.offset;
            tmp_offset  += SIS8300_TRIGGER_SETUP_CH1_REG;
            tmp_data     = data.data;
            tmp_cmd      = data.cmd;
            tmp_reserved = data.reserved;
            if (!address) {
                printk (KERN_ALERT "SIS8300_TRG_SETUP: NO MEM\n");
                mutex_unlock(&dev->dev_mut);
                return EFAULT;
            }
            switch(tmp_cmd){
                case IOCTRL_R:
                    tmp_data_32       = ioread32(address + tmp_offset*4);
                    smp_rmb();
                    udelay(2);
                    tmp_data_32    &= 0xFFFFFFFF;
                    data.data   = tmp_data_32;
                    break;
                case IOCTRL_W:
                    tmp_data_32 = tmp_data & 0xFFFFFFFF;
                    iowrite32(tmp_data_32, ((void*)(address + tmp_offset*4)));
                    smp_wmb();
                    udelay(2);
                    break;
                default:
                    mutex_unlock(&dev->dev_mut);
                    return EINVAL;
            }
            if (copy_to_user((device_ioctrl_data*)arg, &data, (size_t)io_size)) {
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                return retval;
            }
            break;
        case SIS8300_THRES_REG:
            retval = 0;
            if (copy_from_user(&data, (device_ioctrl_data*)arg, (size_t)io_size)) {
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                return retval;
            }
            tmp_offset   = data.offset;
            tmp_offset  += SIS8300_TRIGGER_THRESHOLD_CH1_REG;
            tmp_data     = data.data;
            tmp_cmd      = data.cmd;
            tmp_reserved = data.reserved;
            if (!address) {
                printk (KERN_ALERT "SIS8300_THRES_REG: NO MEM\n");
                mutex_unlock(&dev->dev_mut);
                return EFAULT;
            }
            switch(tmp_cmd){
                case IOCTRL_R:
                    tmp_data_32       = ioread32(address + tmp_offset*4);
                    smp_rmb();
                    udelay(2);
                    tmp_data_32 &= 0xFFFFFFFF;
                    data.data   = tmp_data_32;
                    break;
                case IOCTRL_W:
                    tmp_data_32 = tmp_data & 0xFFFFFFFF;
                    iowrite32(tmp_data_32, ((void*)(address + tmp_offset*4)));
                    smp_wmb();
                    udelay(2);
                    break;
                default:
                    mutex_unlock(&dev->dev_mut);
                    return EINVAL;
            }
            if (copy_to_user((device_ioctrl_data*)arg, &data, (size_t)io_size)) {
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                return retval;
            }
            break;
        case SIS8300_SAMPLE_ADDR:
            retval = 0;
            if (copy_from_user(&data, (device_ioctrl_data*)arg, (size_t)io_size)) {
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                return retval;
            }
            tmp_offset   = data.offset;
            tmp_offset  += SIS8300_SAMPLE_START_ADDRESS_CH1_REG;
            tmp_data     = data.data;
            tmp_cmd      = data.cmd;
            tmp_reserved = data.reserved;
            if (!address) {
                printk (KERN_ALERT "SIS8300_SAMPLE_ADDR: NO MEM\n");
                mutex_unlock(&dev->dev_mut);
                return EFAULT;
            }
            switch(tmp_cmd){
                case IOCTRL_R:
                    tmp_data_32       = ioread32(address + tmp_offset*4);
                    smp_rmb();
                    udelay(2);
                    tmp_data_32 &= 0xFFFFFFFF;
                    data.data   = tmp_data_32;
                    break;
                case IOCTRL_W:
                    tmp_data_32 = tmp_data & 0xFFFFFFFF;
                    iowrite32(tmp_data_32, ((void*)(address + tmp_offset*4)));
                    smp_wmb();
                    udelay(2);
                    break;
	       default:
                    mutex_unlock(&dev->dev_mut);
                    return EINVAL;
            }
            if (copy_to_user((device_ioctrl_data*)arg, &data, (size_t)io_size)) {
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                return retval;
            }
            break;
        case SIS8300_SAMPLE_ADDR_ALL:
            retval = 0;
	   //printk (KERN_ALERT "SIS8300_SAMPLE_ADDR_ALL: CALLED\n");
            if (copy_from_user(&data_buf, (device_ioctrl_data_buf*)arg, (size_t)io_buf_size)) {
	       //printk (KERN_ALERT "SIS8300_SAMPLE_ADDR_ALL: COPY FROM USER\n");
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                return retval;
            }
            tmp_offset   = data_buf.offset;
            tmp_offset  += SIS8300_SAMPLE_START_ADDRESS_CH1_REG;
            tmp_num     = data_buf.num;
	   tmp_step     = data_buf.step;
            tmp_cmd      = data_buf.cmd;
            if (!address) {
                //printk (KERN_ALERT "SIS8300_SAMPLE_ADDR_ALL: NO MEM\n");
                mutex_unlock(&dev->dev_mut);
                return -EFAULT;
            }
            switch(tmp_cmd){
                case IOCTRL_R:
		  for(i = 0; i < tmp_num; i++){
			tmp_offset  = SIS8300_SAMPLE_START_ADDRESS_CH1_REG + i;
			tmp_data_32       = ioread32(address + tmp_offset*4);
			//smp_rmb();
			//udelay(2);
			tmp_data_32 &= 0xFFFFFFFF;
			data_buf.data[i]   = tmp_data_32;
		  }
		  smp_rmb();
                    break;
                case IOCTRL_W:
/*
                    tmp_data_32 = tmp_data & 0xFFFFFFFF;
                    iowrite32(tmp_data_32, ((void*)(address + tmp_offset*4)));
                    smp_wmb();
                    udelay(2);
                    break;
*/
	       default:
                    mutex_unlock(&dev->dev_mut);
                    return EINVAL;
            }
            if (copy_to_user((device_ioctrl_data_buf*)arg, &data_buf, (size_t)io_buf_size)) {
	       //printk (KERN_ALERT "SIS8300_SAMPLE_ADDR_ALL: COPY TO USER\n");
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                return retval;
            }
            break; 
        case SIS8300_SAMPLE_LENGTH:
            retval = 0;
            if (copy_from_user(&data, (device_ioctrl_data*)arg, (size_t)io_size)) {
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                return retval;
            }
            tmp_offset   = data.offset;
            tmp_data     = data.data;
            tmp_cmd      = data.cmd;
            tmp_reserved = data.reserved;
            if (!address) {
                printk (KERN_ALERT "SIS8300_SAMPLE_LENGTH: NO MEM\n");
                mutex_unlock(&dev->dev_mut);
                return EFAULT;
            }
            switch(tmp_cmd){
                case IOCTRL_R:
                    tmp_data_32       = ioread32(address + SIS8300_SAMPLE_LENGTH_REG*4);
                    smp_rmb();
                    udelay(2);
                    tmp_data_32 &= 0xFFFFFFFF;
                    data.data   = tmp_data_32;
                    break;
                case IOCTRL_W:
                    tmp_data_32 = tmp_data & 0xFFFFFFFF;
                    iowrite32(tmp_data_32, ((void*)(address + SIS8300_SAMPLE_LENGTH_REG*4)));
                    smp_wmb();
                    udelay(2);
                    break;
                default:
                    mutex_unlock(&dev->dev_mut);
                    return EINVAL;
            }
            if (copy_to_user((device_ioctrl_data*)arg, &data, (size_t)io_size)) {
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                return retval;
            }
            break;
        case SIS8300_RINGBUF_DELAY:
            retval = 0;
            if (copy_from_user(&data, (device_ioctrl_data*)arg, (size_t)io_size)) {
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                return retval;
            }
            tmp_offset   = data.offset;
            tmp_data     = data.data;
            tmp_cmd      = data.cmd;
            tmp_reserved = data.reserved;
            if (!address) {
                printk (KERN_ALERT "SIS8300_RINGBUF_DELAY: NO MEM\n");
                mutex_unlock(&dev->dev_mut);
                return EFAULT;
            }
            switch(tmp_cmd){
                case IOCTRL_R:
                    tmp_data_32       = ioread32(address + SIS8300_PRETRIGGER_DELAY_REG*4);
                    smp_rmb();
                    udelay(2);
                    tmp_data_32 &= 0x3FF;
                    data.data   = tmp_data_32;
                    break;
                case IOCTRL_W:
                    tmp_data_32 = tmp_data & 0x3FF;
                    iowrite32(tmp_data_32, ((void*)(address + SIS8300_PRETRIGGER_DELAY_REG*4)));
                    smp_wmb();
                    udelay(2);
                    break;
                default:
                    mutex_unlock(&dev->dev_mut);
                    return EINVAL;
            }
            if (copy_to_user((device_ioctrl_data*)arg, &data, (size_t)io_size)) {
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                return retval;
            }
            break;
        case SIS8300_CNTRL_STS:
            retval = 0;
            if (copy_from_user(&data, (device_ioctrl_data*)arg, (size_t)io_size)) {
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                return retval;
            }
            tmp_offset   = data.offset;
            tmp_data     = data.data;
            tmp_cmd      = data.cmd;
            tmp_reserved = data.reserved;
            if (!address) {
                printk (KERN_ALERT "SIS8300_CNTRL_STS: NO MEM\n");
                mutex_unlock(&dev->dev_mut);
                return EFAULT;
            }
            switch(tmp_cmd){
                case IOCTRL_R:
                    tmp_data_32       = ioread32(address + SIS8300_ACQUISITION_CONTROL_STATUS_REG*4);
                    smp_rmb();
                    udelay(2);
                    tmp_data_32 &= 0xFF;
                    data.data   = tmp_data_32;
                    break;
                case IOCTRL_W:
                    tmp_data_32 = tmp_data & 0x7;
                    switch (tmp_data_32){
                        case MODE_RESET:
                        case MODE_TRG:
                        case MODE_NOW:
                            iowrite32(tmp_data_32, ((void*)(address + SIS8300_ACQUISITION_CONTROL_STATUS_REG*4)));
                            smp_wmb();
                            udelay(2);
                            break;
                        default:
                            iowrite32(MODE_RESET, ((void*)(address + SIS8300_ACQUISITION_CONTROL_STATUS_REG*4)));
                            smp_wmb();
                            udelay(2);
                            break;
                    }
                    break;
                default:
                    mutex_unlock(&dev->dev_mut);
                    return EINVAL;
            }
            if (copy_to_user((device_ioctrl_data*)arg, &data, (size_t)io_size)) {
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                return retval;
            }
            break;
        case PCIEDEV_GET_DMA_TIME:
        case SIS8300_GET_DMA_TIME:
            retval = 0;
            if (copy_from_user(&time_data, (device_ioctrl_time*)arg, (size_t)time_size)) {
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                return retval;
            }
            time_data.start_time = sis8300dev->dma_start_time;
            time_data.stop_time  = sis8300dev->dma_stop_time;
            if (copy_to_user((device_ioctrl_time*)arg, &time_data, (size_t)time_size)) {
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                return retval;
            }
            break;
        case SIS8300_HLINK_TRG:
            retval = 0;
            if (copy_from_user(&data, (device_ioctrl_data*)arg, (size_t)io_size)) {
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                return retval;
            }
            tmp_offset   = data.offset;
            tmp_data     = data.data;
            tmp_cmd      = data.cmd;
            tmp_reserved = data.reserved;
            if (!address) {
                printk (KERN_ALERT "SIS8300_HLINK_TRG: NO MEM\n");
                mutex_unlock(&dev->dev_mut);
                return EFAULT;
            }
            if (tmp_offset >=4) {
                printk (KERN_ALERT "SIS8300_HLINK_OUT: WRONG CH_NUM\n");
                mutex_unlock(&dev->dev_mut);
                return EFAULT;
            }
            tmp_data_32       = ioread32(address + SIS8300_HARLINK_IN_OUT_CONTROL_REG*4);
            smp_rmb();
            udelay(2);
            switch(tmp_cmd){
                case IOCTRL_R:
                    data.data = (tmp_data_32 >> (8 + tmp_offset))&0x1;
                    break;
                case IOCTRL_W:
                    tmp_data1 = 0x1 << (8 + tmp_offset);
                    tmp_data1 = ~tmp_data1;
                    tmp_data_32 &= tmp_data1;
                    tmp_data1 = (tmp_data&0x1) << (8 + tmp_offset);
                    tmp_data_32 |= tmp_data1;
                    iowrite32(tmp_data_32, ((void*)(address + SIS8300_HARLINK_IN_OUT_CONTROL_REG*4)));
                    smp_wmb();
                    udelay(2);
                    break;
                default:
                    mutex_unlock(&dev->dev_mut);
                    return EINVAL;
            }
            if (copy_to_user((device_ioctrl_data*)arg, &data, (size_t)io_size)) {
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                return retval;
            }
            break;
        case SIS8300_HLINK_TRG_FEDGE:
            retval = 0;
            if (copy_from_user(&data, (device_ioctrl_data*)arg, (size_t)io_size)) {
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                return retval;
            }
            tmp_offset   = data.offset;
            tmp_data     = data.data;
            tmp_cmd      = data.cmd;
            tmp_reserved = data.reserved;
            if (!address) {
                printk (KERN_ALERT "SIS8300_HLINK_TRG_FEDGE: NO MEM\n");
                mutex_unlock(&dev->dev_mut);
                return EFAULT;
            }
            if (tmp_offset >=4) {
                printk (KERN_ALERT "SIS8300_HLINK_OUT: WRONG CH_NUM\n");
                mutex_unlock(&dev->dev_mut);
                return EFAULT;
            }
            tmp_data_32       = ioread32(address + SIS8300_HARLINK_IN_OUT_CONTROL_REG*4);
            smp_rmb();
            udelay(2);
            switch(tmp_cmd){
                case IOCTRL_R:
                    data.data = (tmp_data_32 >> (12 + tmp_offset))&0x1;
                    break;
                case IOCTRL_W:
                    tmp_data1 = 0x1 << (12 + tmp_offset);
                    tmp_data1 = ~tmp_data1;
                    tmp_data_32 &= tmp_data1;
                    tmp_data1 = (tmp_data&0x1) << (12 + tmp_offset);
                    tmp_data_32 |= tmp_data1;
                    iowrite32(tmp_data_32, ((void*)(address + SIS8300_HARLINK_IN_OUT_CONTROL_REG*4)));
                    smp_wmb();
                    udelay(2);
                    break;
                default:
                    mutex_unlock(&dev->dev_mut);
                    return EINVAL;
            }
            if (copy_to_user((device_ioctrl_data*)arg, &data, (size_t)io_size)) {
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                return retval;
            }
            break;
        case SIS8300_HLINK_OUT:
            retval = 0;
            if (copy_from_user(&data, (device_ioctrl_data*)arg, (size_t)io_size)) {
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                return retval;
            }
            tmp_offset   = data.offset;
            tmp_data     = data.data;
            tmp_cmd      = data.cmd;
            tmp_reserved = data.reserved;
            if (!address) {
                printk (KERN_ALERT "SIS8300_HLINK_OUT: NO MEM\n");
                mutex_unlock(&dev->dev_mut);
                return EFAULT;
            }
            if (tmp_offset >=4) {
                printk (KERN_ALERT "SIS8300_HLINK_OUT: WRONG CH_NUM\n");
                mutex_unlock(&dev->dev_mut);
                return EFAULT;
            }
            tmp_data_32       = ioread32(address + SIS8300_HARLINK_IN_OUT_CONTROL_REG*4);
            smp_rmb();
            udelay(2);
            switch(tmp_cmd){
                case IOCTRL_R:
                    data.data = (tmp_data_32 >> (17 + tmp_offset))&0x1;
                    break;
                case IOCTRL_W:
                    tmp_data1 = 0x1 << (17 + tmp_offset);
                    tmp_data1 = ~tmp_data1;
                    tmp_data_32 &= tmp_data1;
                    tmp_data1 = (tmp_data&0x1) << (17 + tmp_offset);
                    tmp_data_32 |= tmp_data1;
                    iowrite32(tmp_data_32, ((void*)(address + SIS8300_HARLINK_IN_OUT_CONTROL_REG*4)));
                    smp_wmb();
                    udelay(2);
                    break;
                default:
                    mutex_unlock(&dev->dev_mut);
                    return EINVAL;
            }
            if (copy_to_user((device_ioctrl_data*)arg, &data, (size_t)io_size)) {
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                return retval;
            }
            break;
        case SIS8300_HLINK_IN:
            retval = 0;
            if (copy_from_user(&data, (device_ioctrl_data*)arg, (size_t)io_size)) {
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                return retval;
            }
            tmp_offset   = data.offset;
            tmp_data     = data.data;
            tmp_cmd      = data.cmd;
            tmp_reserved = data.reserved;
            if (!address) {
                printk (KERN_ALERT "SIS8300_HLINK_IN: NO MEM\n");
                mutex_unlock(&dev->dev_mut);
                return EFAULT;
            }
            if (tmp_offset >=4) {
                printk (KERN_ALERT "SIS8300_HLINK_OUT: WRONG CH_NUM\n");
                mutex_unlock(&dev->dev_mut);
                return EFAULT;
            }
            tmp_data_32       = ioread32(address + SIS8300_HARLINK_IN_OUT_CONTROL_REG*4);
            smp_rmb();
            udelay(2);
            data.data = (tmp_data_32 >>  tmp_offset)&0x1;
            if (copy_to_user((device_ioctrl_data*)arg, &data, (size_t)io_size)) {
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                return retval;
            }
            break;
        case SIS8300_MLVDS_REG:
            retval = 0;
            if (copy_from_user(&data, (device_ioctrl_data*)arg, (size_t)io_size)) {
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                return retval;
            }
            tmp_offset   = data.offset;
            tmp_data     = data.data;
            tmp_cmd      = data.cmd;
            tmp_reserved = data.reserved;
            if (!address) {
                printk (KERN_ALERT "SIS8300_MLVDS_TRG: NO MEM\n");
                mutex_unlock(&dev->dev_mut);
                return EFAULT;
            }
            tmp_data_32       = ioread32(address + SIS8300_MLVDS_IO_CONTROL_REG*4);
            smp_rmb();
            udelay(2);
            switch(tmp_cmd){
                case IOCTRL_R:
                    data.data = tmp_data_32 & 0xFFFFFFFF;
                    break;
                case IOCTRL_W:
                    tmp_data_32 = tmp_data & 0xFFFFFFFF;
                    iowrite32(tmp_data_32, ((void*)(address + SIS8300_MLVDS_IO_CONTROL_REG*4)));
                    smp_wmb();
                    udelay(2);
                    break;
                default:
                    mutex_unlock(&dev->dev_mut);
                    return EINVAL;
            }
            if (copy_to_user((device_ioctrl_data*)arg, &data, (size_t)io_size)) {
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                return retval;
            }
            break;
        case SIS8300_MLVDS_TRG:
            retval = 0;
            if (copy_from_user(&data, (device_ioctrl_data*)arg, (size_t)io_size)) {
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                return retval;
            }
            tmp_offset   = data.offset;
            tmp_data     = data.data;
            tmp_cmd      = data.cmd;
            tmp_reserved = data.reserved;
            if (!address) {
                printk (KERN_ALERT "SIS8300_MLVDS_TRG: NO MEM\n");
                mutex_unlock(&dev->dev_mut);
                return EFAULT;
            }
            tmp_data_32       = ioread32(address + SIS8300_MLVDS_IO_CONTROL_REG*4);
            smp_rmb();
            udelay(2);
            switch(tmp_cmd){
                case IOCTRL_R:
                    data.data = (tmp_data_32 >> (8 + tmp_offset))&0x1;
                    break;
                case IOCTRL_W:
                    tmp_data1 = 0x1 << (8 + tmp_offset);
                    tmp_data1 = ~tmp_data1;
                    tmp_data_32 &= tmp_data1;
                    tmp_data1 = (tmp_data&0x1) << (8 + tmp_offset);
                    tmp_data_32 |= tmp_data1;
                    iowrite32(tmp_data_32, ((void*)(address + SIS8300_MLVDS_IO_CONTROL_REG*4)));
                    smp_wmb();
                    udelay(2);
                    break;
                default:
                    mutex_unlock(&dev->dev_mut);
                    return EINVAL;
            }
            if (copy_to_user((device_ioctrl_data*)arg, &data, (size_t)io_size)) {
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                return retval;
            }
            break;
        case SIS8300_MLVDS_TRG_EDGE:
            retval = 0;
            if (copy_from_user(&data, (device_ioctrl_data*)arg, (size_t)io_size)) {
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                return retval;
            }
            tmp_offset   = data.offset;
            tmp_data     = data.data;
            tmp_cmd      = data.cmd;
            tmp_reserved = data.reserved;
            if (!address) {
                printk (KERN_ALERT "SIS8300_MLVDS_TRG_EDGE: NO MEM\n");
                mutex_unlock(&dev->dev_mut);
                return EFAULT;
            }
            tmp_data_32       = ioread32(address + SIS8300_MLVDS_IO_CONTROL_REG*4);
            smp_rmb();
            udelay(2);
            switch(tmp_cmd){
                case IOCTRL_R:
                    data.data = (tmp_data_32 >> tmp_offset)&0x1;
                    break;
                case IOCTRL_W:
                    tmp_data1 = 0x1 << tmp_offset;
                    tmp_data1 = ~tmp_data1;
                    tmp_data_32 &= tmp_data1;
                    tmp_data1 = (tmp_data&0x1) << tmp_offset;
                    tmp_data_32 |= tmp_data1;
                    iowrite32(tmp_data_32, ((void*)(address + SIS8300_MLVDS_IO_CONTROL_REG*4)));
                    smp_wmb();
                    udelay(2);
                    break;
                default:
                    mutex_unlock(&dev->dev_mut);
                    return EINVAL;
            }
            if (copy_to_user((device_ioctrl_data*)arg, &data, (size_t)io_size)) {
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                return retval;
            }
            break;
        case SIS8300_MLVDS_OUT_ENBL:
            retval = 0;
            if (copy_from_user(&data, (device_ioctrl_data*)arg, (size_t)io_size)) {
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                return retval;
            }
            tmp_offset   = data.offset;
            tmp_data     = data.data;
            tmp_cmd      = data.cmd;
            tmp_reserved = data.reserved;
            if (!address) {
                printk (KERN_ALERT "SIS8300_MLVDS_OUT_ENBL: NO MEM\n");
                mutex_unlock(&dev->dev_mut);
                return EFAULT;
            }
            tmp_data_32       = ioread32(address + SIS8300_MLVDS_IO_CONTROL_REG*4);
            smp_rmb();
            udelay(2);
            switch(tmp_cmd){
                case IOCTRL_R:
                    data.data = (tmp_data_32 >> (24 + tmp_offset))&0x1;
                    break;
                case IOCTRL_W:
                    tmp_data1 = 0x1 << (24 + tmp_offset);
                    tmp_data1 = ~tmp_data1;
                    tmp_data_32 &= tmp_data1;
                    tmp_data1 = (tmp_data&0x1) << (24 + tmp_offset);
                    tmp_data_32 |= tmp_data1;
                    iowrite32(tmp_data_32, ((void*)(address + SIS8300_MLVDS_IO_CONTROL_REG*4)));
                    smp_wmb();
                    udelay(2);
                    break;
                default:
                    mutex_unlock(&dev->dev_mut);
                    return EINVAL;
            }
            if (copy_to_user((device_ioctrl_data*)arg, &data, (size_t)io_size)) {
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                return retval;
            }
            break;
        case SIS8300_MLVDS_OUT:
            retval = 0;
            if (copy_from_user(&data, (device_ioctrl_data*)arg, (size_t)io_size)) {
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                return retval;
            }
            tmp_offset   = data.offset;
            tmp_data     = data.data;
            tmp_cmd      = data.cmd;
            tmp_reserved = data.reserved;
            if (!address) {
                printk (KERN_ALERT "SIS8300_MLVDS_OUT: NO MEM\n");
                mutex_unlock(&dev->dev_mut);
                return EFAULT;
            }
            tmp_data_32       = ioread32(address + SIS8300_MLVDS_IO_CONTROL_REG*4);
            smp_rmb();
            udelay(2);
            switch(tmp_cmd){
                case IOCTRL_R:
                    data.data = (tmp_data_32 >> (16 + tmp_offset))&0x1;
                    break;
                case IOCTRL_W:
                    tmp_data1 = 0x1 << (16 + tmp_offset);
                    tmp_data1 = ~tmp_data1;
                    tmp_data_32 &= tmp_data1;
                    tmp_data1 = (tmp_data&0x1) << (16 + tmp_offset);
                    tmp_data_32 |= tmp_data1;
                    iowrite32(tmp_data_32, ((void*)(address + SIS8300_MLVDS_IO_CONTROL_REG*4)));
                    smp_wmb();
                    udelay(2);
                    break;
                default:
                    mutex_unlock(&dev->dev_mut);
                    return EINVAL;
            }
            if (copy_to_user((device_ioctrl_data*)arg, &data, (size_t)io_size)) {
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                return retval;
            }
            break;
        case SIS8300_MLVDS_IN:
            retval = 0;
            if (copy_from_user(&data, (device_ioctrl_data*)arg, (size_t)io_size)) {
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                return retval;
            }
            tmp_offset   = data.offset;
            tmp_data     = data.data;
            tmp_cmd      = data.cmd;
            tmp_reserved = data.reserved;
            if (!address) {
                printk (KERN_ALERT "SIS8300_MLVDS_IN: NO MEM\n");
                mutex_unlock(&dev->dev_mut);
                return EFAULT;
            }
            tmp_data_32       = ioread32(address + SIS8300_MLVDS_IO_CONTROL_REG*4);
            smp_rmb();
            udelay(2);
            data.data = (tmp_data_32 >>  tmp_offset)&0x1;
            if (copy_to_user((device_ioctrl_data*)arg, &data, (size_t)io_size)) {
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                return retval;
            }
            break;
         case PCIEDEV_READ_DMA:
         case SIS8300_READ_DMA:
			 //printk (KERN_ALERT "##############SIS8300_READ_DMA: START DMA READ\n");
            retval = 0;
            if (copy_from_user(&dma_data, (device_ioctrl_dma*)arg, (size_t)io_dma_size)) {
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                printk (KERN_ALERT "SIS8300_READ_DMA: COULD NOT COPY FROM USER\n");
                return retval;
            }
            tmp_dma_size          = dma_data.dma_size;
            tmp_dma_offset        = dma_data.dma_offset;
            if(tmp_dma_offset < 0xB0000000){
                if((tmp_dma_offset + tmp_dma_size) > sis8300dev->sis8300_mem_max_size){
                    printk (KERN_ALERT "SIS8300_READ_DMA: OUT OFF MEMMORY %X\n", (tmp_dma_offset + tmp_dma_size));
                    tmp_dma_size = (sis8300dev->sis8300_mem_max_size - tmp_dma_offset);
                    printk (KERN_ALERT "SIS8300_READ_DMA: OUT OFF MEMMORY NEW SIZE %X\n", (tmp_dma_offset + tmp_dma_size));
                }
                if(tmp_dma_offset >= sis8300dev->sis8300_mem_max_size){
                 printk (KERN_ALERT "SIS8300_READ_DMA: BIG OFFSET %d\n", tmp_dma_offset);
                 mutex_unlock(&dev->dev_mut);
                 return EFAULT;
                }
            }
            sis8300dev->dev_dma_size     = tmp_dma_size;
             if(tmp_dma_size <= 0){
                 printk (KERN_ALERT "SIS8300_READ_DMA: SIZE 0 tmp_dma_size %d\n", tmp_dma_size);
                 mutex_unlock(&dev->dev_mut);
                 return EFAULT;
            }
            
            tmp_data_32       = ioread32(address + 0); // be safe all writes are done
            tmp_dma_trns_size    = tmp_dma_size;
            if((tmp_dma_size%SIS8300_DMA_SYZE)){
                tmp_dma_trns_size    = tmp_dma_size + (tmp_dma_size%SIS8300_DMA_SYZE);
            }
            value  = 10000*HZ/200000; /* value is given in jiffies*/
            //value    = HZ/1;                /* value is given in jiffies*/
            length   = tmp_dma_size;
            tmp_order = get_order(tmp_dma_trns_size);
            sis8300dev->dma_page_order = tmp_order;
            pWriteBuf = (void *)__get_free_pages(GFP_KERNEL | __GFP_DMA, tmp_order);
            pTmpDmaHandle      = pci_map_single(pdev, pWriteBuf, tmp_dma_trns_size, PCI_DMA_FROMDEVICE);

            tmp_source_address = tmp_dma_offset;
            dma_sys_addr       = (u32)(pTmpDmaHandle & 0xFFFFFFFF);
            iowrite32(tmp_source_address, ((void*)(address + DMA_READ_SRC_ADR_LO32*4)));
			
            tmp_data_32         = dma_sys_addr;
            iowrite32(tmp_data_32, ((void*)(address + DMA_READ_DST_ADR_LO32*4)));
            smp_wmb();
            dma_sys_addr       = (u32)((pTmpDmaHandle >> 32) & 0xFFFFFFFF);
            tmp_data_32         = dma_sys_addr;
            iowrite32(tmp_data_32, ((void*)(address + DMA_READ_DST_ADR_HI32*4)));
            smp_wmb();
            iowrite32(tmp_dma_trns_size, ((void*)(address + DMA_READ_LEN*4)));
            smp_wmb();
            iowrite32(0xFFFF0000, ((void*)(address + IRQ_ENABLE*4)));
            smp_wmb();
            iowrite32((1<<DMA_READ_DONE), ((void*)(address + IRQ_ENABLE*4)));
            smp_wmb();
            udelay(5);
            tmp_data_32       = ioread32(address + 0); // be safe all writes are done
            smp_rmb();
            do_gettimeofday(&(sis8300dev->dma_start_time));
            sis8300dev->waitFlag = 0;
            iowrite32((1<<DMA_READ_START), ((void*)(address + DMA_READ_CTRL*4)));
            timeDMAwait = wait_event_interruptible_timeout( sis8300dev->waitDMA, sis8300dev->waitFlag != 0, value );
            do_gettimeofday(&(sis8300dev->dma_stop_time));
            if(!sis8300dev->waitFlag){
				printk (KERN_ALERT "SIS8300_READ_DMA:SLOT NUM %i NO INTERRUPT 0 \n", dev->slot_num);
                tmp_data_32       = ioread32((void*)(address + DMA_READ_CTRL*4)); 
                smp_rmb();
                 for(dma_done_count = 0; dma_done_count < 2000; ++dma_done_count){
					 printk (KERN_ALERT "SIS8300_READ_DMA:SLOT NUM %i NO INTERRUPT COUNT %i \n", dev->slot_num, dma_done_count);
			 udelay(20);
			tmp_data_32 = 0;
			tmp_data_32       = ioread32((void*)(address + DMA_READ_CTRL*4)); 
			smp_rmb();
			if(!(tmp_data_32 & 0x1)) break;
		}
                if(tmp_data_32 & 0x1){
                    printk (KERN_ALERT "SIS8300_READ_DMA:SLOT NUM %i NO INTERRUPT \n", dev->slot_num);
                    sis8300dev->waitFlag = 1;
                    iowrite32(0xFFFFFFFF, ((void*)(address + IRQ_CLEAR*4)));
                    smp_wmb();
                    udelay(5);
                    pci_unmap_single(pdev, pTmpDmaHandle, tmp_dma_trns_size, PCI_DMA_FROMDEVICE);
                    free_pages((ulong)pWriteBuf, (ulong)sis8300dev->dma_page_order);
                    mutex_unlock(&dev->dev_mut);
                    return EFAULT;
                }    
            }
		//printk (KERN_ALERT "SIS8300_READ_DMA:SLOT NUM %i START UNMAP \n", dev->slot_num);	
            pci_unmap_single(pdev, pTmpDmaHandle, tmp_dma_trns_size, PCI_DMA_FROMDEVICE);
            if (copy_to_user ((void *)arg, pWriteBuf, tmp_dma_size)) {
                retval = -EFAULT;
                printk (KERN_ALERT "SIS8300_READ_DMA: SLOT NUM %i COULD NOT COPY TO USER\n", dev->slot_num);
            }
            free_pages((ulong)pWriteBuf, (ulong)sis8300dev->dma_page_order);
            iowrite32(0xFFFFFFFF, ((void*)(address + IRQ_CLEAR*4)));
            smp_wmb();
            udelay(5);
            break;
        case PCIEDEV_WRITE_DMA:
        case SIS8300_WRITE_DMA:
			//printk (KERN_ALERT "SIS8300_READ_DMA: START DMA write\n");
            retval = 0;
            if (copy_from_user(&dma_data, (device_ioctrl_dma*)arg, (size_t)io_dma_size)) {
                retval = -EFAULT;
                mutex_unlock(&dev->dev_mut);
                printk (KERN_ALERT "SIS8300_WRITE_DMA: COULD NOT COPY FROM USER\n");
                return retval;
            }
	 // value  = 10000*HZ/20000; /* value is given in jiffies*/
           value    = HZ/1; /* value is given in jiffies*/
            tmp_dma_size           = dma_data.dma_size;
            tmp_dma_offset        = dma_data.dma_offset;
            if(tmp_dma_size <= 0){
                 printk (KERN_ALERT "SIS8300_WRITE_DMA: tmp_dma_size %d\n", tmp_dma_size);
                 mutex_unlock(&dev->dev_mut);
                 return EFAULT;
            }
            if(tmp_dma_offset < 0xB0000000){
                if((tmp_dma_offset + tmp_dma_size) > sis8300dev->sis8300_mem_max_size){
                    tmp_dma_size = sis8300dev->sis8300_mem_max_size - tmp_dma_offset;
                }
            }
            tmp_offset                    = 0;
            tmp_dma_trns_size       = tmp_dma_size;
            if((tmp_dma_size%SIS8300_DMA_SYZE)){
                tmp_dma_trns_size    = tmp_dma_size + (tmp_dma_size%SIS8300_DMA_SYZE);
            }
            tmp_data_32       = ioread32(address + 0); // be safe all writes are done
            length   = tmp_dma_size;
            tmp_order = get_order(tmp_dma_trns_size);
            sis8300dev->dma_page_order = tmp_order;
            pWriteBuf = (void *)__get_free_pages(GFP_KERNEL | __GFP_DMA, tmp_order);
            tmp_source_address = tmp_dma_offset;
            if (copy_from_user(pWriteBuf, ((u_int*)arg + DMA_DATA_OFFSET), (size_t)length)) {
                retval = -EFAULT;
                //pci_unmap_single(pdev, pTmpDmaHandle, tmp_dma_trns_size, PCI_DMA_TODEVICE);
                free_pages((ulong)pWriteBuf, (ulong)sis8300dev->dma_page_order);
                mutex_unlock(&dev->dev_mut);
                return retval;
            }
            pTmpDmaHandle       = pci_map_single(pdev, pWriteBuf, tmp_dma_trns_size, PCI_DMA_TODEVICE);
            tmp_source_address = tmp_dma_offset;
            iowrite32(tmp_source_address, ((void*)(address + DMA_WRITE_DST_ADR_LO32*4)));
            dma_sys_addr       = (u32)(pTmpDmaHandle & 0xFFFFFFFF);
            iowrite32(dma_sys_addr, ((void*)(address + DMA_WRITE_SRC_ADR_LO32*4)));
            dma_sys_addr       = (u32)((pTmpDmaHandle >> 32) & 0xFFFFFFFF);
            iowrite32(dma_sys_addr, ((void*)(address + DMA_WRITE_SRC_ADR_HI32*4)));
            iowrite32(tmp_dma_trns_size, ((void*)(address + DMA_WRITE_LEN*4)));
            iowrite32(0xFFFF0000, ((void*)(address + IRQ_ENABLE*4)));
            iowrite32((1<<DMA_WRITE_DONE), ((void*)(address + IRQ_ENABLE*4)));
            smp_wmb();
            udelay(2);
            tmp_data_32       = ioread32(address + 0); // be safe all writes are done
            smp_rmb();
            do_gettimeofday(&(sis8300dev->dma_start_time));
            sis8300dev->waitFlag = 0;
            iowrite32((1<<DMA_WRITE_START), ((void*)(address + DMA_WRITE_CTRL*4)));
            timeDMAwait = wait_event_interruptible_timeout( sis8300dev->waitDMA, sis8300dev->waitFlag != 0, value );
            do_gettimeofday(&(sis8300dev->dma_stop_time));
            if(!sis8300dev->waitFlag){
                printk (KERN_ALERT "SIS8300_WRITE_DMA: NO INTERRUPT\n");
	      tmp_data_32       = ioread32((void*)(address + DMA_WRITE_CTRL*4)); 
                smp_rmb();
	      for(dma_done_count = 0; dma_done_count < 2000; ++dma_done_count){
			 printk (KERN_ALERT "SIS8300_WRITE_DMA:SLOT NUM %i NO INTERRUPT COUNT %i \n", dev->slot_num, dma_done_count);
			 udelay(20);
			tmp_data_32 = 0;
			tmp_data_32       = ioread32((void*)(address + DMA_WRITE_CTRL*4)); 
			smp_rmb();
			if(!(tmp_data_32 & 0x1)) break;
		}
	       if(tmp_data_32 & 0x1){
			printk (KERN_ALERT "SIS8300_WRITE_DMA:SLOT NUM %i NO INTERRUPT \n", dev->slot_num);
			sis8300dev->waitFlag = 1;
			iowrite32(0xFFFFFFFF, ((void*)(address + IRQ_CLEAR*4)));
			smp_wmb();
			udelay(5);
			pci_unmap_single(pdev, pTmpDmaHandle, tmp_dma_trns_size, PCI_DMA_TODEVICE);
			free_pages((ulong)pWriteBuf, (ulong)sis8300dev->dma_page_order);
			mutex_unlock(&dev->dev_mut);
			return EFAULT;
                }  
				
/*
                sis8300dev->waitFlag = 1;
                iowrite32(0xFFFFFFFF, ((void*)(address + IRQ_CLEAR*4)));
                smp_wmb();
                udelay(2);
                pci_unmap_single(pdev, pTmpDmaHandle, tmp_dma_trns_size, PCI_DMA_TODEVICE);
                free_pages((ulong)pWriteBuf, (ulong)sis8300dev->dma_page_order);
                mutex_unlock(&dev->dev_mut);
                return EFAULT;
*/
            }
            pci_unmap_single(pdev, pTmpDmaHandle, tmp_dma_trns_size, PCI_DMA_TODEVICE);
            free_pages((ulong)pWriteBuf, (ulong)sis8300dev->dma_page_order);
            iowrite32(0xFFFFFFFF, ((void*)(address + IRQ_CLEAR*4)));
            smp_wmb();
            udelay(2);
            break;
			
		case PCIEDEV_WRITE_DMA_P2P:
		case SIS8300_WRITE_DMA_2PEER:
			 //printk (KERN_ALERT "SIS8300_WRITE_DMA_2PEER: START PEER WRITE\n");
			 
		/*
			u_int    p2p_slot;
			u_int    p2p_bar;
			u_int    p2p_offset;
			u_int    p2p_phs_address;
			u_int    p2p_phs_end;
			u_int    p2p_phs_flag;
			device_phys_address  p2p_phaddress;
		
			struct device_phys_address  {
				u_int    slot;
				u_int    bar;
				u_int    offset;
				u_int    phs_address;
				u_int    phs_end;
				u_int    phs_flag;
				u_int    reserved;
			};
		*/
			retval = 0;
			if (copy_from_user(&dma_data, (device_ioctrl_dma*)arg, (size_t)io_dma_size)) {
				retval = -EFAULT;
				mutex_unlock(&dev->dev_mut);
				printk (KERN_ALERT "SIS8300_WRITE_DMA_2PEER: COULD NOT COPY FROM USER\n");
				return retval;
			}
			tmp_dma_size           = dma_data.dma_size;
			tmp_dma_offset        = dma_data.dma_offset;
			tmp_cmd                    = dma_data.dma_cmd;
			p2p_slot                      = (dma_data.dma_reserved1 >> 16) & 0xFFFF; 
			p2p_bar                       = dma_data.dma_reserved1 & 0xFFFF;
			p2p_offset                   = dma_data.dma_reserved2;
           
			sis8300dev->dev_dma_size     = tmp_dma_size;
			 if(tmp_dma_size <= 0){
				 printk (KERN_ALERT "SIS8300_WRITE_DMA_2PEER: SIZE 0 tmp_dma_size %d\n", tmp_dma_size);
				 mutex_unlock(&dev->dev_mut);
				 return EFAULT;
			}
            
			tmp_data_32       = ioread32(address + 0); // be safe all writes are done
			tmp_dma_trns_size    = tmp_dma_size;
			if((tmp_dma_size%SIS8300_DMA_SYZE)){
				tmp_dma_trns_size    = tmp_dma_size + (tmp_dma_size%SIS8300_DMA_SYZE);
			}
			value  = 10000*HZ/200000; /* value is given in jiffies*/
			length   = tmp_dma_size;
            
			tmp_source_address = tmp_dma_offset;
			iowrite32(tmp_source_address, ((void*)(address + DMA_READ_SRC_ADR_LO32*4)));
			
			if(tmp_cmd == 3){
				dma_sys_addr = p2p_offset ; 
				p2p_phs_address = p2p_offset;
			}else{
				p2p_phaddress.slot = p2p_slot;
				p2p_phaddress.bar  = p2p_bar;
				p2p_phs_address = pciedev_get_physical_address(dev, &p2p_phaddress); 
				dma_sys_addr = p2p_phs_address + p2p_offset ; 
			}
			
			
			
			printk (KERN_ALERT "SIS8300_WRITE_DMA_2PEER: SLOT%i BAR%i\n", p2p_slot, p2p_bar);
			printk (KERN_ALERT "SIS8300_WRITE_DMA_2PEER: PHADDRESS %X OFFSET %X\n", p2p_phs_address, p2p_offset);
			printk (KERN_ALERT "SIS8300_WRITE_DMA_2PEER: SIZE %X DMA_SYS_ADDRESS %X\n", tmp_dma_size, dma_sys_addr);
			
			
			
			tmp_data_32         = dma_sys_addr;
			iowrite32(tmp_data_32, ((void*)(address + DMA_READ_DST_ADR_LO32*4)));
			smp_wmb();
			tmp_data_32         = dma_sys_addr;
			tmp_data_32       = (u32)((dma_sys_addr >> 32) & 0xFFFFFFFF);
			iowrite32(tmp_data_32, ((void*)(address + DMA_READ_DST_ADR_HI32*4)));
			smp_wmb();
			iowrite32(tmp_dma_trns_size, ((void*)(address + DMA_READ_LEN*4)));
			smp_wmb();
			iowrite32(0xFFFF0000, ((void*)(address + IRQ_ENABLE*4)));
			smp_wmb();
			iowrite32((1<<DMA_READ_DONE), ((void*)(address + IRQ_ENABLE*4)));
			smp_wmb();
			udelay(5);
			tmp_data_32       = ioread32(address + 0); // be safe all writes are done
			smp_rmb();
			do_gettimeofday(&(sis8300dev->dma_start_time));
			sis8300dev->waitFlag = 0;
			iowrite32((1<<DMA_READ_START), ((void*)(address + DMA_READ_CTRL*4)));
			timeDMAwait = wait_event_interruptible_timeout( sis8300dev->waitDMA, sis8300dev->waitFlag != 0, value );
			do_gettimeofday(&(sis8300dev->dma_stop_time));
			if(!sis8300dev->waitFlag){
				tmp_data_32       = ioread32((void*)(address + DMA_READ_CTRL*4)); 
				smp_rmb();
				 for(dma_done_count = 0; dma_done_count < 2000; ++dma_done_count){
							 udelay(20);
							tmp_data_32 = 0;
							tmp_data_32       = ioread32((void*)(address + DMA_READ_CTRL*4)); 
							smp_rmb();
							if(!(tmp_data_32 & 0x1)) break;
				}
				if(tmp_data_32 & 0x1){
					printk (KERN_ALERT "SIS8300_READ_DMA:SLOT NUM %i NO INTERRUPT \n", dev->slot_num);
					sis8300dev->waitFlag = 1;
					iowrite32(0xFFFFFFFF, ((void*)(address + IRQ_CLEAR*4)));
					smp_wmb();
					udelay(5);
					mutex_unlock(&dev->dev_mut);
					return EFAULT;
				}    
			}
			
			if (copy_to_user ((device_ioctrl_dma *)arg, &dma_data, (size_t)io_dma_size)) {
				retval = -EFAULT;
				printk (KERN_ALERT "SIS8300_WRITE_DMA_2PEER: SLOT NUM %i COULD NOT COPY TO USER\n", dev->slot_num);
			}
			iowrite32(0xFFFFFFFF, ((void*)(address + IRQ_CLEAR*4)));
			smp_wmb();
			udelay(5);
			break;
			
        default:
		   mutex_unlock(&dev->dev_mut);
            return -ENOTTY;
            break;
    }
    mutex_unlock(&dev->dev_mut);
    
    return retval;
}

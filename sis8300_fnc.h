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


#ifndef _SIS8300_FNC_H_
#define _SIS8300_FNC_H_
#include <linux/version.h>
#include "pciedev_io.h"
#include "pciedev_ufn.h"

#ifndef SIS8300_NR_DEVS
#define SIS8300_NR_DEVS 15  /* sis83000 through sis830011 */
#endif

#define SIS8300DEVNAME "sis8300"	                    /* name of device */
#define SIS8300_VENDOR_ID 0x1796	/* FZJZEL vendor ID */
#define SIS8300_DEVICE_ID 0x0018	/* SIS8300 dev board device ID */
#define SIS8300L_DEVICE_ID 0x0019	/* SIS8300L dev board device ID */

#define SIS8300_KERNEL_DMA_BLOCK_SIZE 131072    // 128kByte
//#define SIS8300_KERNEL_DMA_BLOCK_SIZE 262144 // 256kByte
//#define SIS8300_MEM_MAX_SIZE          536870912 // 512MByte
//#define SIS8300_DMA_MAX_SYZE         32768
//#define SIS8300_DMA_MIN_SYZE          128
#define SIS8300_DMA_SYZE                  4096

struct sis8300_dev {
    int                              brd_num;
    struct timeval          dma_start_time;
    struct timeval          dma_stop_time;
    int                              waitFlag;
    u32                            dev_dma_size;
    u32                            dma_page_num;
    int                              dma_offset;
    int                              dma_page_order;
    wait_queue_head_t  waitDMA;
    struct pciedev_dev  *parent_dev;
    int                          sis8300_mem_max_size;
    int                          fpga_1_gb;
    int                          dual_optical_interface;
    int                          dual_port_14_15_interface;
    int                          dual_port_12_13_interface;
    int                          dual_channel_sampling;
    int                          ringbuffer_delay;
    int                          trigger_block_enable;
};
typedef struct sis8300_dev sis8300_dev;

long sis8300_ioctl_dma(struct file *, unsigned int* , unsigned long* );

#endif /* _SIS8300_FNC_H_ */

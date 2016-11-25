#ifndef _SIS8300_FNC_H_
#define _SIS8300_FNC_H_

#include "pciedev_io.h"
#include "pciedev_ufn.h"
#include <PerfLog.h>

// uncomment to enable profiling and profile dump at driver unload
// #define __SIS300_PROFILE

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


#ifdef __SIS300_PROFILE
    #define _Profile(__a, __b, __c) PerfLog_put( (__a)->perflog, (__b), (__c) )
#else
    #define _Profile(__a, __b, __c)
#endif

struct sis8300_dev {
    struct timeval          dma_start_time;
    struct timeval          dma_stop_time;
    int                              waitFlag;
    u32                            dev_dma_size;
    u32                            dma_page_num;
    int                              dma_offset;
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
    #ifdef __SIS300_PROFILE
        PerfLog                 *perflog;
    #endif
};
typedef struct sis8300_dev sis8300_dev;

long sis8300_ioctl_dma(struct file *, unsigned int* , unsigned long* );

#endif /* _SIS8300_FNC_H_ */

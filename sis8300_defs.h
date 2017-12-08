/* 
 * File:   sis8300_defs.h
 * Author: petros
 *
 * Created on January 12, 2011, 5:20 PM
 */

#ifndef SIS8300_DEFS_H
#define	SIS8300_DEFS_H

#include <linux/ioctl.h> /* needed for the _IOW etc stuff used later */
#include "pciedev_io.h"

//#define IOCTRL_R      0x00
//#define IOCTRL_W     0x01
//#define IOCTRL_ALL  0x02
#define MUXA              0x00
#define MUXB              0x01
#define MUXC              0x02
#define MUXD               0x04
#define MUXE               0x05
#define SPI_AD9510     0x41
#define SPI_SI5326       0x42
#define SPI_DAC            0x44
#define SPI_ADC            0x48
#define MODE_RESET  0x4
#define MODE_TRG      0x2
#define MODE_NOW    0x1

struct device_ioctrl_data_buf  {
        u_int    offset;
        u_int    cmd;
        u_int    num;
        u_int    step;
        u_int    data[64];
};
typedef struct device_ioctrl_data_buf device_ioctrl_data_buf;



typedef struct t_sis8300_reg{
	u_int offset; /* offset from bar0 */
	u_int data;   /* data which will be read / written */
}sis8300_reg;

/* Use 'o' as magic number */

#define SIS8300_IOC           			'0'
#define SIS8300_PHYSICAL_SLOT       _IOWR(SIS8300_IOC, 20, int)
#define SIS8300_REG_READ            _IOWR(SIS8300_IOC, 21, int)
#define SIS8300_REG_WRITE           _IOWR(SIS8300_IOC, 22, int)
#define SIS8300_GET_DMA_TIME 	    _IOWR(SIS8300_IOC, 23, int)
#define SIS8300_DRIVER_VERSION      _IOWR(SIS8300_IOC, 24, int)
#define SIS8300_FIRMWARE_VERSION    _IOWR(SIS8300_IOC, 25, int)
#define SIS8300_EXT_TRIGGER         _IOWR(SIS8300_IOC, 26, int)
#define SIS8300_DISABLE_CHANNEL     _IOWR(SIS8300_IOC, 27, int)
#define SIS8300_MUX_SELECT          _IOWR(SIS8300_IOC, 28, int)
#define SIS8300_SPI_INTRFC          _IOWR(SIS8300_IOC, 29, int)
#define SIS8300_DAC_CONTROL         _IOWR(SIS8300_IOC, 30, int)
#define SIS8300_DAC_DATA            _IOWR(SIS8300_IOC, 31, int)
#define SIS8300_TAP_DELAY           _IOWR(SIS8300_IOC, 32, int)
#define SIS8300_TRG_SETUP           _IOWR(SIS8300_IOC, 33, int)
#define SIS8300_THRES_REG           _IOWR(SIS8300_IOC, 34, int)
#define SIS8300_SAMPLE_ADDR         _IOWR(SIS8300_IOC, 35, int)
#define SIS8300_SAMPLE_LENGTH       _IOWR(SIS8300_IOC, 36, int)
#define SIS8300_RINGBUF_DELAY       _IOWR(SIS8300_IOC, 37, int)
#define SIS8300_CNTRL_STS           _IOWR(SIS8300_IOC, 38, int)
#define SIS8300_DMA_OFFSET          _IOWR(SIS8300_IOC, 39, int)
#define SIS8300_READ_DMA            _IOWR(SIS8300_IOC, 40, int)
#define SIS8300_WRITE_DMA           _IOWR(SIS8300_IOC, 41, int)
#define SIS8300_READ_DMA_4k         _IOWR(SIS8300_IOC, 42, int)
#define SIS8300_DDR2_TEST_ENBL      _IOWR(SIS8300_IOC, 43, int)
#define SIS8300_HLINK_IN            _IOWR(SIS8300_IOC, 44, int)
#define SIS8300_HLINK_OUT           _IOWR(SIS8300_IOC, 45, int)
#define SIS8300_HLINK_TRG           _IOWR(SIS8300_IOC, 46, int)
#define SIS8300_HLINK_TRG_FEDGE     _IOWR(SIS8300_IOC, 47, int)
#define SIS8300_MLVDS_OUT           _IOWR(SIS8300_IOC, 48, int)
#define SIS8300_MLVDS_OUT_ENBL      _IOWR(SIS8300_IOC, 49, int)
#define SIS8300_MLVDS_TRG           _IOWR(SIS8300_IOC, 50, int)
#define SIS8300_MLVDS_IN            _IOWR(SIS8300_IOC, 51, int)
#define SIS8300_MLVDS_TRG_EDGE      _IOWR(SIS8300_IOC, 52, int)
#define SIS8300_MLVDS_REG           _IOWR(SIS8300_IOC, 53, int)
#define SIS8300_BLINK_LED           _IOWR(SIS8300_IOC, 54, int)
#define SIS8300_SAMPLE_ADDR_ALL         _IOWR(SIS8300_IOC, 55, int)
#define SIS8300_WRITE_DMA_2PEER         _IOWR(SIS8300_IOC, 56, int)

#define SIS8300_IOC_MAXNR           57
#define SIS8300_IOC_MINNR           20

#endif	/* SIS8300_DEFS_H */


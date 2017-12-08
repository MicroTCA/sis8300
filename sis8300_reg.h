/**
*Copyright 2016-  DESY (Deutsches Elektronen-Synchrotron, www.desy.de)
*
*This file is part of UPCIEDEV driver.
*
*Foobar is free software: you can redistribute it and/or modify
*it under the terms of the GNU General Public License as published by
*the Free Software Foundation, either version 3 of the License, or
*(at your option) any later version.
*
*Foobar is distributed in the hope that it will be useful,
*but WITHOUT ANY WARRANTY; without even the implied warranty of
*MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*GNU General Public License for more details.
*
*You should have received a copy of the GNU General Public License
*along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
**/

/*
*	Author: Ludwig Petrosyan (Email: ludwig.petrosyan@desy.de)
*/


#ifndef SIS8300_REG_H_
#define SIS8300_REG_H_

// all bit definitions are bit numbers

// module firmware/version
#define SIS8300_INDENTIFIER_VERSION_REG  		0x00
#define SIS8300_SERIAL_NUMBER_REG  			0x01
#define SIS8300_XILINX_JTAG_REG  			         0x02
#define SIS8300_XILINX_ECC_REG  			         0x03
#define SIS8300_USER_CONTROL_STATUS_REG 		0x04
#define SIS8300_FIRMWARE_OPTIONS_REG 		         0x05

#define SIS8300_ACQUISITION_CONTROL_STATUS_REG		0x10
#define SIS8300_SAMPLE_CONTROL_REG			         0x11
#define SIS8300_MLVDS_IO_CONTROL_REG			         0x12
#define SIS8300_HARLINK_IN_OUT_CONTROL_REG		0x13

#define SIS8300_DAC_TRIGGER_CONTROL_REG		        0x20


#define SIS8300_CLOCK_DISTRIBUTION_MUX_REG 		0x40
#define SIS8300_AD9510_SPI_REG             		                  0x41
#define SIS8300_CLOCK_MULTIPLIER_SPI_REG       		0x42

#define SIS8300_DAC_SPI_REG 		      		0x44
#define SIS8300_DAC_CONTROL_REG       		0x45
#define SIS8300_DAC_DATA_REG       			0x46
#define SIS8300_ADC_SPI_REG                		0x48
#define SIS8300_ADC_INPUT_TAP_DELAY_REG    	0x49

#define SIS8300_DAC_CLOCK_TRIGGER    		0x4E
#define SIS8300_DAC_RAM_ENDPOINT    		0x4F

#define SIS8300_VIRTEX5_SYSTEM_MONITOR_DATA_REG  	0x90
#define SIS8300_VIRTEX5_SYSTEM_MONITOR_ADDR_REG  	0x91
#define SIS8300_VIRTEX5_SYSTEM_MONITOR_CTRL_REG  	0x92

#define SIS8300_TRIGGER_SETUP_CH1_REG			0x100
#define SIS8300_TRIGGER_SETUP_CH2_REG			0x101
#define SIS8300_TRIGGER_SETUP_CH3_REG			0x102
#define SIS8300_TRIGGER_SETUP_CH4_REG			0x103
#define SIS8300_TRIGGER_SETUP_CH5_REG			0x104
#define SIS8300_TRIGGER_SETUP_CH6_REG			0x105
#define SIS8300_TRIGGER_SETUP_CH7_REG			0x106
#define SIS8300_TRIGGER_SETUP_CH8_REG			0x107
#define SIS8300_TRIGGER_SETUP_CH9_REG			0x108
#define SIS8300_TRIGGER_SETUP_CH10_REG			0x109
#define SIS8300_TRIGGER_THRESHOLD_CH1_REG		0x110
#define SIS8300_TRIGGER_THRESHOLD_CH2_REG		0x111
#define SIS8300_TRIGGER_THRESHOLD_CH3_REG		0x112
#define SIS8300_TRIGGER_THRESHOLD_CH4_REG		0x113
#define SIS8300_TRIGGER_THRESHOLD_CH5_REG		0x114
#define SIS8300_TRIGGER_THRESHOLD_CH6_REG		0x115
#define SIS8300_TRIGGER_THRESHOLD_CH7_REG		0x116
#define SIS8300_TRIGGER_THRESHOLD_CH8_REG		0x117
#define SIS8300_TRIGGER_THRESHOLD_CH9_REG		0x118
#define SIS8300_TRIGGER_THRESHOLD_CH10_REG		0x119
#define SIS8300_SAMPLE_START_ADDRESS_CH1_REG		0x120
#define SIS8300_SAMPLE_START_ADDRESS_CH2_REG		0x121
#define SIS8300_SAMPLE_START_ADDRESS_CH3_REG		0x122
#define SIS8300_SAMPLE_START_ADDRESS_CH4_REG		0x123
#define SIS8300_SAMPLE_START_ADDRESS_CH5_REG		0x124
#define SIS8300_SAMPLE_START_ADDRESS_CH6_REG		0x125
#define SIS8300_SAMPLE_START_ADDRESS_CH7_REG		0x126
#define SIS8300_SAMPLE_START_ADDRESS_CH8_REG		0x127
#define SIS8300_SAMPLE_START_ADDRESS_CH9_REG		0x128
#define SIS8300_SAMPLE_START_ADDRESS_CH10_REG		0x129
#define SIS8300_SAMPLE_LENGTH_REG			0x12A
#define SIS8300_PRETRIGGER_DELAY_REG			0x12B
#define SIS8300_TEST_HISTO_MEM_ADDR               	0x12C
#define SIS8300_TEST_HISTO_MEM_DATA_WR          	0x12D
#define SIS8300_TEST_HISTO_CONTROL			0x12E
#define SIS8300_RTM_LVDS_IO_CONTROL_REG			0x12F
// single block dma read control
#define DMA_READ_DST_ADR_LO32 0x200
#define DMA_READ_DST_ADR_HI32 0x201
#define DMA_READ_SRC_ADR_LO32 0x202
#define DMA_READ_LEN          0x203
#define DMA_READ_CTRL         0x204
// DMA_READ_CTRL bits (bit numbers)
 // write access
 #define DMA_READ_START   0
 // read access
 #define DMA_READ_RUNNING 0

// single block dma write control
#define DMA_WRITE_SRC_ADR_LO32 0x210
#define DMA_WRITE_SRC_ADR_HI32 0x211
#define DMA_WRITE_DST_ADR_LO32 0x212
#define DMA_WRITE_LEN          0x213
#define DMA_WRITE_CTRL         0x214
// DMA_READ_CTRL bits
 // write access
 #define DMA_WRITE_START   0
 // read access
 #define DMA_WRITE_RUNNING 0

// daq done dma chain control
#define DAQ_DMA_CHAIN 0x216
#define DAQ_DMA_CHAIN_ENABLE 0

// interrupt control
#define IRQ_ENABLE  0x220
#define IRQ_STATUS  0x221
#define IRQ_CLEAR   0x222
#define IRQ_REFRESH 0x223
// IRQ_ENABLE bits
 #define IRQ_MASTER_ENABLE 31
// IRQ_ENABLE & IRQ_STATUS & IRQ_CLEAR bits
 #define DMA_READ_DONE     0
 #define DMA_WRITE_DONE    1
 #define DAQ_DONE	   14
 #define USER_IRQ          15

// pcie to ddr2 test write
#define DDR2_ACCESS_CONTROL 0x230
// DDR2_ACCESS_CONTROL bits
 #define DDR2_PCIE_TEST_ENABLE 0




#define AD9510_GENERATE_FUNCTION_PULSE_CMD		0x80000000
#define AD9510_GENERATE_SPI_RW_CMD			         0x40000000

#define AD9510_SPI_SET_FUNCTION_SYNCH_FPGA_CLK69	0x10000000
#define AD9510_SPI_SELECT_NO2				         0x01000000
#define AD9510_SPI_READ_CYCLE				         0x00800000





#endif // SIS8300_REG_

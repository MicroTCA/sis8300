#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <setjmp.h>
#include <fcntl.h>
#include <unistd.h>

#include <iostream>
#include <fstream>

#include "sis8300_defs.h"
#include "sis8300_reg.h"

using namespace std;


/* useconds from struct timeval */
#define	MIKRS(tv) (((double)(tv).tv_usec ) + ((double)(tv).tv_sec * 1000000.0)) 
#define	MILLS(tv) (((double)(tv).tv_usec/1000 )  + ((double)(tv).tv_sec * 1000.0)) 


int	         fd;
struct timeval   start_time;
struct timeval   end_time;

int main(int argc, char* argv[])
{
    int  di               = 0;
    int	 ch_in        = 0;
    char nod_name[15] = "";
    device_rw	  l_RW;
    device_ioctrl_data	  l_Read;
    sis8300_reg                myReg;
    device_ioctrl_dma     DMA_RW;
    device_ioctrl_time    DMA_TIME;
    u_int	          tmp_offset;
    int                     tmp_mode;
    int      	          tmp_barx;
    u_int	          tmp_size;
    u_int	          tmp_sample;
    u_int	          tmp_pattern;
    u_int	          tmp_pattern1;
    u_int	          tmp_pattern2;
    u_int	          tmp_pattern3;
    u_int	          tmp_pattern4;
    int      	          tmp_data;
    int      	          tmp_print = 0;
    int      	          tmp_print_start = 0;
    int      	          tmp_print_stop  = 0;
    int                   len = 0;
    int                   code = 0;
    int*                  tmp_dma_buf;
    int*                  tmp_write_buf;
    int                   k = 0;
    double                time_tmp = 0;
    double                time_tmp_loop               = 0;
    double                time_tmp_loop_dlt        = 0;
    double                time_tmp_loop_drv      = 0;
    double                time_tmp_loop_dlt_drv = 0;
    double                time_dlt;
    float                 tmp_fdata;
    int                   itemsize = 0;
    int                   tmp_loop = 0;
    
   
    
    fstream dac0_file;
    fstream dac1_file;
    fstream dac11_file;
    
    int tmp_dac0_data;
    long int tmp_dac1_data;
    int dac0_data[2048];
    int dac1_data[2048];
    int dac_data[2048];
    int dac0_count;
    int dac1_count;


    itemsize = sizeof(device_rw);

    if(argc ==1){
        printf("Input \"prog /dev/sis8300-0\" \n");
        return 0;
    }
    strncpy(nod_name,argv[1],sizeof(nod_name));
    fd = open (nod_name, O_RDWR);
    if (fd < 0) {
        printf ("#CAN'T OPEN FILE \n");
        exit (1);
    }
    while (ch_in != 11){
        printf("\n READ (1) or WRITE (0) ?-");
        printf("\n GET DRIVER VERSION (2) or GET FIRMWARE VERSION (3) or GET SLOT NUM (4) ?-");
        printf("\n WRITE_IOCTL (5) or READ_ICTL (6) ?-");
        printf("\n HARLINK_OUT (7) or HARLINK_TRG (8) ?-");
        printf("\n HARLINK_TRG_FEDGE (9) or HARLINK_IN (10) ?-");
        printf("\n MLVDS_OUT (12) or MLVDS_OUT_ENBL (13) ?-");
        printf("\n MLVDS_TRG_FEDGE (14) or MLVDS_TRG (15) or MLVDS_IN (16) ?-");
        printf("\n GET_DRV_INFO (17) BLINK_USER_LED (18)?-");
        printf("\n GET SAMPLE_ADDRESS (19)?-");
        printf("\n CTRL_DMA READ (30) CTRL_DMA WRITE (31) ?-");
        printf("\n CTRL_DMA READ in LOOP (32))?-");
        printf("\n CTRL_DMA READ STAT (33) CTRL_DMA READ in LOOP STAT (34)?-");
        printf("\n PEER2PEER DMA (35))?-");
        printf("\n READ DAC0 FILE (40) READ DAC1 FILE (41) SET SPECTRUM (42)?-");
        printf("\n END (11) ?-");
        scanf("%d",&ch_in);
        fflush(stdin);
        myReg.offset   = 0;
        myReg.data     = 0;
switch (ch_in){
            case 0 :
                printf ("\n INPUT  BARx (0,1,2,3,4,5)  -");
                scanf ("%x",&tmp_barx);
                fflush(stdin);

                printf ("\n INPUT  MODE  (0-D8,1-D16,2-D32)  -");
                scanf ("%x",&tmp_mode);
                fflush(stdin);

                printf ("\n INPUT  ADDRESS (IN HEX)  -");
                scanf ("%x",&tmp_offset);
                fflush(stdin);

                printf ("\n INPUT DATA (IN HEX)  -");
                scanf ("%x",&tmp_data);
                fflush(stdin);

                l_RW.data_rw   = tmp_data;
                l_RW.offset_rw = tmp_offset;
                l_RW.mode_rw   = tmp_mode;
                l_RW.barx_rw   = tmp_barx;
                l_RW.size_rw   = 0;

                printf ("MODE - %X , OFFSET - %X, DATA - %X\n",
                     l_RW.mode_rw, l_RW.offset_rw, l_RW.data_rw);

                len = write (fd, &l_RW, sizeof(device_rw));
                if (len != itemsize ){
                        printf ("#CAN'T READ FILE \n");
                }
                break;
	    case 1 :
                printf ("\n INPUT  BARx (0,1,2,3,4,5)  -");
                scanf ("%x",&tmp_barx);
                fflush(stdin);
                printf ("\n INPUT  MODE  (0-D8,1-D16,2-D32)  -");
                scanf ("%x",&tmp_mode);
                fflush(stdin);
                printf ("\n INPUT OFFSET (IN HEX)  -");
                scanf ("%x",&tmp_offset);
                fflush(stdin);
                printf ("\n INPUT SAMPLE NUM (DEC)  -");
                scanf ("%i",&tmp_sample);
                fflush(stdin);
                                
                l_RW.data_rw    = 0;
                l_RW.offset_rw  = tmp_offset;
                l_RW.mode_rw  = tmp_mode;
                l_RW.barx_rw    = tmp_barx;
                l_RW.size_rw     = tmp_sample;
                switch(tmp_mode){
                    case 0:
                         tmp_size = sizeof(u_char)*tmp_sample;
                        break;
                   case 1:
                         tmp_size = sizeof(u_short)*tmp_sample;
                        break;
                   case 2:
                         tmp_size = sizeof(u_int)*tmp_sample;
                        break;
                  default:
                         tmp_size = sizeof(u_int)*tmp_sample;
                        break;
                }
                 printf ("MODE - %X , OFFSET - %X, SAMPLE %i DATA - %X\n", l_RW.mode_rw, l_RW.offset_rw, l_RW.size_rw , l_RW.data_rw);
                if(tmp_sample < 2){
                        len = read (fd, &l_RW, sizeof(device_rw));
                        if (len != itemsize ){
                           printf ("#CAN'T READ FILE ERROR %i \n", len);
                        }
                        printf ("READED : MODE - %X , OFFSET - %X, DATA - %X\n",  l_RW.mode_rw, l_RW.offset_rw, l_RW.data_rw);
                }else{
                       tmp_dma_buf     = new int[tmp_size + itemsize];
                       memcpy(tmp_dma_buf, &l_RW, itemsize);
                       gettimeofday(&start_time, 0);
                       len = read (fd, tmp_dma_buf, sizeof(device_rw));
                       gettimeofday(&end_time, 0);
                        if (len != itemsize ){
                           printf ("#CAN'T READ FILE ERROR %i \n", len);
                        }
                       time_tmp    =  MIKRS(end_time) - MIKRS(start_time);
                       time_dlt       =  MILLS(end_time) - MILLS(start_time);
                       printf("STOP READING TIME %fms : %fmks  SIZE %lu\n", time_dlt, time_tmp,tmp_size);
                       printf("STOP READING KBytes/Sec %f\n",(tmp_size*1000)/time_tmp);
                       
                        printf ("PRINT (0 NO, 1 YES)  -\n");
                        scanf ("%d",&tmp_print);
                        fflush(stdin);
                        while (tmp_print){
                            printf ("START POS  -\n");
                            scanf ("%d",&tmp_print_start);
                            fflush(stdin);
                            printf ("STOP POS  -\n");
                            scanf ("%d",&tmp_print_stop);
                            fflush(stdin);
                            k = tmp_print_start*4;
                            for(int i = tmp_print_start; i < tmp_print_stop; i++){
                                    printf("NUM %i OFFSET %X : DATA %X\n", i,k, (u_int)(tmp_dma_buf[i] & 0xFFFFFFFF));
                                    k += 4;
                            }
                            printf ("PRINT (0 NO, 1 YES)  -\n");
                            scanf ("%d",&tmp_print);
                            fflush(stdin);
                    }
                     if(tmp_dma_buf) delete tmp_dma_buf;
                }
	       break;
            case 2 :
                ioctl(fd, SIS8300_DRIVER_VERSION, &l_Read);
                tmp_fdata = (float)((float)l_Read.offset/10.0);
                tmp_fdata += (float)l_Read.data;
                printf ("DRIVER VERSION IS %f\n", tmp_fdata);
                break;
	    case 3 :
                ioctl(fd, SIS8300_FIRMWARE_VERSION, &l_Read);
                printf ("FIRMWARE VERSION IS - %X\n", l_Read.data);
		break;
            case 4 :
                ioctl(fd, SIS8300_PHYSICAL_SLOT, &l_Read);
                printf ("SLOT NUM IS - %X\n", l_Read.data);
                break;
            case 5 :
                printf ("\n INPUT  ADDRESS (IN HEX)  -");
                scanf ("%x",&tmp_offset);
                fflush(stdin);
                printf ("\n INPUT DATA (IN HEX)  -");
                scanf ("%x",&tmp_data);
                fflush(stdin);
                myReg.data   = tmp_data;
                myReg.offset = tmp_offset;
                printf ("OFFSET - %X, DATA - %X\n", myReg.data, myReg.offset);
                ioctl(fd, SIS8300_REG_WRITE, &myReg);
                break;
	    case 6 :
                printf ("\n INPUT OFFSET (IN HEX)  -");
                scanf ("%x",&tmp_offset);
                fflush(stdin);		
                myReg.data    = 0;
                myReg.offset  = tmp_offset;
                printf ("OFFSET - %X, DATA - %X\n", myReg.data, myReg.offset);                
                ioctl(fd, SIS8300_REG_READ, &myReg);
                printf ("READED : OFFSET - %X, DATA - %X\n", myReg.offset, myReg.data);
		break;
            case 7 : //HARLINK_OUT
                printf ("\n INPUT CHANNEL (IN HEX)  -");
                scanf ("%x",&tmp_offset);
                fflush(stdin);
                printf ("\n READ/WRITE (0/1)  -");
                scanf ("%x",&tmp_mode);
                fflush(stdin);
                tmp_data     = 0;
                if(tmp_mode){
                    printf ("\n READ/WRITE (0/1)  -");
                    scanf ("%x",&tmp_data);
                    fflush(stdin);
                }
                l_Read.data     = tmp_data;
                l_Read.offset   = tmp_offset;
                l_Read.cmd      = tmp_mode;
                l_Read.reserved = 0;
                ioctl(fd, SIS8300_HLINK_OUT, &l_Read);
                printf ("READED : CHANNEL - %X, DATA - %X\n", l_Read.offset, l_Read.data);
		break;
            case 8 : //HARLINK_TRG
                printf ("\n INPUT CHANNEL (IN HEX)  -");
                scanf ("%x",&tmp_offset);
                fflush(stdin);
                printf ("\n READ/WRITE (0/1)  -");
                scanf ("%x",&tmp_mode);
                fflush(stdin);
                tmp_data     = 0;
                if(tmp_mode){
                    printf ("\n DATA  -");
                    scanf ("%x",&tmp_data);
                    fflush(stdin);
                }
                l_Read.data     = tmp_data;
                l_Read.offset   = tmp_offset;
                l_Read.cmd      = tmp_mode;
                l_Read.reserved = 0;
                ioctl(fd, SIS8300_HLINK_TRG, &l_Read);
                printf ("READED : CHANNEL - %X, DATA - %X\n", l_Read.offset, l_Read.data);
		break;
            case 9 : //HARLINK_TRG_FEDGE
                printf ("\n INPUT CHANNEL (IN HEX)  -");
                scanf ("%x",&tmp_offset);
                fflush(stdin);
                printf ("\n READ/WRITE (0/1)  -");
                scanf ("%x",&tmp_mode);
                fflush(stdin);
                tmp_data     = 0;
                if(tmp_mode){
                    printf ("\n DATA  -");
                    scanf ("%x",&tmp_data);
                    fflush(stdin);
                }
                l_Read.data     = tmp_data;
                l_Read.offset   = tmp_offset;
                l_Read.cmd      = tmp_mode;
                l_Read.reserved = 0;
                ioctl(fd, SIS8300_HLINK_TRG_FEDGE, &l_Read);
                printf ("READED : CHANNEL - %X, DATA - %X\n", l_Read.offset, l_Read.data);
		break;
            case 10 : //HARLINK_IN
                printf ("\n INPUT CHANNEL (IN HEX)  -");
                scanf ("%x",&tmp_offset);
                fflush(stdin);
                l_Read.data     = tmp_data;
                l_Read.offset   = tmp_offset;
                l_Read.cmd      = tmp_mode;
                l_Read.reserved = 0;
                ioctl(fd, SIS8300_HLINK_IN, &l_Read);
                printf ("READED : CHANNEL - %X, DATA - %X\n", l_Read.offset, l_Read.data);
		break;
            case 12 : //MLVDS_OUT
                printf ("\n INPUT CHANNEL (IN HEX)  -");
                scanf ("%x",&tmp_offset);
                fflush(stdin);
                printf ("\n READ/WRITE (0/1)  -");
                scanf ("%x",&tmp_mode);
                fflush(stdin);
                tmp_data     = 0;
                if(tmp_mode){
                    printf ("\n DATA  -");
                    scanf ("%x",&tmp_data);
                    fflush(stdin);
                }
                l_Read.data     = tmp_data;
                l_Read.offset   = tmp_offset;
                l_Read.cmd      = tmp_mode;
                l_Read.reserved = 0;
                ioctl(fd, SIS8300_MLVDS_OUT, &l_Read);
                printf ("READED : CHANNEL - %X, DATA - %X\n", l_Read.offset, l_Read.data);
		break;
            case 13 : //MLVDS_OUT_ENBL
                printf ("\n INPUT CHANNEL (IN HEX)  -");
                scanf ("%x",&tmp_offset);
                fflush(stdin);
                printf ("\n READ/WRITE (0/1)  -");
                scanf ("%x",&tmp_mode);
                fflush(stdin);
                tmp_data     = 0;
                if(tmp_mode){
                    printf ("\n DATA  -");
                    scanf ("%x",&tmp_data);
                    fflush(stdin);
                }
                l_Read.data     = tmp_data;
                l_Read.offset   = tmp_offset;
                l_Read.cmd      = tmp_mode;
                l_Read.reserved = 0;
                ioctl(fd, SIS8300_MLVDS_OUT_ENBL, &l_Read);
                printf ("READED : CHANNEL - %X, DATA - %X\n", l_Read.offset, l_Read.data);
		break;
            case 14 : //MLVDS_TRG_FEDGE
                printf ("\n INPUT CHANNEL (IN HEX)  -");
                scanf ("%x",&tmp_offset);
                fflush(stdin);
                printf ("\n READ/WRITE (0/1)  -");
                scanf ("%x",&tmp_mode);
                fflush(stdin);
                tmp_data     = 0;
                if(tmp_mode){
                    printf ("\n DATA  -");
                    scanf ("%x",&tmp_data);
                    fflush(stdin);
                }
                l_Read.data     = tmp_data;
                l_Read.offset   = tmp_offset;
                l_Read.cmd      = tmp_mode;
                l_Read.reserved = 0;
                ioctl(fd, SIS8300_MLVDS_TRG_EDGE, &l_Read);
                printf ("READED : CHANNEL - %X, DATA - %X\n", l_Read.offset, l_Read.data);
		break;
            case 15 : //MLVDS_TRG
                printf ("\n INPUT CHANNEL (IN HEX)  -");
                scanf ("%x",&tmp_offset);
                fflush(stdin);
                printf ("\n READ/WRITE (0/1)  -");
                scanf ("%x",&tmp_mode);
                fflush(stdin);
                tmp_data     = 0;
                if(tmp_mode){
                    printf ("\n DATA  -");
                    scanf ("%x",&tmp_data);
                    fflush(stdin);
                }
                l_Read.data     = tmp_data;
                l_Read.offset   = tmp_offset;
                l_Read.cmd      = tmp_mode;
                l_Read.reserved = 0;
                ioctl(fd, SIS8300_MLVDS_TRG, &l_Read);
                printf ("READED : CHANNEL - %X, DATA - %X\n", l_Read.offset, l_Read.data);
		break;
            case 16 : //MLVDS_IN
                printf ("\n INPUT CHANNEL (IN HEX)  -");
                scanf ("%x",&tmp_offset);
                fflush(stdin);
                l_Read.data     = tmp_data;
                l_Read.offset   = tmp_offset;
                l_Read.cmd     = tmp_mode;
                l_Read.reserved = 0;
                ioctl(fd, SIS8300_MLVDS_IN, &l_Read);
                printf ("READED : CHANNEL - %X, DATA - %X\n", l_Read.offset, l_Read.data);
                break;
          case 17 :
                fflush(stdin);
                l_RW.data_rw   = 0;
                l_RW.offset_rw = 0;
                l_RW.mode_rw   = RW_INFO;
                l_RW.barx_rw   = 0;
                l_RW.size_rw   = 0;
                len = read (fd, &l_RW, sizeof(device_rw));
                if (len != itemsize ){
                    printf ("#CAN'T READ FILE \n");
                }
                printf ("READED : SLOT             - %X \n", l_RW.barx_rw);
                printf ("READED : FIRMWARE VERSION - %X \n", l_RW.mode_rw);
                tmp_fdata = (float)((float)l_RW.offset_rw/10.0);
                tmp_fdata += (float)l_RW.data_rw;
                printf ("DRIVER VERSION IS %f\n", tmp_fdata);
               break;
          case 18 :
                printf ("\n INPUT  DELAY (usec)  -");
                scanf ("%x",&tmp_data);
                fflush(stdin);
                printf ("\n INPUT  ON->OFF (1), OFF->ON (0) (IN HEX)");
                scanf ("%x",&tmp_offset);
                fflush(stdin);
                myReg.offset = tmp_offset;
                myReg.data   = tmp_data;
                ioctl(fd, SIS8300_BLINK_LED, &myReg);
                break;
           case 19 : //GET_SAMPLE ADDRESS
                printf ("\n INPUT CHANNEL (IN HEX)  -");
                scanf ("%x",&tmp_offset);
                fflush(stdin);
                tmp_data     = 0;
                l_Read.data     = tmp_data;
                l_Read.offset   = tmp_offset;
                l_Read.cmd      = IOCTRL_R;
                l_Read.reserved = 0;
                ioctl(fd, SIS8300_SAMPLE_ADDR, &l_Read);
                printf ("READED : CHANNEL - %X, DATA - %X\n", l_Read.offset, l_Read.data);
               break;
           case 30 :
                DMA_RW.dma_offset  = 0;
                DMA_RW.dma_size    = 0;
                DMA_RW.dma_cmd     = 0;
                DMA_RW.dma_pattern = 0; 
                printf ("\n INPUT  DMA_SIZE (num of sumples (int))  -");
                scanf ("%d",&tmp_size);
                fflush(stdin);
                DMA_RW.dma_size    = sizeof(int)*tmp_size;
                printf ("\n INPUT OFFSET (HEX)  -");
                scanf ("%x",&tmp_offset);
                fflush(stdin);
                DMA_RW.dma_offset = tmp_offset;
                
                printf ("DMA_OFFSET - %X, DMA_SIZE - %X\n", DMA_RW.dma_offset, DMA_RW.dma_size);
                printf ("MAX_MEM- %X, DMA_MEM - %X:%X\n", 536870912,  (DMA_RW.dma_offset + DMA_RW.dma_size),
                                                                                              (DMA_RW.dma_offset + DMA_RW.dma_size*4));
                
                tmp_dma_buf     = new int[tmp_size + DMA_DATA_OFFSET];
                memcpy(tmp_dma_buf, &DMA_RW, sizeof (device_ioctrl_dma));
                // enable ddr2 test write interface
                myReg.offset = DDR2_ACCESS_CONTROL;
                myReg.data = (1<<DDR2_PCIE_TEST_ENABLE);
                ioctl(fd, SIS8300_REG_WRITE, &myReg);  
                
                gettimeofday(&start_time, 0);
                code = ioctl (fd, SIS8300_READ_DMA, tmp_dma_buf);
                gettimeofday(&end_time, 0);
                printf ("===========READED  CODE %i\n", code);
                time_tmp    =  MIKRS(end_time) - MIKRS(start_time);
                time_dlt       =  MILLS(end_time) - MILLS(start_time);
                printf("STOP READING TIME %fms : %fmks  SIZE %lu\n", time_dlt, time_tmp,(sizeof(int)*tmp_size));
                printf("STOP READING KBytes/Sec %f\n",((sizeof(int)*tmp_size*1000)/time_tmp));
                code = ioctl (fd, SIS8300_GET_DMA_TIME, &DMA_TIME);
                if (code) {
                    printf ("######ERROR GET TIME %d\n", code);
                }
                printf ("===========DRIVER TIME \n");
                time_tmp = MIKRS(DMA_TIME.stop_time) - MIKRS(DMA_TIME.start_time);
                time_dlt    = MILLS(DMA_TIME.stop_time) - MILLS(DMA_TIME.start_time);
                printf("STOP DRIVER TIME START %li:%li STOP %li:%li\n",
                                                            DMA_TIME.start_time.tv_sec, DMA_TIME.start_time.tv_usec, 
                                                            DMA_TIME.stop_time.tv_sec, DMA_TIME.stop_time.tv_usec);
                printf("STOP DRIVER READING TIME %fms : %fmks  SIZE %lu\n", time_dlt, time_tmp,(sizeof(int)*tmp_size));
                printf("STOP DRIVER READING KBytes/Sec %f\n",((sizeof(int)*tmp_size*1000)/time_tmp));
                printf ("PRINT (0 NO, 1 YES)  -\n");
                scanf ("%d",&tmp_print);
                fflush(stdin);
                while (tmp_print){
                    printf ("START POS  -\n");
                    scanf ("%d",&tmp_print_start);
                    fflush(stdin);
                    printf ("STOP POS  -\n");
                    scanf ("%d",&tmp_print_stop);
                    fflush(stdin);
                    k = tmp_print_start*4;
                    for(int i = tmp_print_start; i < tmp_print_stop; i++){
                            printf("NUM %i OFFSET %X : DATA %X\n", i,k, (u_int)(tmp_dma_buf[i] & 0xFFFFFFFF));
                            k += 4;
                    }
                    printf ("PRINT (0 NO, 1 YES)  -\n");
		    scanf ("%d",&tmp_print);
		    fflush(stdin);
                }
                if(tmp_dma_buf) delete tmp_dma_buf;
                break;
        case 31 :
            DMA_RW.dma_offset  = 0;
            DMA_RW.dma_size    = 0;
            DMA_RW.dma_cmd     = 0;
            DMA_RW.dma_pattern = 0; 
            DMA_RW.dma_reserved1 = 33; 
            DMA_RW.dma_reserved2 = 44;
            printf ("\n INPUT  DMA_SIZE (num of sumples (int))  -");
            scanf ("%d",&tmp_size);
            fflush(stdin);
            DMA_RW.dma_size = sizeof(int)*tmp_size;
            printf ("\n INPUT OFFSET (HEX)  -");
            scanf ("%x",&tmp_offset);
            fflush(stdin);
            DMA_RW.dma_offset = tmp_offset;

            printf ("\n INPUT PATTERN (HEX)  -");
            scanf ("%x",&tmp_pattern);
            fflush(stdin);

            tmp_write_buf     = new int[tmp_size + DMA_DATA_OFFSET];
            memcpy(tmp_write_buf, &DMA_RW, sizeof (device_ioctrl_dma));
              
            if(tmp_offset == 0xB0000000){
                printf ("\n INPUT PATTERN DAC01 first 20 (HEX)  -");
                scanf ("%x",&tmp_pattern1);
                fflush(stdin);
                printf ("\n INPUT PATTERN DAC01 second 20(HEX)  -");
                scanf ("%x",&tmp_pattern2);
                fflush(stdin);
                printf ("\n INPUT PATTERN DAC01 next 20 (HEX)  -");
                scanf ("%x",&tmp_pattern3);
                fflush(stdin);
                printf ("\n INPUT PATTERN DAC01 REST (HEX)  -");
                scanf ("%x",&tmp_pattern4);
                fflush(stdin);
                for(int ii = (tmp_pattern + 0); ii < (tmp_pattern + 20); ++ii){
                    tmp_write_buf[ii + DMA_DATA_OFFSET] =tmp_pattern1;
                }
                for(int ii = (tmp_pattern + 20); ii < (tmp_pattern + 40); ++ii){
                    tmp_write_buf[ii + DMA_DATA_OFFSET] =tmp_pattern2;
                }
                for(int ii = (tmp_pattern + 40); ii < (tmp_pattern + 60); ++ii){
                    tmp_write_buf[ii + DMA_DATA_OFFSET] =tmp_pattern3;
                }
                for(int ii = (tmp_pattern + 60); ii < tmp_size; ++ii){
                    tmp_write_buf[ii + DMA_DATA_OFFSET] = tmp_pattern4;
                }
            }else{
                for(int ii = 0; ii < tmp_size; ++ii){
                    tmp_write_buf[ii + DMA_DATA_OFFSET] = ((tmp_pattern + ii) & 0xFFFF) + (((tmp_pattern + ii) & 0xFFFF)<< 16);
                }
            }
            k= 0;
            for(int i = 0; i < 10; i++){
                printf("OFFSET %d : DATA %X\n", k, (u_int)(tmp_write_buf[i] & 0xFFFFFFFF));
                k++;
            }
            // disable ddr2 test write interface
            myReg.offset = DDR2_ACCESS_CONTROL;
            myReg.data = 1;
            ioctl(fd, SIS8300_REG_WRITE, &myReg);  
            
            if(tmp_offset == 0xB0000000){
                printf("WRITING TO DAC RAM\n");
                /* DAC Reset */
                printf("Reset DAC...               ");
                myReg.offset = SIS8300_DAC_CONTROL_REG;
                myReg.data = 0x300;
                ioctl(fd, SIS8300_REG_WRITE, &myReg);  
                printf("OK\n");
                /* Setup DAC Values */
                printf("Setup DAC Values...        ");
                myReg.offset = DDR2_ACCESS_CONTROL;
                myReg.data = 0x123;
                ioctl(fd, SIS8300_REG_WRITE, &myReg);  
                myReg.offset = DMA_WRITE_DST_ADR_LO32;
                myReg.data = 0xB0000000;
                ioctl(fd, SIS8300_REG_WRITE, &myReg);  
                myReg.offset = SIS8300_DAC_CONTROL_REG;
                myReg.data = 0xE3;
                ioctl(fd, SIS8300_REG_WRITE, &myReg);  
            }

            code = ioctl (fd, SIS8300_WRITE_DMA, tmp_write_buf);
            if (code) {
                printf ("######ERROR DMA  %d\n", code);
            }   
            
            if(tmp_offset == 0xB0000000){
                printf("WRITING TO DAC RAM\n");
                myReg.offset = SIS8300_DAC_CONTROL_REG;
                myReg.data = 0x23;
                ioctl(fd, SIS8300_REG_WRITE, &myReg);  
            }
            
            if(tmp_write_buf) delete tmp_write_buf;
            break;
        case 32 :
                DMA_RW.dma_offset  = 0;
                DMA_RW.dma_size    = 0;
                DMA_RW.dma_cmd     = 0;
                DMA_RW.dma_pattern = 0; 
                printf ("\n INPUT  DMA_SIZE (num of sumples (int))  -");
                scanf ("%d",&tmp_size);
                fflush(stdin);
                DMA_RW.dma_size    = sizeof(int)*tmp_size;
                printf ("\n INPUT OFFSET (IN HEX)  -");
                scanf ("%x",&tmp_offset);
                fflush(stdin);
                printf ("\n INPUT LOOP NUM  -");
                scanf ("%i",&tmp_loop);
                fflush(stdin);
                
                
                DMA_RW.dma_offset = tmp_offset;
                tmp_dma_buf     = new int[tmp_size + DMA_DATA_OFFSET];
                
                
                // enable ddr2 test write interface
                myReg.offset = DDR2_ACCESS_CONTROL;
                myReg.data = (1<<DDR2_PCIE_TEST_ENABLE);
                ioctl(fd, SIS8300_REG_WRITE, &myReg);  
                
                gettimeofday(&start_time, 0);
                for(int i = 0; i < tmp_loop; i++){
                        DMA_RW.dma_offset  = 0;
                        DMA_RW.dma_size    = 0;
                        DMA_RW.dma_cmd     = 0;
                        DMA_RW.dma_pattern = 0; 
                        
                        DMA_RW.dma_size    = sizeof(int)*tmp_size;
                        DMA_RW.dma_offset = tmp_offset + sizeof(int)*tmp_size*i;
                        memcpy(tmp_dma_buf, &DMA_RW, sizeof (device_ioctrl_dma));
                        code = ioctl (fd, SIS8300_READ_DMA, tmp_dma_buf);
                        
                 }
                gettimeofday(&end_time, 0);
                printf ("===========READED  CODE %i\n", code);
                time_tmp    =  MIKRS(end_time) - MIKRS(start_time);
                time_dlt       =  MILLS(end_time) - MILLS(start_time);
                printf("STOP READING TIME %fms : %fmks  SIZE %lu\n", time_dlt, time_tmp,(sizeof(int)*tmp_size));
                printf("STOP READING KBytes/Sec %f\n",((sizeof(int)*tmp_size*1000*tmp_loop)/time_tmp));
                code = ioctl (fd, SIS8300_GET_DMA_TIME, &DMA_TIME);
                if (code) {
                    printf ("######ERROR GET TIME %d\n", code);
                }
                printf ("===========DRIVER TIME \n");
                time_tmp = MIKRS(DMA_TIME.stop_time) - MIKRS(DMA_TIME.start_time);
                time_dlt    = MILLS(DMA_TIME.stop_time) - MILLS(DMA_TIME.start_time);
                printf("STOP DRIVER TIME START %li:%li STOP %li:%li\n",
                                                            DMA_TIME.start_time.tv_sec, DMA_TIME.start_time.tv_usec, 
                                                            DMA_TIME.stop_time.tv_sec, DMA_TIME.stop_time.tv_usec);
                printf("STOP DRIVER READING TIME %fms : %fmks  SIZE %lu\n", time_dlt, time_tmp,(sizeof(int)*tmp_size));
                printf("STOP DRIVER READING KBytes/Sec %f\n",((sizeof(int)*tmp_size*1000)/time_tmp));
                printf ("PRINT (0 NO, 1 YES)  -\n");
                scanf ("%d",&tmp_print);
                fflush(stdin);
                while (tmp_print){
                    printf ("START POS  -\n");
                    scanf ("%d",&tmp_print_start);
                    fflush(stdin);
                    printf ("STOP POS  -\n");
                    scanf ("%d",&tmp_print_stop);
                    fflush(stdin);
                    k = tmp_print_start*4;
                    for(int i = tmp_print_start; i < tmp_print_stop; i++){
                            printf("NUM %i OFFSET %X : DATA %X\n", i,k, (u_int)(tmp_dma_buf[i] & 0xFFFFFFFF));
                            k += 4;
                    }
                    printf ("PRINT (0 NO, 1 YES)  -\n");
		    scanf ("%d",&tmp_print);
		    fflush(stdin);
                }
                if(tmp_dma_buf) delete tmp_dma_buf;
                break;
         case 33 :
                DMA_RW.dma_offset  = 0;
                DMA_RW.dma_size    = 0;
                DMA_RW.dma_cmd     = 0;
                DMA_RW.dma_pattern = 0; 
                printf ("\n INPUT  DMA_SIZE (num of sumples (int))  -");
                scanf ("%d",&tmp_size);
                fflush(stdin);
                DMA_RW.dma_size    = sizeof(int)*tmp_size;
                printf ("\n INPUT OFFSET (IN HEX)  -");
                scanf ("%x",&tmp_offset);
                fflush(stdin);
                DMA_RW.dma_offset = tmp_offset;
                tmp_dma_buf              = new int[tmp_size + DMA_DATA_OFFSET];
                memcpy(tmp_dma_buf, &DMA_RW, sizeof (device_ioctrl_dma));
                myReg.offset = DDR2_ACCESS_CONTROL;
                myReg.data = (1<<DDR2_PCIE_TEST_ENABLE);
                ioctl(fd, SIS8300_REG_WRITE, &myReg);  
                time_tmp_loop        = 0;
                time_tmp_loop_dlt = 0;
                time_tmp_loop_drv      = 0;
                time_tmp_loop_dlt_drv = 0;
                 
                 for(k = 0 ; k <= 1000; k++){
                     //usleep(2);
                     DMA_RW.dma_offset  = 0;
                     DMA_RW.dma_size      = 0;
                     DMA_RW.dma_cmd     = 0;
                     DMA_RW.dma_pattern = 0; 
                     DMA_RW.dma_size    = sizeof(int)*tmp_size;
                     DMA_RW.dma_offset = tmp_offset;
                     memcpy(tmp_dma_buf, &DMA_RW, sizeof (device_ioctrl_dma));
                     gettimeofday(&start_time, 0);
                     code = ioctl (fd, SIS8300_READ_DMA, tmp_dma_buf);
                     gettimeofday(&end_time, 0);
                     time_tmp    =  MIKRS(end_time) - MIKRS(start_time);
                     time_dlt       =  MILLS(end_time) - MILLS(start_time);
                     time_tmp_loop        += time_tmp;
                     time_tmp_loop_dlt += time_dlt;
                     code = ioctl (fd, SIS8300_GET_DMA_TIME, &DMA_TIME);
                    if (code) {
                        printf ("######ERROR GET TIME %d\n", code);
                    }
                    time_tmp = MIKRS(DMA_TIME.stop_time) - MIKRS(DMA_TIME.start_time);
                    time_dlt    = MILLS(DMA_TIME.stop_time) - MILLS(DMA_TIME.start_time);
                    time_tmp_loop_drv       += time_tmp;
                    time_tmp_loop_dlt_drv += time_dlt;
                    usleep(2000);
                 }
                 
                time_tmp_loop                 = time_tmp_loop/1000;
                time_tmp_loop_dlt          = time_tmp_loop_dlt/1000;
                time_tmp_loop_drv         = time_tmp_loop_drv/1000;
                time_tmp_loop_dlt_drv  = time_tmp_loop_dlt_drv/1000;
                printf("STOP READING TIME %fms : %fmks  SIZE %lu\n", time_tmp_loop_dlt, time_tmp_loop,(sizeof(int)*tmp_size));
                printf("STOP READING KBytes/Sec %f\n",((sizeof(int)*tmp_size*1000)/time_tmp_loop));
                printf ("===========DRIVER TIME \n");
                printf("STOP DRIVER READING TIME %fms : %fmks  SIZE %lu\n", time_tmp_loop_dlt_drv, time_tmp_loop_drv,(sizeof(int)*tmp_size));
                printf("STOP DRIVER READING KBytes/Sec %f\n",((sizeof(int)*tmp_size*1000)/time_tmp_loop_drv));
                
                if(tmp_dma_buf) delete tmp_dma_buf;
                break;
        case 34 :
                DMA_RW.dma_offset  = 0;
                DMA_RW.dma_size    = 0;
                DMA_RW.dma_cmd     = 0;
                DMA_RW.dma_pattern = 0; 
                printf ("\n INPUT  DMA_SIZE (num of sumples (int))  -");
                scanf ("%d",&tmp_size);
                fflush(stdin);
                DMA_RW.dma_size    = sizeof(int)*tmp_size;
                printf ("\n INPUT OFFSET (IN HEX)  -");
                scanf ("%x",&tmp_offset);
                fflush(stdin);
                printf ("\n INPUT LOOP NUM  -");
                scanf ("%i",&tmp_loop);
                fflush(stdin);
                
                
                DMA_RW.dma_offset = tmp_offset;
                tmp_dma_buf     = new int[tmp_size + DMA_DATA_OFFSET];
                
                
                // enable ddr2 test write interface
                myReg.offset = DDR2_ACCESS_CONTROL;
                myReg.data = (1<<DDR2_PCIE_TEST_ENABLE);
                ioctl(fd, SIS8300_REG_WRITE, &myReg);  
                
                time_tmp_loop        = 0;
                time_tmp_loop_dlt = 0;
                time_tmp_loop_drv      = 0;
                time_tmp_loop_dlt_drv = 0;
                
                for(k = 0 ; k <= 1000; k++){
                     //usleep(2);
                    gettimeofday(&start_time, 0);
                    for(int i = 0; i < tmp_loop; i++){
                            DMA_RW.dma_offset  = 0;
                            DMA_RW.dma_size    = 0;
                            DMA_RW.dma_cmd     = 0;
                            DMA_RW.dma_pattern = 0; 

                            DMA_RW.dma_size    = sizeof(int)*tmp_size;
                            DMA_RW.dma_offset = tmp_offset + sizeof(int)*tmp_size*i;
                            memcpy(tmp_dma_buf, &DMA_RW, sizeof (device_ioctrl_dma));
                            code = ioctl (fd, SIS8300_READ_DMA, tmp_dma_buf);

                        }
                    gettimeofday(&end_time, 0);
                    time_tmp    =  MIKRS(end_time) - MIKRS(start_time);
                    time_dlt       =  MILLS(end_time) - MILLS(start_time);
                    time_tmp_loop        += time_tmp;
                    time_tmp_loop_dlt  += time_dlt;
                   code = ioctl (fd, SIS8300_GET_DMA_TIME, &DMA_TIME);
                    if (code) {
                        printf ("######ERROR GET TIME %d\n", code);
                    }
                    time_tmp = MIKRS(DMA_TIME.stop_time) - MIKRS(DMA_TIME.start_time);
                    time_dlt    = MILLS(DMA_TIME.stop_time) - MILLS(DMA_TIME.start_time);
                    time_tmp_loop_drv        += time_tmp;
                    time_tmp_loop_dlt_drv  += time_dlt;
                    usleep(2000);
                 }
                time_tmp_loop                = time_tmp_loop/1000;
                time_tmp_loop_dlt          = time_tmp_loop_dlt/1000;
                 time_tmp_loop_drv        = time_tmp_loop_drv/1000;
                time_tmp_loop_dlt_drv  = time_tmp_loop_dlt_drv/1000;
                
                printf("STOP READING TIME %fms : %fmks  SIZE %lu\n", time_tmp_loop_dlt, time_tmp_loop,(sizeof(int)*tmp_size));
                printf("STOP READING KBytes/Sec %f\n",((sizeof(int)*tmp_size*1000*tmp_loop)/time_tmp_loop));
                 printf ("===========DRIVER TIME \n");
               printf("STOP DRIVER READING TIME %fms : %fmks  SIZE %lu\n", time_tmp_loop_dlt_drv, time_tmp_loop_drv,(sizeof(int)*tmp_size));
                printf("STOP DRIVER READING KBytes/Sec %f\n",((sizeof(int)*tmp_size*1000)/time_tmp_loop_drv));
                
                if(tmp_dma_buf) delete tmp_dma_buf;
                break;
				
	case 35 :
                DMA_RW.dma_offset  = 0;
                DMA_RW.dma_size    = 0;
                DMA_RW.dma_cmd     = 0;
                DMA_RW.dma_pattern = 0; 
				
	            printf ("\n INPUT COMAND (0-get address from universal driver, 3 use OFFSET as adress)  -");
                scanf ("%d",&tmp_data);
                fflush(stdin);
                DMA_RW.dma_cmd = tmp_data;
				
                printf ("\n INPUT  DMA_SIZE (num of sumples (int))  -");
                scanf ("%d",&tmp_size);
                fflush(stdin);
                DMA_RW.dma_size    = sizeof(int)*tmp_size;
				
                printf ("\n INPUT OSURCE OFFSET (HEX)  -");
                scanf ("%x",&tmp_offset);
                fflush(stdin);
                DMA_RW.dma_offset = tmp_offset;
                
                printf ("\n INPUT DEST OFFSET (HEX)  -");
                scanf ("%x",&tmp_offset);
                fflush(stdin);
                DMA_RW.dma_reserved2 = tmp_offset;
	      
	            printf ("\n INPUT  PERR SLOT NUM  -");
                scanf ("%d",&tmp_mode);
                fflush(stdin);
                DMA_RW.dma_reserved1    = (tmp_mode << 16) & 0xFFFF0000;
				
                printf ("\n INPUT PEER BAR NUM  -");
                scanf ("%d",&tmp_barx);
                fflush(stdin);
                DMA_RW.dma_reserved1 += (tmp_barx & 0xFFFF);
                
				
		
                printf ("SLOT - %i, BAR %i OFFSET %X DMA_SIZE - %X\n", DMA_RW.dma_reserved1, DMA_RW.dma_reserved2, DMA_RW.dma_offset, DMA_RW.dma_size);
                
                // enable ddr2 test write interface
                myReg.offset = DDR2_ACCESS_CONTROL;
                myReg.data = (1<<DDR2_PCIE_TEST_ENABLE);
                ioctl(fd, SIS8300_REG_WRITE, &myReg);  
                
                gettimeofday(&start_time, 0);
                code = ioctl (fd, SIS8300_WRITE_DMA_2PEER, &DMA_RW);
                gettimeofday(&end_time, 0);
                printf ("===========READED  CODE %i\n", code);
                time_tmp    =  MIKRS(end_time) - MIKRS(start_time);
                time_dlt       =  MILLS(end_time) - MILLS(start_time);
                printf("STOP READING TIME %fms : %fmks  SIZE %lu\n", time_dlt, time_tmp,(sizeof(int)*tmp_size));
                printf("STOP READING KBytes/Sec %f\n",((sizeof(int)*tmp_size*1000)/time_tmp));
                code = ioctl (fd, SIS8300_GET_DMA_TIME, &DMA_TIME);
                if (code) {
                    printf ("######ERROR GET TIME %d\n", code);
                }
                printf ("===========DRIVER TIME \n");
                time_tmp = MIKRS(DMA_TIME.stop_time) - MIKRS(DMA_TIME.start_time);
                time_dlt    = MILLS(DMA_TIME.stop_time) - MILLS(DMA_TIME.start_time);
                printf("STOP DRIVER TIME START %li:%li STOP %li:%li\n",
                                                            DMA_TIME.start_time.tv_sec, DMA_TIME.start_time.tv_usec, 
                                                            DMA_TIME.stop_time.tv_sec, DMA_TIME.stop_time.tv_usec);
                printf("STOP DRIVER READING TIME %fms : %fmks  SIZE %lu\n", time_dlt, time_tmp,(sizeof(int)*tmp_size));
                printf("STOP DRIVER READING KBytes/Sec %f\n",((sizeof(int)*tmp_size*1000)/time_tmp));
                break;
				
        case 40:
//            std::fstream dac0_file("/home/petros/doocs.git/doocs/server/common/sis8300dma/dac0_data.txt", std::ios_base::in);
//            std::fstream dac1_file("/home/petros/doocs.git/doocs/server/common/sis8300dma/dac0_data.txt", std::ios_base::in);
//            int tmp_dac0_data;
//            int tmp_dac1_data;
//            int dac0_data[2048];
//            int dac1_data[2048];
//            int dac_data[2048];
//            int dac0_count;
//            int dac1_count;
            dac0_file.open("/home/petros/doocs.git/doocs/server/common/sis8300dma/dac0_data.txt", fstream::in|fstream::out|fstream::app);
            dac0_file >> std::hex;
            dac0_count = 0;
            while (dac0_file >> tmp_dac0_data)
            {
                printf("%X ", tmp_dac0_data);
                dac0_count++;
            }
            getchar();
            printf("\n DAC0_COUNT %i\n", dac0_count);
            dac0_file.close();
            break;
        case 41:
            dac1_file.open("/home/petros/doocs/source/unixdriver/utca/linux/sis8300/dac0_data.txt", fstream::in|fstream::out|fstream::app);
            dac1_file >> std::hex;
            dac1_count = 0;
            if(!dac1_file.is_open()){
                printf("???????????????ERROR COULD NOT OPEN DAC0_DATA.TXT \n");
//               fs.clear();
//               fs.open(filename, std::ios::out); //Create file.
//               fs.close();
//               fs.open(filename);
            }
            while (dac1_file >> tmp_dac1_data)
            {
                //printf("%X ", tmp_dac1_data);
                //dac1_data[dac1_count] =(int)( (tmp_dac1_data >> 16) & 0xFFFF);
                dac1_data[dac1_count] =(int)( tmp_dac1_data);
                dac1_count++;
            }
            getchar();
            printf("\n DAC1_COUNT %i\n", dac1_count);
            dac1_file.flush();
            dac1_file.close();
            //dac11_file.open("/home/petros/doocs/source/unixdriver/utca/linux/sis8300/dac11_data.txt", fstream::out|fstream::trunc|fstream::app);
            dac11_file.open("/home/petros/doocs/source/unixdriver/utca/linux/sis8300/dac11_data.txt", fstream::out|fstream::trunc);
            //dac11_file << std::hex;
            if(!dac11_file.is_open()){
               printf("???????????????ERROR COULD NOT OPEN DAC11_DATA.TXT \n");
               dac11_file.clear();
               dac11_file.open("/home/petros/doocs/source/unixdriver/utca/linux/sis8300/dac11_data.txt", std::ios::out); //Create file.
               dac11_file.close();
               dac11_file.open("/home/petros/doocs/source/unixdriver/utca/linux/sis8300/dac11_data.txt", fstream::out|fstream::trunc|fstream::app);
                if(!dac11_file.is_open()){
                   printf("???????????????ERROR COULD NOT OPEN DAC11_DATA.TXT \n");
                }
            }
            for(di =0; di < dac1_count; di++){
                //printf("WRITING DATA TO FILE %i ; %X\n",di, dac1_data[di]);
                dac11_file <<  dac1_data[di] << "\n";
                //dac1_file  << std::endl;
                //dac11_file << dac1_data[di] << "\n";
            }
            dac11_file.close();
            break;
   
    default:
          break;
	}
    }
    close(fd);
    return 0;
}


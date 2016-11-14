#include <linux/module.h>
#include <linux/fs.h>	
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/timer.h>

#include "sis8300_fnc.h"
#include "sis8300_defs.h"
#include "sis8300_reg.h"

MODULE_AUTHOR("Lyudvig Petrosyan");
MODULE_DESCRIPTION("SIS8300 board driver");
MODULE_VERSION("5.0.0");
MODULE_LICENSE("Dual BSD/GPL");

pciedev_cdev     *sis8300_cdev_m = 0;

static int        sis8300_open( struct inode *inode, struct file *filp );
static int        sis8300_release(struct inode *inode, struct file *filp);
static ssize_t sis8300_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos);
static ssize_t sis8300_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos);
static long     sis8300_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);

struct file_operations sis8300_fops = {
    .owner                   =  THIS_MODULE,
    .read                     =  sis8300_read,
    .write                    =  sis8300_write,
    .unlocked_ioctl    =  sis8300_ioctl,
    .open                    =  sis8300_open,
    .release                =  sis8300_release,
};

static struct pci_device_id sis8300_ids[] = {
    { PCI_DEVICE(SIS8300_VENDOR_ID, SIS8300_DEVICE_ID), },
    { PCI_DEVICE(SIS8300_VENDOR_ID, SIS8300L_DEVICE_ID), },
    { 0, }
};
MODULE_DEVICE_TABLE(pci, sis8300_ids);

/*
 * The top-half interrupt handler.
 */
static irqreturn_t sis8300_interrupt(int irq, void *dev_id UPKCOMPAT_IHARGPOS(regs) )
{
    uint32_t intreg = 0;
    void*                   address;
    
    struct pciedev_dev *pdev   = (pciedev_dev*)dev_id;
    struct sis8300_dev *dev     = (sis8300_dev *)(pdev->dev_str);
    
    address = pciedev_get_baraddress(BAR0, pdev);
    
    //intreg       = ioread32((void*)pdev->memmory_base0 + IRQ_STATUS*4);
    intreg       = ioread32(address + IRQ_STATUS*4);
    smp_rmb();
     if(intreg == 0){
        return IRQ_NONE;
    }
    if(dev->waitFlag){
        //iowrite32(intreg, ((void*)((void*)pdev->memmory_base0 + IRQ_CLEAR*4)));
        iowrite32(intreg, (address + IRQ_CLEAR*4));
        return IRQ_HANDLED;
    }
    //iowrite32(intreg, ((void*)((void*)pdev->memmory_base0 + IRQ_CLEAR*4)));
    iowrite32(intreg, (address + IRQ_CLEAR*4));
    dev->waitFlag = 1;
    wake_up_interruptible(&(dev->waitDMA));
    return IRQ_HANDLED;
}

static int UPKCOMPAT_INIT sis8300_probe(struct pci_dev *dev, const struct pci_device_id *id)
{
    int result                = 0;
    u32 tmp_info          = 0;
    pciedev_dev       *sis8300_pcie_dev;
    sis8300_dev       *sis8300_dev_pp;
    void*                   address;
    
    printk(KERN_ALERT "SIS8300-PCIEDEV_PROBE CALLED \n");
    result = pciedev_probe2_exp(dev, id, &sis8300_fops, sis8300_cdev_m, &sis8300_pcie_dev);
    printk(KERN_ALERT "SIS8300-PCIEDEV_PROBE_EXP CALLED  FOR BOARD result %i\n", result);
    /*if board has created we will create our structure and pass it to pcedev_dev*/
    if(!result){
        
        if(!(sis8300_pcie_dev->pciedev_all_mems)){
            printk(KERN_ALERT "SIS8300-PCIEDEV_PROBE CALLED; NO BARs \n");
            result = pciedev_remove2_exp(sis8300_pcie_dev);
            printk(KERN_ALERT "SIS8300-PCIEDEV_REMOVE_EXP CALLED  FOR SLOT %i\n", sis8300_pcie_dev->brd_num);  
            return -ENOMEM;
        }
 
        printk(KERN_ALERT "SIS8300-PCIEDEV_PROBE_EXP CREATING CURRENT STRUCTURE FOR BOARD %i\n", sis8300_pcie_dev->brd_num);
        // allocate private data
        sis8300_dev_pp = kzalloc(sizeof(sis8300_dev), GFP_KERNEL);
        if(!sis8300_dev_pp){
                return -ENOMEM;
        }
        printk(KERN_ALERT "SIS8300-PCIEDEV_PROBE CALLED; CURRENT STRUCTURE CREATED \n");
        sis8300_dev_pp->parent_dev  = sis8300_pcie_dev;
        init_waitqueue_head(&sis8300_dev_pp->waitDMA);
        pciedev_set_drvdata(sis8300_pcie_dev, sis8300_dev_pp);
        pciedev_setup_interrupt_exp(sis8300_interrupt, sis8300_pcie_dev);
        
        /*****Switch ON USER_LED*****/
        //address = sis8300_pcie_dev->memmory_base0;
        address = pciedev_get_baraddress(BAR0, sis8300_pcie_dev);
        iowrite32(0x1, (address + SIS8300_USER_CONTROL_STATUS_REG*4));
        smp_wmb();
        /*Collect INFO*/
        tmp_info = ioread32(address + 0x0);
        smp_rmb();
        sis8300_pcie_dev->brd_info_list.PCIEDEV_BOARD_ID = tmp_info;
        
        sis8300_dev_pp->fpga_1_gb =0;
        sis8300_dev_pp->dual_optical_interface = 0;
        sis8300_dev_pp->dual_port_14_15_interface = 0;
        sis8300_dev_pp->dual_port_12_13_interface = 0;
        sis8300_dev_pp->dual_channel_sampling = 0;
        sis8300_dev_pp->ringbuffer_delay = 0;
        sis8300_dev_pp->trigger_block_enable = 0;
        tmp_info = ioread32(address + 0x0);
        smp_rmb();
        sis8300_dev_pp->sis8300_mem_max_size = 536870912;
        sis8300_dev_pp->fpga_1_gb = (tmp_info >> 8) & 0x1;
        sis8300_dev_pp->dual_optical_interface = (tmp_info >> 6) & 0x1;
        sis8300_dev_pp->dual_port_14_15_interface = (tmp_info >> 5) & 0x1;
        sis8300_dev_pp->dual_port_12_13_interface = (tmp_info >> 4) & 0x1;
        sis8300_dev_pp->dual_channel_sampling = (tmp_info >> 2) & 0x1;
        sis8300_dev_pp->ringbuffer_delay = (tmp_info >> 1) & 0x1;
        sis8300_dev_pp->trigger_block_enable = tmp_info & 0x1;
        if(sis8300_dev_pp->fpga_1_gb ) sis8300_dev_pp->sis8300_mem_max_size = 1073741824;
/*
        sis8300_pcie_dev->brd_info_list.PCIEDEV_BOARD_ID = (tmp_info >> 16) & 0xFFFF;
        sis8300_pcie_dev->brd_info_list.PCIEDEV_BOARD_VERSION = (tmp_info >> 8)  & 0xFFFF;
*/
        tmp_info = ioread32(address + 0x4);
        smp_rmb();
        sis8300_pcie_dev->brd_info_list.PCIEDEV_HW_VERSION = tmp_info & 0xFFFF;
     }
    return result;
}

static void UPKCOMPAT_EXIT sis8300_remove(struct pci_dev *dev)
{
    int                  result       = 0;
    struct pciedev_dev  *sis8300_pcie_dev;

    sis8300_pcie_dev = pciedev_get_pciedata(dev);
    if( sis8300_pcie_dev != NULL ) {
        sis8300_dev *sis8300_dev_pp;
        int brd_num, slot_num;
        sis8300_dev_pp = pciedev_get_drvdata(sis8300_pcie_dev);
        brd_num = sis8300_pcie_dev->brd_num;
        slot_num = sis8300_pcie_dev->slot_num;
        printk(KERN_ALERT "SIS8300-REMOVE CALLED brd %i slot %i - PRIVATE DATA OK\n", brd_num, slot_num );

        /*now we can call pciedev_remove_exp to clean all standard allocated resources
        will clean all interrupts if it seted 
        */
        result = pciedev_remove2_exp(sis8300_pcie_dev);
        printk(KERN_ALERT "SIS8300-PCIEDEV_REMOVE_EXP CALLED, brd=%i slot %i result=%d\n", brd_num, slot_num, result);

        /* clean up any allocated resources and stuff here */
        kfree(sis8300_dev_pp);
        
    } else {
        printk(KERN_ALERT "SIS8300-REMOVE - PRIVATE DATA NOT FOUND \n");
    }
 
}

/****************************************************************************************/
static struct pci_driver pci_sis8300_driver = {
    .name       = SIS8300DEVNAME,
    .id_table   = sis8300_ids,
    .probe      = sis8300_probe,
    .remove   = UPKCOMPAT_EXIT_P(sis8300_remove),
};

static int sis8300_open( struct inode *inode, struct file *filp )
{
    int    result = 0;
    result = pciedev_open_exp( inode, filp );
    return result;
}

static int sis8300_release(struct inode *inode, struct file *filp)
{
     int result            = 0;
     result = pciedev_release_exp(inode, filp);
     return result;
} 

static ssize_t sis8300_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
     ssize_t    retval         = 0;
    retval  = pciedev_read_exp(filp, buf, count, f_pos);
    return retval;
}

static ssize_t sis8300_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    ssize_t         retval = 0;
    retval = pciedev_write_exp(filp, buf, count, f_pos);
    return retval;
}

static long  sis8300_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    long result = 0;

    if (_IOC_TYPE(cmd) == PCIEDOOCS_IOC) {
        int icode = _IOC_NR(cmd);
        if (        (icode <= PCIEDOOCS_IOC_MAXNR)     && (icode >= PCIEDOOCS_IOC_MINNR) ) {
            result = pciedev_ioctl_exp(filp, &cmd, &arg, sis8300_cdev_m);
        } else if ( (icode <= PCIEDOOCS_IOC_DMA_MAXNR) && (icode >= PCIEDOOCS_IOC_DMA_MINNR) ) {
            result = sis8300_ioctl_dma(filp, &cmd, &arg);
        } else if ( (icode <= SIS8300_IOC_MAXNR)       && (icode >= SIS8300_IOC_MINNR) ) {
            result = sis8300_ioctl_dma(filp, &cmd, &arg);
        } else {
            return -ENOTTY;
        }
    } else {
        return -ENOTTY;
    }
    return result;
}

static void __exit sis8300_cleanup_module(void)
{
    printk(KERN_NOTICE "SIS8300_CLEANUP_MODULE: PCI DRIVER UNREGISTERED\n");
    pci_unregister_driver(&pci_sis8300_driver);
    printk(KERN_NOTICE "SIS8300_CLEANUP_MODULE CALLED\n");
    upciedev_cleanup_module_exp(&sis8300_cdev_m);
    
}

static int __init sis8300_init_module(void)
{
    int   result = 0;
    
    printk(KERN_WARNING "SIS8300_INIT_MODULE CALLED\n");
    result = upciedev_init_module_exp(&sis8300_cdev_m, &sis8300_fops, SIS8300DEVNAME);
    result = pci_register_driver(&pci_sis8300_driver);
    printk(KERN_ALERT "SIS8300_INIT_MODULE:REGISTERING PCI DRIVER RESUALT %d\n", result);
    return 0; /* succeed */
}

module_init(sis8300_init_module);
module_exit(sis8300_cleanup_module);


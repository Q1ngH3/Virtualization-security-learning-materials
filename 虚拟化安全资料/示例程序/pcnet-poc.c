#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <asm/io.h>
MODULE_LICENSE("GPL");

static int hello_init(void)
{
  void* pvirt;
  void* pvirt3;
  void* pphy;
  void* pphy3;
  
  pvirt =kmalloc(0x100,GFP_KERNEL);
  memset(pvirt,1,0x100);
  pphy=virt_to_phys(pvirt);
 
  pvirt3 =kmalloc(0x1100,GFP_KERNEL);
  memset(pvirt3,1,0x1100);
  pphy3=virt_to_phys(pvirt3);
 
  *(int*)pvirt=0x8044;  
  *(int*)pvirt3=pphy3+16;
  *((short*)pvirt3 +2)=-4095;
  *((short*)pvirt3 +3)=0x8300;
  *((int*)pvirt+5)=pphy3;
  *((int*)pvirt+6)=pphy3;//tmd结构的起始地址//#2.2
 
  outl(0,0xc054);
  outl(0x4,0xc050);//stop
         
  outl(1,0xc054);
  outl((int)pphy&0xffff,0xc050);
  outl(2,0xc054);
  outl((int)pphy>>16,0xc050);//写地址
   
  outl(72,0xc054);
  outl(0,0xc050);
  outl(76,0xc054);
  outl(0,0xc050);
  outl(78,0xc054);
  outl(0,0xc050);
  outl(74,0xc054);
  outl(0,0xc050);
   
  outl(0x3a,0xc054);   
  outl(0x1,0xc050);
   
  outl(0,0xc054);
  outl(0,0xc050);
   
  outl(0,0xc054);
  outl(1,0xc050);//重新初始化,进入发包
  outl(0,0xc054);
  outl(8,0xc050);//transmit//#3.1
    
return 0;
}
static void hello_exit(void)
{
printk(KERN_ALERT "goodbye,kernel\n");
}
module_init(hello_init);
module_exit(hello_exit);
MODULE_AUTHOR("edsionte Wu");
MODULE_DESCRIPTION("This is a simple example!\n");
MODULE_ALIAS("A simplest example");
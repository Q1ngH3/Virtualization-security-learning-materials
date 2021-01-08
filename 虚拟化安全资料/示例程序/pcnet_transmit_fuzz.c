#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <asm/io.h>
#define PAGE_OFFSET 0x0C000000
MODULE_LICENSE("GPL");
void get_random_bytes(void* buf,int size);
static int hello_init(void)
{
  unsigned long rand; 
  void* pvirt;
  void*pvirt2;
  void* pphy;
  unsigned long* pdbal;
  unsigned int tmp;
  unsigned short status; 
  int flags;
  flags=1;
  pvirt =kmalloc(0x100,GFP_KERNEL);
  memset(pvirt,1,0x100);
  pphy=virt_to_phys(pvirt);
while(1)
{     get_random_bytes(&rand,sizeof(rand));
      if(rand%5!=0)//决定开启loopback还是填充一个随机数
      {   get_random_bytes(&rand,sizeof(rand));
          *(int*)pvirt=rand;//(int)pphy+16;
      }
      else
      {   get_random_bytes(&rand,sizeof(rand));
          *(int*)pvirt=0x44|rand;  
      }
      get_random_bytes(&rand,sizeof(rand));
      if(rand%3==0)//填充length
      {*( (short*)pvirt+2)=-(rand%2048 +2048);}
      else{*( (short*)pvirt+2)=-(rand%2048);}
      get_random_bytes(&rand,sizeof(rand));
      rand%=20;
      if(rand==0)//填充status
      { get_random_bytes(&rand,sizeof(rand));
        rand&=0xffff;
        rand|=0x8000;
        *((short*)pvirt+3)=rand;
       }
      else
	  { get_random_bytes(&rand,sizeof(rand));
        rand&=0xffff;
        rand|=0x8300;
        *((short*)pvirt+3)=rand;
       }
        
      pvirt2=(int*)pvirt+4;
      get_random_bytes(&rand,sizeof(rand));
      *((int*)pvirt2)=rand;//填充一个随机数做tbadr
      get_random_bytes(&rand,sizeof(rand));
      if(rand%3==0)//填充length
      {*( (short*)pvirt2+2)=-(rand%2048+2048);}
      else{*( (short*)pvirt2+2)=-(rand%2048);}
      get_random_bytes(&rand,sizeof(rand));
      if(rand%2==0)//填充status
      { *((short*)pvirt2+3)=0x8000;}
      else
	  { *((short*)pvirt2+3)=rand&0x7fff;}
      get_random_bytes(&rand,sizeof(rand));
      if(rand%3==0)//填充status
      {  pvirt2=(int*)pvirt+8;
         *((short*)pvirt2+3)&=0x0;
      }
      *((int*)pvirt+6)=pphy;//tmd结构的起始地址
   //*(int*)pvirt=0x0000;
    
      printk(KERN_ALERT "%08x\n",pvirt);
      printk(KERN_ALERT "%08x\n",pphy);
      
	    outl(0,0xc054);
      outl(0x4,0xc050);//stop
      outl(1,0xc054);
      outl((int)pphy&0xffff,0xc050);
      outl(2,0xc054);
      outl((int)pphy>>16,0xc050);//写地址
      
      get_random_bytes(&rand,sizeof(rand));
      if(rand%3!=0)//设置偏移
      { get_random_bytes(&rand,sizeof(rand));
        rand&=0xf;
        outl(78,0xc054);
        outl(-rand,0xc050);
        outl(74,0xc054);
        outl(-rand,0xc050);
      }
      else
      {get_random_bytes(&rand,sizeof(rand));
       rand&=0xf;
       outl(78,0xc054);
       outl(-rand,0xc050);
       get_random_bytes(&rand,sizeof(rand));
       rand&=0xf;
       outl(74,0xc054);
       outl(-rand,0xc050);
      }                    
      get_random_bytes(&rand,sizeof(rand));
      if(rand%3==1)//一定概率进1.9
      { outl(0x3a,0xc054);
        outl(0x1,0xc050);
      }
      else if(rand%3==2)
      { outl(0x3a,0xc054);
        outl(2,0xc050);
       }
      else
      {outl(0x3a,0xc054);
       outl(3,0xc050);
       }
	   
      outl(0,0xc054);
      outl(1,0xc050);//重新初始化,进入发包
      get_random_bytes(&rand,sizeof(rand));
      if(rand%3==0)//一定概率进1.16
      { get_random_bytes(&rand,sizeof(rand));
        outl(3,0xc054);
        outl(rand&0xffbf,0xc050);
       }
      else
	  {  get_random_bytes(&rand,sizeof(rand));
         outl(3,0xc054);
         outl(0x40|rand&0xffff,0xc050);
      } 
     get_random_bytes(&rand,sizeof(rand));
     if(rand%3==0)//一定概率进1.0
     {rand=0x10;}
     else
     {rand=0;}
     outl(0,0xc054);
   //tmp=inl(0xc050);
     
     outl(8|rand,0xc050);//transmit
     mdelay(5); 
 }  
   
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

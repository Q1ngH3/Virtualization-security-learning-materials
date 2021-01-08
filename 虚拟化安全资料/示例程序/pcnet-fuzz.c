#include <asm/io.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/random.h>
MODULE_LICENSE("GPL");

#define IOBASE 0xc040

typedef struct __attribute__ ((packed)){
    uint16_t mode;
    uint8_t rlen;
    uint8_t tlen;
    uint16_t padr[3];
    uint16_t _res;
    uint16_t ladrf[4];
    uint32_t rdra;
    uint32_t tdra;
} initblk32;

typedef struct __attribute__ ((packed)){
    uint32_t tbadr;
    int16_t length;
    int16_t status;
    uint32_t misc;
    uint32_t res;
} pcnet_TMD;

void writeWord(int reg,int value){
  outw(reg,IOBASE+0x12);
  outw(value,IOBASE+0x10);
}

initblk32* blk;
pcnet_TMD *tmd;
void* buffer;
int minit(void){
  blk=kmalloc(sizeof(initblk32),GFP_KERNEL);
  tmd=kmalloc(sizeof(pcnet_TMD)*10,GFP_KERNEL);//假设最多有10个tmd结构体待处理
  buffer=kmalloc(0x1000,GFP_KERNEL);
  int temp;
  int cases[51]={1,2,8,9,10,11,12,13,14,15,18,19,20,21,22,23,24,25,26,
27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,72,74,76,78,112,3,4,5,16,17,58};
  while(1){// 循环触发pcnet_transmit流程
    get_random_bytes(blk,sizeof(initblk32));//构造随机的blk
    memset(tmd,0,0x30);
    blk->tdra=virt_to_phys(tmd);//指向我们构造的tmd数组首地址
    blk->rlen=blk->tlen=(1<<4);// 假设只给两个tmd结构体

    get_random_bytes(&temp,4);
    if(temp&1)//一半的概率发包,一半的概率收包
      writeWord(15,0);// disable loop,发包
  else
      writeWord(15,4);// enable loop,收包

    /*第一次tmd不发包或者收包*/
    tmd->tbadr=virt_to_phys(buffer);// 赋值buffer
    get_random_bytes(buffer,0x1000);//随机的buffer数据
    tmd->length|=(15<<12);// x&0xf000 >>12 != 15,TMDL_ONES_MASK
    get_random_bytes(&temp,4);
    tmd->length|=(temp&0xfff);// TMDL_BCNT_MASK,随机的buffer长度
    tmd->status|=(0x200);// TMDS_STP_MASK
    tmd->status|=0x8000;// !!(CSR_CXST(s) & 0x8000)
    /*第二次tmd开始发包或者收包*/
    (tmd+1)->tbadr=virt_to_phys(buffer);
    get_random_bytes(buffer,0x1000);
    (tmd+1)->length|=(15<<12);// x&0xf000 >>12 != 15,TMDL_ONES_MASK
    get_random_bytes(&temp,4);
    (tmd+1)->length|=(temp&0xfff);// TMDL_BCNT_MASK 0xfff
    (tmd+1)->status|=(0x100);//TMDS_ENP_MASK
    (tmd+1)->status|=0x8000;// !!(CSR_CXST(s) & 0x8000)

    u32 phy=virt_to_phys(blk);
    writeWord(0,4);// pcnet_stop
    writeWord(1,phy&0xffff);
    writeWord(2,phy>>16);
    /* enable BCR_SSIZE32*/
    outw(20,IOBASE+0x12);
    outw(1,IOBASE+0x16);
    get_random_bytes(&temp,4);
    temp&=0xf;
    while(temp--){
      int reg,val;
      get_random_bytes(&reg,4);
      get_random_bytes(&val,4);
      writeWord(reg % 51 , val);
    }
    writeWord(0,1);// pcnet_init blk
    writeWord(0,8);// pcnet_transmit
  }
  printk("Module loaded!\n");
  return 0;
}

void mexit(void){
  kfree(blk);
  kfree(tmd);
  printk("Module unloaded!\n");
}

module_init(minit);
module_exit(mexit);
//csr[60][61] 为[34][35]的备份
//csr[62][63] 为CSR_CXBC(s); CSR_CXST(s);备份
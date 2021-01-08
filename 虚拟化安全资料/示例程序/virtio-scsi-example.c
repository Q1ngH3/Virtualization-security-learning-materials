#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/virtio.h>

#define SIZE 8
#define VIRTIO_SCSI_IO  0xc040

typedef struct cmd_req
{
  u8 lun[8];
  u64 tag;
  u8 task_attr;
  u8 prio;
  u8 crn;
  u8 cdb[8];
}cmdq;

typedef struct tmf_req
{
  u32 type;
  u32 subtype;
  u8 lun[8];
  u64 tag;
}tmfq;

typedef struct an_req
{
  u32 type;
  u8 lun[8];
  u32 event_requested;
}anq;

typedef union {
  cmdq cmd;
  tmfq tmf;
  anq an;
}req;

typedef struct VRingDesc
{
  u64 addr;
  u32 len;
  u16 flags;
  u16 next;
}VRingDesc;

typedef struct VRingAvail
{
  u16 flags;
  u16 idx;
  u16 ring[];
}VRingAvail;

typedef struct VRingUsedElem
{
  u32 id;
  u32 len;
}VRingUsedElem;

typedef struct VRingUsed
{
  u16 flags;
  u16 idx;
  VRingUsedElem ring[];
}VRingUsed;
MODULE_LICENSE("GPL");


void queue_pfn(u32 value)
{
  outl(value,VIRTIO_SCSI_IO + 8);
}

void queue_sel(u16 value)
{
  outw(value,VIRTIO_SCSI_IO + 14);
}

void queue_notify(u32 value)
{
  outl(value,VIRTIO_SCSI_IO + 16);
}

int handle_cmd(void)
{
  VRingDesc *desc1;
  req * buffer;
  VRingAvail *avail;
  VRingUsed *used1;
  unsigned long mem
  mem = kmalloc(0x3000,GFP_KERNEL);//align
  memset(mem,0,0x3000);
  desc1 = mem;
  //因为设备默认最大有0x80个描述符表,一个描述符的大小为0x10
  //qemu实现中把avail表接在了描述符表之后,因此avail表=desc+0x80*0x10;`
  avail = mem + 0x800;
  // 而一个avail结构体为0x2*0x80+4=>0x104,而qemu做了一个4k对齐操作,因此变成了+0x1000
  used1 = mem + 0x1000;

  desc1[0].addr = (u64)virt_to_phys(buffer);
  desc1[0].len = (u32)0x33;// buffer的大小
  desc1[0].flags = (u16)0x2;// VRING_DESC_F_WRITE,因为没有VRING_DESC_F_NEXT标志,表示没有下一个描述符
  desc1[0].next = (u16)0x2;//这个字段无效了

  // buffer为scsi定义的结构体,详见virtio-scsi.h的99行
  buffer = kmalloc(sizeof(req) * SIZE,GFP_KERNEL);
  buffer->cmd.cdb[0] = 0x28;
  buffer->cmd.lun[0] = 0x0;//0x1
  buffer->cmd.lun[1] = 0x0;
  buffer->cmd.lun[2] = 0x0;//0x40
  //初始一个avail表
  avail->idx = 0;
  avail->ring[0] = 0x0;
  
  queue_sel(2);// 设定命令类型为2,代表 virtio_scsi_handle_cmd
  queue_pfn(mem>>12);// 设定描述符表
  queue_notify(2);// 触发virtio_scsi_handle_cmd函数.

  kfree(buffer);
  kfree(desc1);
  return 0;
}

int moduleInit(void)
{
  printk(KERN_ALERT"[+]Start!\n");
  handle_cmd();
}

int moduleExit(void)
{
  printk(KERN_ALERT"[+]Exit!\n");
  return 0;
}

module_init(moduleInit);
module_exit(moduleExit);
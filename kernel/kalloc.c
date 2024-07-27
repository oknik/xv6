// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem[NCPU];// 每个CPU需要有一个空闲内存页链表以及相应的锁

void
kinit()// 初始化kmem[]的锁并调用freearrange()初始化分配物理页
{
  char lockname[8];
  for(int i = 0;i < NCPU; i++) {
    snprintf(lockname, sizeof(lockname), "kmem_%d", i);// 设置锁名称，存储到lockname
    initlock(&kmem[i].lock, lockname);
  }
  freerange(end, (void*)PHYSTOP);// 分配物理页
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  push_off();  // 关闭中断并保存当前中断状态
  int id = cpuid();
  acquire(&kmem[id].lock);
  r->next = kmem[id].freelist;
  kmem[id].freelist = r;
  release(&kmem[id].lock);
  pop_off();  // 开中断并恢复之前保存的中断状态
}


// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  push_off();// 关中断
  int id = cpuid();
  acquire(&kmem[id].lock);// 获取当前CPU的空闲列表的锁
  r = kmem[id].freelist;// 获取当前CPU空闲列表的头部节点
  if(r)// 如果当前CPU的空闲列表不为空，取出一个空闲页面
    kmem[id].freelist = r->next;
  else {// 如果当前CPU没有空闲物理页，“偷取”
    int antid;// 另一个CPU的ID
    for(antid = 0; antid < NCPU; ++antid) {// 遍历所有CPU的空闲列表
      if(antid == id)// 跳过当前CPU
        continue;
      acquire(&kmem[antid].lock);
      r = kmem[antid].freelist;// 获取其他CPU空闲列表的头部节点
      if(r) {// 如果找到一个不为空的空闲列表，取出一个空闲页面
        kmem[antid].freelist = r->next;// 更新该空闲列表头部节点为下一个节点
        release(&kmem[antid].lock);
        break;
      }
      release(&kmem[antid].lock);
    }
  }
  release(&kmem[id].lock);
  pop_off();  //开中断

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}

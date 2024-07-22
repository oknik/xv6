#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "date.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;


  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}


#ifdef LAB_PGTBL
int
sys_pgaccess(void)
{
  // lab pgtbl: your code here.
  uint64 address;
  if(argaddr(0,&address)<0)// 获取基地址
	  return -1;
  
  int len;
  if(argint(1, &len) < 0 || len > 32)// 获取页数量，最大32
	  return -1;

  uint64 mask_address;
  if(argaddr(2, &mask_address) < 0)// 获取掩码地址
	  return -1;

  struct proc* proc = myproc();
  uint32 mask = 0;// 掩码缓冲区
  for(int i=0;i<len;++i){
  	// walk函数在页表中找到给定虚拟地址的页表项
  	pte_t* pte = walk(proc->pagetable, address + i * PGSIZE, 0);
	if(*pte & PTE_A){// 被访问过
		mask |= 1 << i;// 标记
		*pte &= ~PTE_A;// 清除PTE_A
	}
  }
  if(copyout(proc->pagetable, mask_address, (char*)&mask, 4) < 0) {// 拷贝到用户空间	  
	  return -1;
  }
  return 0;
}
#endif

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

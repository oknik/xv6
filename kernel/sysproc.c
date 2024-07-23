#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
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

  backtrace();

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

// 用户系统调用，用于恢复中断上下文并返回到中断代码
uint64
sys_sigreturn(void)
{
  struct proc* proc = myproc();

  // 恢复之前保存的陷阱帧，以便返回到中断代码
  *proc->trapframe = proc->saved_trapframe;
  proc->have_return = 1; // 设置返回标志为 true
  return proc->trapframe->a0; // 返回陷阱帧中的返回值
}

// 用户系统调用，用于设置定时器中断和信号处理函数
uint64
sys_sigalarm(void)
{
  int ticks;          // 定时器滴答数
  uint64 handler_va;  // 信号处理函数的虚拟地址
  argint(0, &ticks);          // 获取定时器滴答数参数
  argaddr(1, &handler_va);    // 获取信号处理函数的虚拟地址参数

  struct proc* proc = myproc();
  // 设置进程的定时器间隔和信号处理函数地址
  proc->alarm_interval = ticks;
  proc->handler_va = handler_va;

  proc->have_return = 1; // 设置返回标志为 true
  return 0; 
}








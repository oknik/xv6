#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char const *argv[]){
	int pid;//进程编号
	int p1[2], p2[2];//p1父->子 p2子->父 [0]读 [1]写
	pipe(p1);//创建管道
	pipe(p2);

	if (fork() == 0){ //创建子进程
		pid = getpid();
		char buf[2];
		close(p1[1]);
        close(p2[0]);
		if (read(p1[0], buf, 1) != 1){//从p1读取一个字节到buf
        //read返回1代表成功读取一个字节，不为1则读取失败
			fprintf(2, "failed to read in child\n");
			exit(1);
		}
		close(p1[0]);
		printf("%d: received ping\n", pid);//读取成功
		if(write(p2[1], buf, 1) != 1){//将读到的数据写到p2
        //write返回1代表成功写入一个字节，不为1则写入失败
			fprintf(2, "failed to write in child\n");
			exit(1);
		}
		close(p2[1]);
		exit(0);
		}
		else{//创建父进程
			pid = getpid();
			char info[2] = "a";
			char buf[2];
			buf[1] = 0;
			close(p1[0]);
        	close(p2[1]);
			if (write(p1[1], info, 1) != 1){//将一个字符"a"写到p1
				fprintf(2, "failed to write in parent\n");
				exit(1);
			}
		close(p1[1]);
        //等待子进程接收
		wait(0);
		if(read(p2[0], buf, 1) != 1){//从p2读取一个字节到buf
			fprintf(2, "failed to read in parent\n");
			exit(1);
		}
		printf("%d: received pong\n", pid);//读取成功
		close(p2[0]);
		exit(0);
	}
}

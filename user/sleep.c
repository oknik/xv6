#include "kernel/types.h"//包含了各种基本数据类型的定义
#include "kernel/stat.h"//定义了用于文件状态信息的结构体，例如 struct stat。这个结构体包含了文件的元数据，比如大小、权限、创建时间等。
#include "user/user.h"//包含了用户空间的库函数和系统调用的原型定义。例如，fork, exec, wait, sleep, kill等。

int main(int argc, char *argv[]) {//argc命令行参数的数量 argv[]命令行参数的数组
	int ticks;
	if(argc != 2) {//数量应该是2 程序名+时间参数
		fprintf(2, "Usage: sleep number\n");
		exit(1);
	}
	ticks = atoi(argv[1]);//参数转换 将argv[1]转换为int型
	if(ticks == 0 && strcmp(argv[1],"0") != 0){//有效性检查 确保输入的时间参数有效 
    //ticks被转换为0切原字符串不为0时输入是无效的
		fprintf(2, "sleep: invalid time interval '%s'\n", argv[1]);
		exit(2);
	}
	sleep(ticks);
	exit(0);
}

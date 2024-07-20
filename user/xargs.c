#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[]){
    int i, j = 0, k, l, m = 0;
    char block[32], buf[32];//block[]按行读取 buf[]每行中的字符
    char *p = buf, *lineSplit[32];//p存buf中的字符添加到lineSplit lineSplit存命令用于后续执行
    for(i = 1; i < argc; i++){//将命令行参数存到lineSplit中
    //这里i从1开始因为0是程序名称
        lineSplit[j++] = argv[i];
    }
    while( (k = read(0, block, sizeof(block))) > 0){//每次读一行处理
        for(l = 0; l < k; l++){
            if(block[l] == '\n'){//一行结束
                buf[m] = 0;//先完成跟==" "一样的操作
                lineSplit[j++] = p;
                lineSplit[j] = 0;//再多加一部给lineSplit加上尾零

                m = 0;//缓冲区索引重置为0，下一行要从头开始输入buf         
                p = buf;//回到缓冲区其实位置，下一行要从头开始输入lineSplit
                j = argc - 1;//重置j

                if(fork() == 0){//创建子进程执行lineSplit中的命令
                    exec(argv[1], lineSplit);
                }                
                wait(0);//等子进程执行完
            }
            else if(block[l] == ' ') {//一个命令参数结束
                buf[m++] = 0;//加上尾零
                lineSplit[j++] = p;//把新读到的字符加到lineSplit里
                p = &buf[m];//下一个参数开始的地方
            }
            else {//正常读取到buf
                buf[m++] = block[l];
            }
        }
    }
    exit(0);
}
/*eg: echo -e "Hello World\nThis is a test\nAnother line" | ./myexec echo
输入内容为三行，当第一行读完时
buf = "Hello\0World\0"
lineSplit = ["echo", "Hello", "World", NULL, ..., NULL]读完就去执行了
第二行读完时
buf = "This\0is\0a\0test\0"
lineSplit = ["echo", "This", "is", "a", "test", NULL, ..., NULL]*/

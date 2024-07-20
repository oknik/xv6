#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void func(int *input, int num) {
    if (num == 1) {//只剩最后一个数字了，说明它无法被所有比它小的数字整除，是素数
        printf("prime %d\n", *input);
        return;
    }

    int p[2];
    int prime = *input;//将input[0]作为当前素数prime
    int temp;
    printf("prime %d\n", prime);//打印prime 这里上来就打印跟输入的第一个数字是2有关系
    pipe(p);

    if (fork() == 0) {//子进程 将不能被prime整除的数送入p
        close(p[0]);
        for (int i = 1; i < num; i++) {
            temp = *(input + i);
            if (temp % prime != 0) {
                write(p[1], (char *)(&temp), 4);//写
            }
        }
        close(p[1]);
        exit(0);
    } 
    else {//父进程
        close(p[1]);
        int counter = 0;//计数器
        char buffer[4];//每次读4个字节(int)到buffer
        while (read(p[0], buffer, 4) != 0) {//从p中读数据存为新的input
            temp = *((int *)buffer);//把buffer转换成int型temp
            input[counter++] = temp;
        }
        close(p[0]);
        wait(0); //等待子进程结束
        if (counter > 0) {
            func(input, counter); //递归处理剩余的数
        }
    }
}

int main() {
    int input[34];
    for (int i = 0; i < 34; i++) {
        input[i] = i + 2;
    }
    func(input, 34);
    exit(0);
}

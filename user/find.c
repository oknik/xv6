#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"//包括定义文件系统中的数据结构和操作系统内核对文件系统的接口

void find(char *path, char *filename){
    char buf[512], *p;//buf存储当前路径 p指向路径中最后一个文件/目录名的位置
    int fd;//文件描述符
    struct dirent de;//目录项结构，表示目录中每个文件
    struct stat st;//文件状态结构，存储文件的元数据

    if ((fd = open(path, 0)) < 0){//打开目录path
    //小于0说明打开失败
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }

    if (fstat(fd, &st) < 0){//获取路径状态信息
    //小于0说明获取失败
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
    return;
    }

    if (st.type == T_FILE){
    //如果是一个文件说明路径错误
        fprintf(2, "find: can't find files in a file\n");
        exit(1);
    }

    if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
    //路径长度超过缓冲区大小 路径长度+分隔符+最长文件名+尾零
        printf("find: path too long\n");
        exit(1);
    }

    //以上检查都通过，路径正确
    strcpy(buf, path);//路径拷贝到缓冲区buf
    p = buf + strlen(buf);//p指向最后路径中最后一个文件/目录名的位置
    *p++ = '/';
    
    //读目录项
    while (read(fd, &de, sizeof(de)) == sizeof(de)){
    //read(fd, &de, sizeof(de)):每次读sizeof(de)个字节，将读到的目录项fd都存到de中
    //read会返回读取的字节数，跟sizeof(de)相等说明读取成功
        if (de.inum == 0){//如果inum为0说明目录项无效或空闲，跳过
            continue;
        }
        memmove(p, de.name, DIRSIZ);//将目录项的名字复制到p指向的位置
        p[DIRSIZ] = 0;//尾零
        if (stat(buf, &st) < 0){//将目录项信息存到st中
        //小于零说明失败，处理下一个目录项
            fprintf(2, "find: cannot stat %s\n", buf);
            continue;
        }
        if (st.type == T_DIR && strcmp(".", p) != 0 && strcmp("..", p) != 0){//检查目录项类型
        //如果是一个目录，且不是当前目录即"."，也不是父目录即".."则递归调用find进入目录查找
            find(buf, filename);
        }
        else if (st.type == T_FILE && strcmp(p, filename) == 0){
        //如果是一个文件就比较一下
            printf("%s\n", buf);
        }
    }

    close(fd);
    }

int main(int argc, char *argv[]){
    if (argc < 2){//要读两个参数：传递搜索路径和文件名
        fprintf(2, "too little arguments\n");
        exit(1);
    }
    find(argv[1], argv[2]);
    exit(0);
}

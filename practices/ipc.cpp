#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>

#define FIFO1 "t_fifo_1"
#define FIFO2 "t_fifo_2"

void createPipe(int fd[2])
{
    int ret = 0;
    if ((ret = pipe(fd)) != 0)
    // if ((ret = socketpair(AF_UNIX, SOCK_STREAM, 0, fd)))
    {
        printf("pipe failed! ret = %d\n", ret);
        exit(-1);
    }
    printf("create pipe success! read = %d write = %d\n", fd[0], fd[1]);
}

void serverPipe(int read_fd, int write_fd)
{
    printf("init server pipe read = %d, write = %d\n", read_fd, write_fd);
    // fcntl(read_fd, F_SETFL, O_NONBLOCK);
    int n = 0;
    char buff[256] = {0};
    if ((n = read(read_fd, buff, 256)) == 0)
    {
        printf("child read 0\n");
        return;
    }
    printf("server read %s\n", buff);
    
    int fd = -1;
    if ((fd = open(buff, O_RDONLY)) < 0)
    {
        snprintf(buff + n, 256, " can't open\n");
        write(write_fd, buff, strlen(buff));
        return;
    }

    while ((n = read(fd, buff, 256)) > 0)
        write(write_fd, buff, n);
    close(fd);
    printf("close\n");
}

void clientPipe(int read_fd, int write_fd)
{
    printf("init client pipe read = %d, write = %d\n", read_fd, write_fd);
    char buff[256];
    fgets(buff, 256, stdin);
    int len = strlen(buff);
    if (buff[len-1] == '\n')
        len--;
    printf("client input %s", buff);
    write(write_fd, buff, len);
    if (errno != 0)
    {
        printf("client write failed! ret = %d, err = %s\n", errno, strerror(errno));
        return;
    }

    int n = 0;
    while ((n = read(read_fd, buff, 256)) > 0)
        write(STDOUT_FILENO, buff, n);
    // sleep(10);
}

void testPipe()
{
    // int fd1[2], fd2[2];
    // createPipe(fd1);
    // createPipe(fd2);
    if ((mkfifo(FIFO1, 0600) < 0) && errno != EEXIST)
    {
        printf("can't create %s", FIFO1);
        return;
    }
    if ((mkfifo(FIFO2, 0600) < 0) && errno != EEXIST)
    {
        printf("can't create %s", FIFO2);
        return;
    }

    pid_t child_t = 0;
    if ((child_t = fork()) == 0)
    {
        // close(fd1[1]);
        // close(fd2[0]);
        // int read_fd = fd1[0];
        // int write_fd = fd2[1];
        int read_fd = open(FIFO1, O_RDONLY, 0);
        int write_fd = open(FIFO2, O_WRONLY, 0);
        serverPipe(read_fd, write_fd);
        printf("child proccess llllllllllllllllllllllllllllllllllllexit!\n");
        exit(0);
    }
    printf("child proccess %d\n", child_t);

    // close(fd1[0]);
    // close(fd2[1]);
    // int read_fd = fd2[0];
    // int write_fd = fd1[1];
    int write_fd = open(FIFO1, O_WRONLY, 0);
    int read_fd = open(FIFO2, O_RDONLY, 0);
    clientPipe(read_fd,write_fd);
    waitpid(child_t, NULL, 0);
    unlink(FIFO1);
    unlink(FIFO2);
    printf("main process exit!\n");
}

int main()
{
    testPipe();
    return 0;
}
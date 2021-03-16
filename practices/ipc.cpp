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
#include <signal.h>
#include <pthread.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <string>

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
    // sleep(10);
    // close(read_fd);
    // close(write_fd);
    // printf("serverPipe close %d\n", write_fd);
    // sleep(10);
    // fcntl(read_fd, F_SETFL, O_NONBLOCK);
    char buff[256] = {0};
    int n = read(read_fd, buff, 256);
    if (n < 0)
    {
        printf("child read failed! ret = %d, err = %s\n", errno, strerror(errno));
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
    {
        n = write(write_fd, buff, n);
        if (n < 0)
        {
            printf("child write failed! ret = %d, err = %s\n", errno, strerror(errno));
            break;
        }
    }
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
    int n = write(write_fd, buff, len);
    if (n < 0)
    {
        printf("client write failed! ret = %d, err = %s\n", errno, strerror(errno));
        return;
    }
    printf("client write success!\n");

    n = 0;
    while ((n = read(read_fd, buff, 256)) > 0)
        write(STDOUT_FILENO, buff, n);

    printf("clientPipe return\n");
}

void testPipe()
{
    int fd1[2], fd2[2];
    createPipe(fd1);
    createPipe(fd2);
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
        close(fd1[1]);
        close(fd2[0]);
        int read_fd = fd1[0];
        int write_fd = fd2[1];
        // int read_fd = open(FIFO1, O_RDONLY, 0);
        // int write_fd = open(FIFO2, O_WRONLY, 0);
        serverPipe(read_fd, write_fd);
        printf("child proccess llllllllllllllllllllllllllllllllllllexit!\n");
        exit(0);
    }
    printf("child proccess %d\n", child_t);

    close(fd1[0]);
    close(fd2[1]);
    int read_fd = fd2[0];
    int write_fd = fd1[1];
    // int write_fd = open(FIFO1, O_WRONLY, 0);
    // int read_fd = open(FIFO2, O_RDONLY, 0);
    clientPipe(read_fd,write_fd);
    waitpid(child_t, NULL, 0);
    unlink(FIFO1);
    unlink(FIFO2);
    printf("main process exit!\n");
}

class RWLock
{
public:
    RWLock()
        :wait_readers(0), wait_writters(0), ref_count(0)
    {
        pthread_mutex_init(&mutex_, NULL);
        pthread_cond_init(&read_cond, NULL);
        pthread_cond_init(&write_cond, NULL);
    }

    ~RWLock()
    {
        pthread_cond_destroy(&read_cond);
        pthread_cond_destroy(&write_cond);
        pthread_mutex_destroy(&mutex_);
    }

    void RLock()
    {
        pthread_mutex_lock(&mutex_);
        while (ref_count < 0 || wait_writters > 0)
        {
            wait_readers++;
            pthread_cond_wait(&read_cond, &mutex_);
            wait_readers--;
        }
        ref_count++;
        pthread_mutex_unlock(&mutex_);
    }

    void UnRLock()
    {
        pthread_mutex_lock(&mutex_);
        ref_count--;
        if (0 == ref_count)
            pthread_cond_signal(&write_cond);
        pthread_mutex_unlock(&mutex_);
    }

    void Lock()
    {
        pthread_mutex_lock(&mutex_);
        while (ref_count != 0)
        {
            wait_writters++;
            pthread_cond_wait(&write_cond, &mutex_);
            wait_writters--;
        }
        ref_count = -1;
        pthread_mutex_unlock(&mutex_);
    }

    void UnLock()
    {
        pthread_mutex_lock(&mutex_);
        if (ref_count > 0)
            ref_count--;
        else if (ref_count < 0)
            ref_count = 0;

        if (0 == ref_count)
        {
            if (wait_writters > 0)
                pthread_cond_signal(&write_cond);
            else if (wait_readers > 0)
                pthread_cond_broadcast(&read_cond);
        }
        pthread_mutex_unlock(&mutex_);
    }
private:
    pthread_mutex_t mutex_;
    pthread_cond_t read_cond;
    pthread_cond_t write_cond;
    int wait_readers;
    int wait_writters;
    int ref_count;
};

void testRWLock()
{
    //TODO
}

struct shared
{
    sem_t mutex;
    int count;

    shared() : count(0)
    {}
};

void testShareMemeory()
{
    int fd = open("mp.txt", O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
    if (-1 == fd)
    {
        printf("open file failed! %s\n", strerror(errno));
        return;
    }

    // shared sh;
    // write(fd, &sh, sizeof(shared));
    shared* ptr = (shared*)mmap(NULL, sizeof(shared), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);

    sem_init(&ptr->mutex, 1, 1);

    if (fork() == 0)
    {
        for (int i = 0; i < 100; ++i)
        {
            sem_wait(&ptr->mutex);
            printf("child: %d\n", ptr->count++);
            sem_post(&ptr->mutex);
        }
        exit(0);
    }

    for (int i = 0; i < 100; ++i)
    {
        sem_wait(&ptr->mutex);
        printf("parent: %d\n", ptr->count++);
        sem_post(&ptr->mutex);
    }
    exit(0);
}

class Sem
{
public:
    Sem(const std::string _n)
        :name(_n)
    {
        sem_fd[0] = sem_fd[1] = -1;
    }

    void init()
    {
        int ret = mkfifo(name.c_str(), 0600);
        if (ret < 0)
        {
            printf("mkfifo failed! name:%s err:%s\n", name.c_str(), strerror(errno));
            return;
        }

        sem_fd[0] = open(name.c_str(), O_RDONLY | O_NONBLOCK, 0);
        sem_fd[1] = open(name.c_str(), O_WRONLY | O_NONBLOCK, 0);

        int flag = fcntl(sem_fd[0], F_GETFL, 0);
        flag &= ~O_NONBLOCK;
        fcntl(sem_fd[0], F_SETFL, flag);

        flag = fcntl(sem_fd[1], F_GETFL, 0);
        flag &= ~O_NONBLOCK;
        fcntl(sem_fd[1], F_SETFL, flag);
    }

    void wait()
    {
        char c;
        int ret = read(sem_fd[0], &c, 1);
        if (ret < 0)
        {
            printf("read failed! err:%s\n", strerror(errno));
            return;
        }
    }

    void post()
    {
        char c = 1;
        int ret = write(sem_fd[1], &c, 1);
        if (ret < 0)
        {
            printf("write failed! err:%s\n", strerror(errno));
            return;
        }
    }

private:
    int sem_fd[2];
    std::string name;
};

void testSem()
{
    Sem sem("t_sem_fifo");
    sem.init();
    if (0 == fork())
    {
        sem.wait();
        printf("child wait\n");
        exit(0);
    }
    sleep(10);
    sem.post();
    printf("parent post\n");
}

int main()
{
    signal(SIGPIPE, SIG_IGN);
    // testPipe();
    // testRWLock();
    // testShareMemeory();
    testSem();
    return 0;
}
#include <unistd.h>
#include <getopt.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>

int lock_num = 0;
pid_t pid;

void lock_file(char* lck_name, char write_buf[10]) {
    int lck_fd = -1;
    while (lck_fd == -1) {
        lck_fd = open(lck_name, O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
    }

    ssize_t buf_size = strlen(write_buf);
    if (write(lck_fd, write_buf, buf_size) != buf_size) {
        perror("File write error");
        exit(-1);
    }

    if (close(lck_fd) == -1) {
        perror("File close error");
        exit(-1);
    }
}

void unlock_file(char* lck_name, int w_buf_size) {
    char lck_r_buf[10] = {0};
    int lck_fd = open(lck_name, O_RDONLY, 0640);
    lseek(lck_fd, 0, SEEK_SET);
    int n = read(lck_fd, lck_r_buf, 10);

    if (n == -1) {
        perror("File read error");
        exit(-1);
    }

    int read_pid = atoi(lck_r_buf);

    if (read_pid != pid) {
        perror("Bad lock pid");
        exit(-1);
    }

    if (close(lck_fd) == -1) {
        perror("File close error");
        exit(-1);
    }

    int rm_ex = remove(lck_name);

    if (rm_ex != 0) {
        perror("File remove error");
        exit(-1);
    }
}

void sig_func(int sig) {
    char out_buf[1024] = {0};
    sprintf(out_buf, "pid: %d Succseed locks: %d\n", pid, lock_num);
    int stat_fd = open("stat.txt", O_WRONLY | O_APPEND | O_CREAT, 0640);
    int size = strlen(out_buf);
    write(stat_fd, out_buf, size);
    exit(0);
}

int main(int argc, char *argv[]) {
    signal(SIGINT, sig_func);
    char lck_suf[] = ".lck";

    if (argv[1] == NULL) {
        perror("You must enter file to lock");
        exit(-1);
    }

    char* to_lock = argv[1];
    char* lck_name = strcat(to_lock, lck_suf);
    pid = getpid();

    while (1) {
        char write_buf[10];
        sprintf(write_buf, "%d", pid);
        int w_buf_size = strlen(write_buf);
        lock_file(lck_name, write_buf);
        sleep(1);
        unlock_file(lck_name, w_buf_size);
        lock_num++;
    }

    return 0;
}
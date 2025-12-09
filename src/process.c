#include "../include/process_manager.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

static Process process_table[MAX_PROCESSES];
static int next_pid = 1;

// 初始化进程表
void init_process_table(void) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        process_table[i].pid = 0;
        process_table[i].system_pid = 0;
        process_table[i].status = 0;
        strcpy(process_table[i].name, "");
    }
}

// 读取二进制文件到内存（用于从Disk Image中提取）
unsigned char* read_file(const char *path, size_t *out_size) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    *out_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    unsigned char *buffer = malloc(*out_size);
    if (!buffer) {
        fclose(f);
        return NULL;
    }

    fread(buffer, 1, *out_size, f);
    fclose(f);
    return buffer;
}

// 将内存中的二进制文件写入临时文件并设为可执行
int write_file(const char *path, unsigned char *data, size_t size) {
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) return -1;

    write(fd, data, size);
    close(fd);

    // 设为可执行
    chmod(path, 0755);
    return 0;
}

// 运行提取出的程序
void run_program(const char *path) {
    pid_t pid = fork();
    if (pid == 0) {
        // 子进程：运行程序
        execl(path, path, (char *)NULL);
        perror("execl failed");
        _exit(1);
    } else if (pid > 0) {
        // 父进程：记录PID
        printf("[INFO] Process started with system PID: %d\n", pid);
    } else {
        perror("fork failed");
    }
}

// 创建新进程（run命令）
int create_process(const char *program_name, const char *program_path) {
    // 1. 查找空闲槽位
    int slot = -1;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].status == 0) {
            slot = i;
            break;
        }
    }

    if (slot == -1) {
        printf("[ERROR] Process table full (max %d processes)\n", MAX_PROCESSES);
        return -1;
    }

    // 2. 读取二进制文件
    size_t size;
    unsigned char *data = read_file(program_path, &size);
    if (!data) {
        printf("[ERROR] Could not read program: %s\n", program_path);
        return -1;
    }

    // 3. 写入临时文件
    char temp_path[] = "/tmp/neumini_XXXXXX";
    int fd = mkstemp(temp_path);
    if (fd == -1) {
        free(data);
        return -1;
    }
    close(fd);

    if (write_file(temp_path, data, size) != 0) {
        free(data);
        return -1;
    }

    free(data);

    // 4. 使用fork和execl运行
    pid_t system_pid = fork();
    if (system_pid == 0) {
        // 子进程
        execl(temp_path, program_name, (char *)NULL);
        perror("[ERROR] execl failed");
        _exit(1);
    } else if (system_pid > 0) {
        // 父进程：记录进程信息
        process_table[slot].pid = next_pid++;
        process_table[slot].system_pid = system_pid;
        strcpy(process_table[slot].name, program_name);
        process_table[slot].status = 1;

        printf("[OK] Process %d started (NeuMiniOS PID: %d, System PID: %d)\n",
               process_table[slot].pid, process_table[slot].pid, system_pid);

        return process_table[slot].pid;
    } else {
        perror("[ERROR] fork failed");
        return -1;
    }
}

// 停止进程（stop命令）
int stop_process(int pid) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].pid == pid && process_table[i].status == 1) {
            if (kill(process_table[i].system_pid, SIGTERM) == 0) {
                waitpid(process_table[i].system_pid, NULL, 0);
                process_table[i].status = 0;
                printf("[OK] Process %d stopped\n", pid);
                return 0;
            } else {
                perror("[ERROR] kill failed");
                return -1;
            }
        }
    }

    printf("[ERROR] Process %d not found\n", pid);
    return -1;
}

// 列出所有进程（plist命令）
void list_processes(void) {
    printf("=== Running Processes (max %d) ===\n", MAX_PROCESSES);
    printf("%-10s %-10s %-20s %s\n", "PID", "System PID", "Name", "Status");
    printf("------------------------------------------------\n");
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].status == 1) {
            printf("%-10d %-10d %-20s %s\n",
                   process_table[i].pid,
                   (int)process_table[i].system_pid,
                   process_table[i].name,
                   "Running");
        }
    }
}

// 清理进程表（退出时调用）
void cleanup_process_table(void) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].status == 1) {
            kill(process_table[i].system_pid, SIGKILL);
            waitpid(process_table[i].system_pid, NULL, 0);
        }
    }
    printf("[INFO] All processes cleaned up\n");
}

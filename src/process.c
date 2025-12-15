#include "../include/process_manager.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

static Process* process_list = NULL;
static int process_count = 0;
static int next_pid = 1;

// 在链表中查找指定 PID 的进程，返回节点指针，可选返回前驱指针
static Process* find_process(int pid, Process** out_prev) {
    Process* prev = NULL;
    Process* curr = process_list;
    while (curr) {
        if (curr->pid == pid) {
            if (out_prev) *out_prev = prev;
            return curr;
        }
        prev = curr;
        curr = curr->next;
    }
    if (out_prev) *out_prev = NULL;
    return NULL;
}

// 初始化进程表
void init_process_table(void) {
    // 启动时确保链表为空
    Process* curr = process_list;
    while (curr) {
        Process* next = curr->next;
        free(curr);
        curr = next;
    }
    process_list = NULL;
    process_count = 0;
    next_pid = 1;
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
    // 1. 检查容量
    if (process_count >= MAX_PROCESSES) {
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
        Process* node = (Process*)malloc(sizeof(Process));
        if (!node) {
            perror("[ERROR] malloc failed");
            return -1;
        }
        node->pid = next_pid++;
        node->system_pid = system_pid;
        strcpy(node->name, program_name);
        node->status = 1;
        node->next = NULL;

        // 追加到链表尾部，保持插入顺序
        if (!process_list) {
            process_list = node;
        } else {
            Process* tail = process_list;
            while (tail->next) {
                tail = tail->next;
            }
            tail->next = node;
        }
        process_count++;

        printf("[OK] Process %d started (NeuMiniOS PID: %d, System PID: %d)\n",
               node->pid, node->pid, system_pid);

        return node->pid;
    } else {
        perror("[ERROR] fork failed");
        return -1;
    }
}

// 淇：停止进程（stop命令）
int stop_process(int pid) {
    if (pid <= 0) {
        printf("Error: Invalid process ID: %d\n", pid);
        return -1;
    }
    
    Process* prev = NULL;
    Process* target = find_process(pid, &prev);
    if (target && target->status == 1) {
        // 检查进程是否仍然存在
        if (kill(target->system_pid, 0) != 0) {
            // 进程已经不存在，移除节点
            if (prev) prev->next = target->next; else process_list = target->next;
            printf("Warning: Process %d (system PID %d) is no longer running\n", 
                   pid, (int)target->system_pid);
            free(target);
            process_count--;
            return 0;
        }
        
        // 发送终止信号
        if (kill(target->system_pid, SIGTERM) == 0) {
            // 等待进程结束
            int status;
            waitpid(target->system_pid, &status, 0);

            // 从链表移除并释放
            if (prev) prev->next = target->next; else process_list = target->next;
            printf("Process %d (%s) stopped successfully\n", pid, target->name);
            free(target);
            process_count--;
            return 0;
        } else {
            perror("Error: Failed to stop process");
            return -1;
        }
    }
    
    printf("Error: Process %d not found or not running\n", pid);
    printf("Use 'plist' to see running processes\n");
    return -1;
}

// 淇：列出所有进程（plist命令）
void list_processes(void) {
    int running_count = 0;
    
    printf("=== Running Processes (max %d) ===\n", MAX_PROCESSES);
    printf("%-10s %-10s %-20s %s\n", "PID", "System PID", "Name", "Status");
    printf("------------------------------------------------\n");
    
    for (Process* curr = process_list; curr; curr = curr->next) {
        if (curr->status == 1) {
            printf("%-10d %-10d %-20s %s\n",
                   curr->pid,
                   (int)curr->system_pid,
                   curr->name,
                   "Running");
            running_count++;
        }
    }
    
    if (running_count == 0) {
        printf("(no running processes)\n");
    } else {
        printf("Total: %d process(es) running\n", running_count);
    }
}

// 清理进程表（退出时调用）
void cleanup_process_table(void) {
    Process* curr = process_list;
    while (curr) {
        if (curr->status == 1) {
            kill(curr->system_pid, SIGKILL);
            waitpid(curr->system_pid, NULL, 0);
        }
        Process* next = curr->next;
        free(curr);
        curr = next;
    }
    process_list = NULL;
    process_count = 0;
    printf("[INFO] All processes cleaned up\n");
}
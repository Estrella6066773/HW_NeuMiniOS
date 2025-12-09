#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

#include <sys/types.h>

#define MAX_PROCESSES 5
#define MAX_PROCESS_NAME 256

// 淇：进程信息结构（使用数组方式）
typedef struct {
    int pid;                    // NeuMiniOS 进程 ID（唯一）
    pid_t system_pid;           // Linux 系统进程 ID
    int status;                 // 进程状态（0=空闲, 1=运行中）
    char name[MAX_PROCESS_NAME]; // 进程名称
} Process;

// 淇：进程管理函数声明
void init_process_table(void);
int create_process(const char* program_name, const char* program_path);
int stop_process(int pid);
void list_processes(void);
void cleanup_process_table(void);

// 淇：兼容性：ProcessManager类型定义（用于接口兼容）
typedef void ProcessManager;

#endif // PROCESS_MANAGER_H


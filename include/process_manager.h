#ifndef NEUMINIOS_PROCESS_MANAGER_H
#define NEUMINIOS_PROCESS_MANAGER_H

#include <sys/types.h>

#define MAX_PROCESSES 5

// 进程信息结构
typedef struct {
    int pid;                // 进程ID (NeuMiniOS自定义)
    pid_t system_pid;       // Linux系统PID
    char name[100];         // 进程名 (如"helloworld")
    int status;             // 状态: 0=未使用, 1=运行中
} Process;

// 兼容类型：为了保持与现有接口的兼容性
// 新的实现使用全局数组，不再需要ProcessManager指针
typedef void ProcessManager;

// 函数声明
void init_process_table(void);
int create_process(const char *program_name, const char *program_path);
int stop_process(int pid);
void list_processes(void);
void cleanup_process_table(void);

#endif // NEUMINIOS_PROCESS_MANAGER_H


#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H
// 防止头文件重复包含

#include <sys/types.h>// 提供 pid_t 类型（进程ID类型）

#define MAX_PROCESSES 5// 最大进程数限制
#define MAX_PROCESS_NAME 256// 进程名称最大长度

// ruby: 进程管理核心结构，记录 NeuMiniOS PID 与系统 PID 的映射及状态
// 进程信息结构（链表节点）
typedef struct Process {
    int pid;                     // NeuMiniOS 进程 ID（唯一）
    pid_t system_pid;            // Linux 系统进程 ID
    int status;                  // 进程状态（0=空闲, 1=运行中）
    char name[MAX_PROCESS_NAME]; // 进程名称
    struct Process* next;        // 指向下一个进程节点
} Process;

// ruby: 进程管理对外 API，供引导、CLI 命令调用
// 进程管理函数声明
void init_process_table(void);
int create_process(const char* program_name, const char* program_path);
int stop_process(int pid);
void list_processes(void);
void cleanup_process_table(void);

// 兼容性：ProcessManager类型定义（用于接口兼容）
typedef void ProcessManager;

#endif // PROCESS_MANAGER_H
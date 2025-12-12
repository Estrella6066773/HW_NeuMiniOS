#ifndef PROCESS_H
#define PROCESS_H

#include <sys/types.h>

#define MAX_PROCESSES 5

// 进程信息结构
typedef struct ProcessNode {
    pid_t pid;                // Linux 进程 ID
    int process_id;           // NeuMiniOS 进程 ID（唯一）
    char* command;            // 执行的命令
    int status;              // 进程状态（0=运行中, 1=已停止）
    struct ProcessNode* next; // 链表指针（用于加分项）
} ProcessNode;

// 进程管理器结构
typedef struct {
    ProcessNode* process_list; // 进程链表（加分项：使用链表）
    // 或者使用数组（基本要求）：
    // ProcessNode processes[MAX_PROCESSES];
    int process_count;        // 当前进程数量
    int next_process_id;      // 下一个可用的进程 ID
} ProcessManager;

// 函数声明
ProcessManager* init_process_manager(void);
void destroy_process_manager(ProcessManager* pm);
int create_process(ProcessManager* pm, pid_t pid, const char* command);
int stop_process(ProcessManager* pm, int process_id);
void list_processes(ProcessManager* pm);
ProcessNode* find_process(ProcessManager* pm, int process_id);
void cleanup_processes(ProcessManager* pm);

#endif // PROCESS_H




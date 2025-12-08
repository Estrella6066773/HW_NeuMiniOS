#include "../include/process.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#define _POSIX_C_SOURCE 200809L

// 初始化进程管理器
ProcessManager* init_process_manager(void) {
    ProcessManager* pm = (ProcessManager*)malloc(sizeof(ProcessManager));
    if (!pm) return NULL;
    
    pm->process_list = NULL;
    pm->process_count = 0;
    pm->next_process_id = 1;
    
    return pm;
}

// 销毁进程管理器
void destroy_process_manager(ProcessManager* pm) {
    if (!pm) return;
    
    cleanup_processes(pm);
    free(pm);
}

// 创建进程（添加到进程列表）
int create_process(ProcessManager* pm, pid_t pid, const char* command) {
    if (!pm || pm->process_count >= MAX_PROCESSES) {
        return -1;
    }
    
    ProcessNode* new_process = (ProcessNode*)malloc(sizeof(ProcessNode));
    if (!new_process) return -1;
    
    new_process->pid = pid;
    new_process->process_id = pm->next_process_id++;
    new_process->command = strdup(command);
    new_process->status = 0; // 运行中
    new_process->next = NULL;
    
    // 添加到链表
    if (pm->process_list == NULL) {
        pm->process_list = new_process;
    } else {
        ProcessNode* current = pm->process_list;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_process;
    }
    
    pm->process_count++;
    return new_process->process_id;
}

// 停止进程
int stop_process(ProcessManager* pm, int process_id) {
    if (!pm) return -1;
    
    ProcessNode* process = find_process(pm, process_id);
    if (!process || process->status != 0) {
        return -1; // 进程未找到或已停止
    }
    
    // 发送 SIGTERM 信号
    if (kill(process->pid, SIGTERM) == 0) {
        process->status = 1; // 标记为已停止
        waitpid(process->pid, NULL, 0); // 等待进程结束
        pm->process_count--;
        return 0;
    }
    
    return -1;
}

// 列出所有进程
void list_processes(ProcessManager* pm) {
    if (!pm) return;
    
    printf("Running processes:\n");
    if (pm->process_count == 0) {
        printf("  (no processes)\n");
        return;
    }
    
    ProcessNode* current = pm->process_list;
    while (current != NULL) {
        if (current->status == 0) { // 只显示运行中的进程
            printf("  [%d] PID: %d, Command: %s, Status: Running\n", 
                   current->process_id, current->pid, current->command);
        }
        current = current->next;
    }
}

// 查找进程
ProcessNode* find_process(ProcessManager* pm, int process_id) {
    if (!pm) return NULL;
    
    ProcessNode* current = pm->process_list;
    while (current != NULL) {
        if (current->process_id == process_id) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;
}

// 清理已停止的进程
void cleanup_processes(ProcessManager* pm) {
    if (!pm) return;
    
    ProcessNode* current = pm->process_list;
    ProcessNode* prev = NULL;
    
    while (current != NULL) {
        // 检查进程是否还在运行
        if (kill(current->pid, 0) != 0) {
            // 进程已结束，从链表中移除
            ProcessNode* to_remove = current;
            if (prev == NULL) {
                pm->process_list = current->next;
            } else {
                prev->next = current->next;
            }
            current = current->next;
            
            free(to_remove->command);
            free(to_remove);
            pm->process_count--;
        } else {
            prev = current;
            current = current->next;
        }
    }
}


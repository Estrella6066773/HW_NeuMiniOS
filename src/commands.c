#include "../include/commands.h"
#include "../include/process.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>

// 主命令分发函数（控制台指令入口）
int execute_command(ParsedCommand* cmd, FileSystem* fs, ProcessManager* pm) {
    if (!cmd || !cmd->command) return -1;
    
    if (strcmp(cmd->command, "list") == 0) {
        return execute_list(fs);
    }
    // 进程管理相关指令
    else if (strcmp(cmd->command, "plist") == 0) {
        return execute_plist(pm);
    }
    else if (strcmp(cmd->command, "stop") == 0) {
        if (cmd->arg_count < 2) {
            printf("Usage: stop <process_id>\n");
            printf("Example: stop 1\n");
            return -1;
        }
        // 淇：验证进程ID格式（增强错误处理）
        char* endptr;
        long process_id_long = strtol(cmd->args[1], &endptr, 10);
        // 淇：检查转换是否成功，以及值是否在有效范围内
        if (*endptr != '\0' || process_id_long <= 0 || process_id_long > INT_MAX) {
            printf("Error: Invalid process ID '%s'. Process ID must be a positive integer (1-%d).\n", 
                   cmd->args[1], INT_MAX);
            printf("Use 'plist' to see running processes and their IDs.\n");
            return -1;
        }
        int process_id = (int)process_id_long;
        return execute_stop(pm, process_id);
    }
    else if (strcmp(cmd->command, "run") == 0) {
        if (cmd->arg_count < 2) {
            printf("Usage: run <filename>\n");
            return -1;
        }
        return execute_run(fs, pm, cmd->args[1]);
    }

    // 文件系统 / 目录相关指令（顺序与 execute_* / file_system 保持一致）
    else if (strcmp(cmd->command, "copy") == 0) {
        if (cmd->arg_count < 3) {
            printf("Usage: copy <src_filename> <dest_filename>\n");
            return -1;
        }
        return execute_copy(fs, cmd->args[1], cmd->args[2]);
    }
    else if (strcmp(cmd->command, "rename") == 0) {
        if (cmd->arg_count < 3) {
            printf("Usage: rename <old_filename> <new_filename>\n");
            return -1;
        }
        return execute_rename(fs, cmd->args[1], cmd->args[2]);
    }
    else if (strcmp(cmd->command, "list") == 0) {
        return execute_list(fs);
    }
    else if (strcmp(cmd->command, "view") == 0) {
        if (cmd->arg_count < 2) {
            printf("Usage: view <filename>\n");
            return -1;
        }
        return execute_view(fs, cmd->args[1]);
    }
    else if (strcmp(cmd->command, "delete") == 0) {
        if (cmd->arg_count < 2) {
            printf("Usage: delete <filename>\n");
            return -1;
        }
        return execute_delete(fs, cmd->args[1]);
    }
    else if (strcmp(cmd->command, "mkdir") == 0) {
        if (cmd->arg_count < 2) {
            printf("Usage: mkdir <directory>\n");
            return -1;
        }
        return execute_mkdir(fs, cmd->args[1]);
    }
    else if (strcmp(cmd->command, "cd") == 0) {
        if (cmd->arg_count < 2) {
            printf("Usage: cd <directory>\n");
            return -1;
        }
        return execute_cd(fs, cmd->args[1]);
    }
    // 系统控制和帮助类指令
    else if (strcmp(cmd->command, "exit") == 0) {
        return -2; // 淇：特殊返回值，表示退出
    }
    else if (strcmp(cmd->command, "help") == 0) {
        printf("NeuMiniOS Command Reference:\n");
        printf("===========================\n\n");
        printf("File Operations:\n");
        printf("  list                    - List all files in current directory\n");
        printf("  view <filename>         - Display file contents\n");
        printf("  delete <filename>       - Delete a file\n");
        printf("  copy <src> <dest>       - Copy a file\n");
        printf("  rename <old> <new>      - Rename a file\n\n");
        printf("Process Operations:\n");
        printf("  plist                   - List all running processes\n");
        printf("  stop <pid>              - Stop a running process\n");
        printf("  run <filename>          - Run an executable file\n\n");
        printf("Directory Operations (bonus):\n");
        printf("  cd <directory>          - Change directory\n");
        printf("  mkdir <directory>      - Create directory\n\n");
        printf("System:\n");
        printf("  exit                    - Exit NeuMiniOS\n");
        printf("  help                    - Show this help message\n\n");
        printf("Command History (bonus):\n");
        printf("  !!                      - Execute previous command\n");
        return 0;
    }
    else if (strcmp(cmd->command, "history") == 0) {
        // 淇：加分项：显示命令历史
        // 淇：注意：实际的历史查看需要通过CLI接口
        printf("Use arrow keys (up/down) to navigate command history\n");
        printf("Note: History navigation requires terminal support\n");
        return 0;
    }
    else {
        printf("Error: Unknown command '%s'\n", cmd->command);
        printf("Available commands:\n");
        printf("  File operations: list, view, delete, copy, rename\n");
        printf("  Process operations: plist, stop, run\n");
        printf("  Directory operations: cd, mkdir (bonus)\n");
        printf("  System: exit\n");
        printf("Type 'help' for more information\n");
        return -1;
    }
}

// =========================
// 进程管理相关执行函数
// =========================

// ruby: CLI -> 进程管理：列出所有已登记进程
// 淇：执行 plist 命令
int execute_plist(ProcessManager* pm) {
    (void)pm;  // 淇：不再需要pm参数，但保持接口兼容
    
    // 淇：确保进程表已初始化
    // 注意：init_process_table()应该在neuboot中调用，这里只是安全检查
    list_processes();
    return 0;
}

// ruby: CLI -> 进程管理：停止指定 NeuMiniOS PID
// 淇：执行 stop 命令
int execute_stop(ProcessManager* pm, int process_id) {
    (void)pm;  // 淇：不再需要pm参数，但保持接口兼容
    
    if (process_id <= 0) {
        printf("Error: Invalid process ID. Process ID must be a positive integer.\n");
        return -1;
    }
    
    return stop_process(process_id);
}

// ruby: CLI -> 进程管理：解包文件后创建运行进程
// 执行 run 命令
int execute_run(FileSystem* fs, ProcessManager* pm, const char* filename) {
    (void)pm;  // 不再需要pm参数，但保持接口兼容
    
    if (!fs || !filename) {
        printf("Usage: run <filename>\n");
        return -1;
    }
    
    // 提取文件到临时位置
    char temp_path[256];
    snprintf(temp_path, sizeof(temp_path), "/tmp/neuminios_%s_%d", filename, getpid());
    
    if (extract_file_to_host(fs, filename, temp_path) != 0) {
        printf("Error: Failed to extract file '%s'\n", filename);
        return -1;
    }
    
    // 使用新的create_process函数，它会处理文件读取、临时文件创建和进程启动
    int process_id = create_process(filename, temp_path);
    if (process_id > 0) {
        return 0;
    } else {
        return -1;
    }
}

// =========================
// 文件系统 | File System
// =========================

// copy <filename> <new_filename>
int execute_copy(FileSystem* fs, const char* src_filename, const char* dest_filename) {
    if (!fs || !src_filename || !dest_filename) {
        printf("Usage: copy <src_filename> <dest_filename>\n");
        return -1;
    }

    FileNode* new_file = copy_file(fs, src_filename, dest_filename);
    if (new_file) {
        printf("File '%s' copied to '%s'\n", src_filename, dest_filename);
        return 0;
    } else {
        printf("Error: Failed to copy file\n");
        return -1;
    }
}

// rename <old_filename> <new_filename>
int execute_rename(FileSystem* fs, const char* old_filename, const char* new_filename) {
    if (!fs || !old_filename || !new_filename) {
        printf("Usage: rename <old_filename> <new_filename>\n");
        return -1;
    }

    if (rename_file(fs, old_filename, new_filename) == 0) {
        printf("File '%s' renamed to '%s'\n", old_filename, new_filename);
        return 0;
    } else {
        printf("Error: Failed to rename file\n");
        return -1;
    }
}

// list
int execute_list(FileSystem* fs) {
    if (!fs) return -1;
    list_files(fs);
    return 0;
}

// view <filename>
int execute_view(FileSystem* fs, const char* filename) {
    if (!fs || !filename) {
        printf("Usage: view <filename>\n");
        return -1;
    }
    return view_file(fs, filename);
}

// delete <filename>
int execute_delete(FileSystem* fs, const char* filename) {
    if (!fs || !filename) {
        printf("Usage: delete <filename>\n");
        return -1;
    }
    
    if (delete_file(fs, filename) == 0) {
        printf("File '%s' deleted successfully\n", filename);
        return 0;
    } else {
        printf("Error: Failed to delete file '%s'\n", filename);
        return -1;
    }
}

// mkdir <directory>
int execute_mkdir(FileSystem* fs, const char* dirname) {
    if (!fs || !dirname) {
        printf("Usage: mkdir <directory>\n");
        return -1;
    }

    FileNode* new_dir = create_directory(fs, dirname);
    if (new_dir) {
        printf("Directory '%s' created successfully\n", dirname);
        return 0;
    } else {
        printf("Error: Failed to create directory\n");
        return -1;
    }
}

// cd <directory>
int execute_cd(FileSystem* fs, const char* dirname) {
    if (!fs || !dirname) {
        printf("Usage: cd <directory>\n");
        return -1;
    }
    
    if (change_directory(fs, dirname) == 0) {
        printf("Changed to directory: %s\n", fs->current_dir->path);
        return 0;
    } else {
        printf("Error: Directory '%s' not found\n", dirname);
        return -1;
    }
}

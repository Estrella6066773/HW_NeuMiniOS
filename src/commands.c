#include "../include/commands.h"
#include "../include/process_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>

// 执行 list 命令
int execute_list(FileSystem* fs) {
    if (!fs) return -1;
    list_files(fs);
    return 0;
}

// 执行 view 命令
int execute_view(FileSystem* fs, const char* filename) {
    if (!fs || !filename) {
        printf("Usage: view <filename>\n");
        return -1;
    }
    return view_file(fs, filename);
}

// 执行 delete 命令
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

// 执行 copy 命令
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

// 执行 rename 命令
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

// 执行 plist 命令
int execute_plist(ProcessManager* pm) {
    (void)pm;  // 不再需要pm参数，但保持接口兼容
    list_processes();
    return 0;
}

// 执行 stop 命令
int execute_stop(ProcessManager* pm, int process_id) {
    (void)pm;  // 不再需要pm参数，但保持接口兼容
    
    if (stop_process(process_id) == 0) {
        return 0;
    } else {
        return -1;
    }
}

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

// 执行 cd 命令（加分项）
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

// 执行 mkdir 命令（加分项）
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

// 主命令分发函数
int execute_command(ParsedCommand* cmd, FileSystem* fs, ProcessManager* pm) {
    if (!cmd || !cmd->command) return -1;
    
    if (strcmp(cmd->command, "list") == 0) {
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
    else if (strcmp(cmd->command, "plist") == 0) {
        return execute_plist(pm);
    }
    else if (strcmp(cmd->command, "stop") == 0) {
        if (cmd->arg_count < 2) {
            printf("Usage: stop <process_id>\n");
            return -1;
        }
        int process_id = atoi(cmd->args[1]);
        return execute_stop(pm, process_id);
    }
    else if (strcmp(cmd->command, "run") == 0) {
        if (cmd->arg_count < 2) {
            printf("Usage: run <filename>\n");
            return -1;
        }
        return execute_run(fs, pm, cmd->args[1]);
    }
    else if (strcmp(cmd->command, "cd") == 0) {
        if (cmd->arg_count < 2) {
            printf("Usage: cd <directory>\n");
            return -1;
        }
        return execute_cd(fs, cmd->args[1]);
    }
    else if (strcmp(cmd->command, "mkdir") == 0) {
        if (cmd->arg_count < 2) {
            printf("Usage: mkdir <directory>\n");
            return -1;
        }
        return execute_mkdir(fs, cmd->args[1]);
    }
    else if (strcmp(cmd->command, "exit") == 0) {
        return -2; // 特殊返回值，表示退出
    }
    else {
        printf("Unknown command: %s\n", cmd->command);
        printf("Available commands: list, view, delete, copy, rename, plist, stop, run");
        printf(", cd, mkdir, exit\n");
        return -1;
    }
}


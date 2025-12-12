#include "../include/cli.h"
#include "../include/commands.h"
#include "../include/file_system.h"
#include "../include/process_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _POSIX_C_SOURCE 200809L

// 淇：初始化 CLI
CLI* init_cli(void) {
    CLI* cli = (CLI*)malloc(sizeof(CLI));
    if (!cli) return NULL;
    
    // 淇：初始化命令历史（加分项）
    cli->history = (CommandHistory*)malloc(sizeof(CommandHistory));
    if (!cli->history) {
        free(cli);
        return NULL;
    }
    
    cli->history->history = (char**)malloc(MAX_HISTORY * sizeof(char*));
    if (!cli->history->history) {
        free(cli->history);
        free(cli);
        return NULL;
    }
    
    cli->history->count = 0;
    cli->history->index = -1;
    cli->history->max_size = MAX_HISTORY;
    
    cli->running = 1;
    
    return cli;
}

// 淇：销毁 CLI
void destroy_cli(CLI* cli) {
    if (!cli) return;
    
    if (cli->history) {
        for (int i = 0; i < cli->history->count; i++) {
            free(cli->history->history[i]);
        }
        free(cli->history->history);
        free(cli->history);
    }
    
    free(cli);
}

// 淇：CLI 主循环（集成命令执行系统）
void cli_loop(CLI* cli, FileSystem* fs, ProcessManager* pm) {
    if (!cli) return;
    
    char* input;
    ParsedCommand* cmd;
    
    while (cli->running) {
        printf("> ");
        fflush(stdout);
        
        input = read_input(cli);
        if (!input) continue;
        
        if (strlen(input) == 0) {
            free(input);
            continue;
        }
        
        // 淇：添加到历史记录
        add_to_history(cli, input);
        
        // 淇：解析命令
        cmd = parse_command(input);
        if (cmd) {
            // 淇：执行命令
            int result = execute_command(cmd, fs, pm);
            if (result == -2) {
                // 淇：exit 命令
                cli->running = 0;
            } else if (result < 0 && result != -2) {
                // 淇：命令执行失败，但不退出CLI（错误已在命令函数中显示）
                // 这里可以添加额外的错误处理逻辑
            }
            free_parsed_command(cmd);
        } else {
            // 淇：解析失败，但输入不为空，可能是无效命令格式
            if (strlen(input) > 0) {
                printf("Error: Invalid command format. Type 'help' for available commands.\n");
            }
        }
        
        free(input);
    }
}

// 淇：读取用户输入（支持基本的历史记录功能）
char* read_input(CLI* cli) {
    if (!cli) return NULL;
    
    char* buffer = (char*)malloc(MAX_INPUT_LENGTH);
    if (!buffer) {
        // 淇：内存分配失败，返回NULL但不崩溃
        return NULL;
    }
    
    // 淇：使用fgets读取输入（简单版本，方向键导航需要更复杂的终端控制）
    if (fgets(buffer, MAX_INPUT_LENGTH, stdin) == NULL) {
        // 淇：EOF或读取错误
        free(buffer);
        return NULL;
    }
    
    // 淇：移除换行符
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
    }
    
    // 淇：检查输入长度，防止缓冲区溢出
    if (len >= MAX_INPUT_LENGTH - 1) {
        // 淇：输入过长，清空缓冲区并提示
        printf("Warning: Input too long (max %d characters). Command truncated.\n", MAX_INPUT_LENGTH - 1);
        buffer[MAX_INPUT_LENGTH - 1] = '\0';
    }
    
    // 淇：处理特殊命令（加分项：历史命令查看）
    if (strcmp(buffer, "!!") == 0) {
        // 淇：执行上一条命令
        if (cli->history && cli->history->count > 0) {
            char* last_cmd = cli->history->history[cli->history->count - 1];
            if (last_cmd) {
                free(buffer);
                char* result = strdup(last_cmd);
                if (!result) {
                    // 淇：内存分配失败，返回原始buffer
                    return buffer;
                }
                return result;
            }
        } else {
            printf("Error: No previous command in history\n");
            free(buffer);
            return NULL;
        }
    }
    
    return buffer;
}

// 淇：解析命令（增强错误处理）
ParsedCommand* parse_command(const char* input) {
    if (!input) {
        return NULL;
    }
    
    // 淇：跳过前导空白字符
    while (*input == ' ' || *input == '\t') {
        input++;
    }
    
    // 淇：检查空输入
    if (*input == '\0') {
        return NULL;
    }
    
    ParsedCommand* cmd = (ParsedCommand*)malloc(sizeof(ParsedCommand));
    if (!cmd) {
        return NULL;
    }
    
    // 淇：初始化
    cmd->command = NULL;
    cmd->args = (char**)malloc(10 * sizeof(char*));
    if (!cmd->args) {
        free(cmd);
        return NULL;
    }
    
    // 淇：初始化参数数组
    for (int i = 0; i < 10; i++) {
        cmd->args[i] = NULL;
    }
    cmd->arg_count = 0;
    
    // 淇：复制输入字符串用于分割
    char* input_copy = strdup(input);
    if (!input_copy) {
        free(cmd->args);
        free(cmd);
        return NULL;
    }
    
    // 淇：分割字符串（支持空格和制表符）
    char* token = strtok(input_copy, " \t\n\r");
    
    while (token != NULL && cmd->arg_count < 10) {
        // 淇：跳过空token
        if (strlen(token) == 0) {
            token = strtok(NULL, " \t\n\r");
            continue;
        }
        
        if (cmd->arg_count == 0) {
            // 淇：第一个token是命令名
            cmd->command = strdup(token);
            if (!cmd->command) {
                free(input_copy);
                free_parsed_command(cmd);
                return NULL;
            }
        }
        
        // 淇：存储参数
        cmd->args[cmd->arg_count] = strdup(token);
        if (!cmd->args[cmd->arg_count]) {
            free(input_copy);
            free_parsed_command(cmd);
            return NULL;
        }
        cmd->arg_count++;
        
        token = strtok(NULL, " \t\n\r");
    }
    
    free(input_copy);
    
    // 淇：验证命令名存在
    if (cmd->arg_count == 0 || !cmd->command) {
        free_parsed_command(cmd);
        return NULL;
    }
    
    return cmd;
}

// 淇：释放解析后的命令
void free_parsed_command(ParsedCommand* cmd) {
    if (!cmd) return;
    
    if (cmd->command) free(cmd->command);
    if (cmd->args) {
        for (int i = 0; i < cmd->arg_count; i++) {
            if (cmd->args[i]) free(cmd->args[i]);
        }
        free(cmd->args);
    }
    free(cmd);
}

// 淇：添加到历史记录
void add_to_history(CLI* cli, const char* command) {
    if (!cli || !cli->history || !command) return;
    
    // 淇：跳过空命令
    if (strlen(command) == 0) return;
    
    // 淇：避免重复添加相同的连续命令
    if (cli->history->count > 0) {
        char* last_cmd = cli->history->history[cli->history->count - 1];
        if (last_cmd && strcmp(last_cmd, command) == 0) {
            return; // 不添加重复命令
        }
    }
    
    // 淇：如果历史记录已满，移除最旧的
    if (cli->history->count >= cli->history->max_size) {
        free(cli->history->history[0]);
        for (int i = 0; i < cli->history->count - 1; i++) {
            cli->history->history[i] = cli->history->history[i + 1];
        }
        cli->history->count--;
    }
    
    // 淇：添加新命令到历史
    cli->history->history[cli->history->count] = strdup(command);
    if (!cli->history->history[cli->history->count]) {
        // 淇：内存分配失败，但不应导致程序崩溃
        return;
    }
    cli->history->count++;
    cli->history->index = cli->history->count - 1;
}

// 淇：获取历史命令（加分项：用于方向键导航）
char* get_history_command(CLI* cli, int direction) {
    if (!cli || !cli->history || cli->history->count == 0) return NULL;
    
    if (direction < 0) {
        // 淇：上一条
        if (cli->history->index > 0) {
            cli->history->index--;
        }
    } else if (direction > 0) {
        // 淇：下一条
        if (cli->history->index < cli->history->count - 1) {
            cli->history->index++;
        }
    }
    
    if (cli->history->index >= 0 && cli->history->index < cli->history->count) {
        return cli->history->history[cli->history->index];
    }
    
    return NULL;
}


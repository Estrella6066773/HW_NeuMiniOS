#include "../include/cli.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _POSIX_C_SOURCE 200809L

// 初始化 CLI
CLI* init_cli(void) {
    CLI* cli = (CLI*)malloc(sizeof(CLI));
    if (!cli) return NULL;
    
    // 初始化命令历史（加分项）
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

// 销毁 CLI
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

// CLI 主循环
void cli_loop(CLI* cli) {
    if (!cli) return;
    
    char* input;
    
    while (cli->running) {
        printf("> ");
        fflush(stdout);
        
        input = read_input(cli);
        if (!input) continue;
        
        if (strlen(input) == 0) continue;
        
        // 添加到历史记录
        add_to_history(cli, input);
        
        // 解析并执行命令（这里只显示，实际执行在 commands.c 中）
        printf("Command: %s\n", input);
        
        // 如果输入 "exit"，退出 CLI
        if (strcmp(input, "exit") == 0) {
            cli->running = 0;
        }
        
        free(input);
    }
}

// 读取用户输入
char* read_input(CLI* cli) {
    (void)cli;  // 未使用参数
    char* buffer = (char*)malloc(MAX_INPUT_LENGTH);
    if (!buffer) return NULL;
    
    if (fgets(buffer, MAX_INPUT_LENGTH, stdin) == NULL) {
        free(buffer);
        return NULL;
    }
    
    // 移除换行符
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
    }
    
    return buffer;
}

// 解析命令
ParsedCommand* parse_command(const char* input) {
    if (!input) return NULL;
    
    ParsedCommand* cmd = (ParsedCommand*)malloc(sizeof(ParsedCommand));
    if (!cmd) return NULL;
    
    cmd->args = (char**)malloc(10 * sizeof(char*)); // 最多10个参数
    if (!cmd->args) {
        free(cmd);
        return NULL;
    }
    
    // 复制输入字符串用于分割
    char* input_copy = strdup(input);
    if (!input_copy) {
        free(cmd->args);
        free(cmd);
        return NULL;
    }
    
    // 分割字符串
    char* token = strtok(input_copy, " \t");
    cmd->arg_count = 0;
    
    while (token != NULL && cmd->arg_count < 10) {
        if (cmd->arg_count == 0) {
            cmd->command = strdup(token);
        }
        cmd->args[cmd->arg_count] = strdup(token);
        cmd->arg_count++;
        token = strtok(NULL, " \t");
    }
    
    free(input_copy);
    return cmd;
}

// 释放解析后的命令
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

// 添加到历史记录
void add_to_history(CLI* cli, const char* command) {
    if (!cli || !cli->history || !command) return;
    
    // 如果历史记录已满，移除最旧的
    if (cli->history->count >= cli->history->max_size) {
        free(cli->history->history[0]);
        for (int i = 0; i < cli->history->count - 1; i++) {
            cli->history->history[i] = cli->history->history[i + 1];
        }
        cli->history->count--;
    }
    
    cli->history->history[cli->history->count] = strdup(command);
    cli->history->count++;
    cli->history->index = cli->history->count - 1;
}

// 获取历史命令（加分项：用于方向键导航）
char* get_history_command(CLI* cli, int direction) {
    if (!cli || !cli->history || cli->history->count == 0) return NULL;
    
    if (direction < 0) {
        // 上一条
        if (cli->history->index > 0) {
            cli->history->index--;
        }
    } else if (direction > 0) {
        // 下一条
        if (cli->history->index < cli->history->count - 1) {
            cli->history->index++;
        }
    }
    
    if (cli->history->index >= 0 && cli->history->index < cli->history->count) {
        return cli->history->history[cli->history->index];
    }
    
    return NULL;
}


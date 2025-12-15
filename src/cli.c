#include "../include/cli.h"
#include "../include/commands.h"
#include "../include/file_system.h"
#include "../include/process_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
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
    
    // 显示提示符（仅用于用户输入行）
    printf("> ");
    fflush(stdout);
    
    struct termios oldt, newt;
    if (tcgetattr(STDIN_FILENO, &oldt) == -1) {
        return NULL;
    }
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO); // 关闭规范模式和回显
    newt.c_cc[VMIN] = 1;
    newt.c_cc[VTIME] = 0;
    if (tcsetattr(STDIN_FILENO, TCSANOW, &newt) == -1) {
        return NULL;
    }
    
    char buffer[MAX_INPUT_LENGTH] = {0};
    int len = 0;
    int cursor_pos = 0; // 光标在缓冲区中的位置
    int old_len = 0; // 用于重绘时清理残余字符
    
    // 辅助函数：重绘当前行并移动光标到正确位置
    void redraw_line(void) {
        // 清除当前行并重绘
        printf("\r> %s", buffer);
        // 移动光标到正确位置：提示符 "> " 占2个字符，加上cursor_pos
        int target_col = 2 + cursor_pos;
        int current_col = 2 + len;
        if (target_col < current_col) {
            // 光标在中间，需要向左移动
            printf("\033[%dD", current_col - target_col);
        } else if (target_col > current_col) {
            // 光标在末尾之后（理论上不应该发生）
            printf("\033[%dC", target_col - current_col);
        }
        fflush(stdout);
    }
    
    while (1) {
        unsigned char ch;
        ssize_t read_bytes = read(STDIN_FILENO, &ch, 1);
        if (read_bytes <= 0) {
            // 读取失败或 EOF
            tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
            return NULL;
        }
        
        if (ch == '\n' || ch == '\r') {
            // 回车结束输入
            putchar('\n');
            buffer[len] = '\0';
            tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
            
            // 处理 "!!" 快捷历史
            if (strcmp(buffer, "!!") == 0) {
                if (cli->history && cli->history->count > 0) {
                    char* last_cmd = cli->history->history[cli->history->count - 1];
                    if (last_cmd) {
                        char* result = strdup(last_cmd);
                        return result;
                    }
                } else {
                    printf("Error: No previous command in history\n");
                    return NULL;
                }
            }
            
            char* result = strdup(buffer);
            return result;
        } else if (ch == 0x7f || ch == 0x08) { // 退格 (Backspace)
            if (cursor_pos > 0) {
                // 将光标后的字符前移
                for (int i = cursor_pos - 1; i < len; i++) {
                    buffer[i] = buffer[i + 1];
                }
                len--;
                cursor_pos--;
                buffer[len] = '\0';
                redraw_line();
            }
            continue;
        } else if (ch == 0x1b) { // 可能是方向键序列
            unsigned char seq[2];
            if (read(STDIN_FILENO, &seq[0], 1) <= 0 || read(STDIN_FILENO, &seq[1], 1) <= 0) {
                continue;
            }
            if (seq[0] == '[') {
                if (seq[1] == 'A') {
                    // 上箭头：上一条历史
                    char* hist = get_history_command(cli, -1);
                    if (hist) {
                        old_len = len;
                        len = (int)snprintf(buffer, sizeof(buffer), "%s", hist);
                        cursor_pos = len; // 光标移到末尾
                        redraw_line();
                    }
                } else if (seq[1] == 'B') {
                    // 下箭头：下一条历史
                    char* hist = get_history_command(cli, 1);
                    if (hist) {
                        old_len = len;
                        len = (int)snprintf(buffer, sizeof(buffer), "%s", hist);
                        cursor_pos = len; // 光标移到末尾
                        redraw_line();
                    } else {
                        // 如果没有下一条，清空输入
                        if (len > 0) {
                            old_len = len;
                            len = 0;
                            cursor_pos = 0;
                            buffer[0] = '\0';
                            printf("\r> ");
                            for (int i = 0; i < old_len; i++) putchar(' ');
                            printf("\r> ");
                            fflush(stdout);
                        }
                    }
                } else if (seq[1] == 'C') {
                    // 右箭头：光标右移
                    if (cursor_pos < len) {
                        cursor_pos++;
                        printf("\033[C"); // ANSI: 光标右移
                        fflush(stdout);
                    }
                } else if (seq[1] == 'D') {
                    // 左箭头：光标左移
                    if (cursor_pos > 0) {
                        cursor_pos--;
                        printf("\033[D"); // ANSI: 光标左移
                        fflush(stdout);
                    }
                }
            }
            continue;
        } else if (ch == 4) { // Ctrl+D
            tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
            return NULL;
        } else {
            // 普通字符：在光标位置插入
            if (len < MAX_INPUT_LENGTH - 1) {
                // 如果光标不在末尾，需要移动后面的字符
                if (cursor_pos < len) {
                    for (int i = len; i > cursor_pos; i--) {
                        buffer[i] = buffer[i - 1];
                    }
                }
                buffer[cursor_pos] = (char)ch;
                len++;
                cursor_pos++;
                buffer[len] = '\0';
                // 重绘整行以确保正确显示
                redraw_line();
            } else {
                // 达到长度上限，忽略超出部分
                continue;
            }
        }
    }
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
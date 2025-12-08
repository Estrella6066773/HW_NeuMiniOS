#ifndef CLI_H
#define CLI_H

#define MAX_INPUT_LENGTH 256
#define MAX_HISTORY 100

// 命令历史结构（加分项）
typedef struct {
    char** history;      // 历史命令数组
    int count;           // 历史命令数量
    int index;           // 当前历史索引
    int max_size;        // 最大历史记录数
} CommandHistory;

// CLI 结构
typedef struct {
    CommandHistory* history; // 命令历史（加分项）
    int running;             // CLI 运行状态
} CLI;

// 解析后的命令结构
typedef struct {
    char* command;      // 命令名
    char** args;        // 参数数组
    int arg_count;      // 参数数量
} ParsedCommand;

// 函数声明
CLI* init_cli(void);
void destroy_cli(CLI* cli);
void cli_loop(CLI* cli);
char* read_input(CLI* cli);
ParsedCommand* parse_command(const char* input);
void free_parsed_command(ParsedCommand* cmd);
void add_to_history(CLI* cli, const char* command);
char* get_history_command(CLI* cli, int direction); // direction: -1=上一条, 1=下一条

#endif // CLI_H


#ifndef COMMANDS_H
#define COMMANDS_H

#include "file_system.h"
#include "process_manager.h"
#include "cli.h"

// 命令执行函数声明
int execute_list(FileSystem* fs);
int execute_view(FileSystem* fs, const char* filename);
int execute_delete(FileSystem* fs, const char* filename);
int execute_copy(FileSystem* fs, const char* src_filename, const char* dest_filename);
int execute_rename(FileSystem* fs, const char* old_filename, const char* new_filename);
int execute_plist(ProcessManager* pm);// 进程管理里接口，命令能显示进程列表
int execute_stop(ProcessManager* pm, int process_id);// 命令能停止进程
int execute_run(FileSystem* fs, ProcessManager* pm, const char* filename);// 命令能创建进程
int execute_cd(FileSystem* fs, const char* dirname);      // 加分项
int execute_mkdir(FileSystem* fs, const char* dirname);  // 加分项

// 主命令分发函数
int execute_command(ParsedCommand* cmd, FileSystem* fs, ProcessManager* pm);

#endif // COMMANDS_H
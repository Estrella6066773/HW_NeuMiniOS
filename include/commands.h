#ifndef COMMANDS_H
#define COMMANDS_H

#include "file_system.h"
#include "process.h"
#include "cli.h"

// 命令执行函数声明
// 文件管理 | File System（顺序与 file_system 保持一致）
int execute_copy(FileSystem* fs, const char* src_filename, const char* dest_filename);
int execute_rename(FileSystem* fs, const char* old_filename, const char* new_filename);
int execute_list(FileSystem* fs);
int execute_view(FileSystem* fs, const char* filename);
int execute_delete(FileSystem* fs, const char* filename);
int execute_mkdir(FileSystem* fs, const char* dirname);   // mkdir <directory>
int execute_cd(FileSystem* fs, const char* dirname);      // cd <directory>

// 进程管理 | Process
int execute_plist(ProcessManager* pm);                     // 进程管理里接口，命令能显示进程列表
int execute_stop(ProcessManager* pm, int process_id);      // 命令能停止进程
int execute_run(FileSystem* fs, ProcessManager* pm, const char* filename); // 命令能创建进程

// 主命令分发函数
int execute_command(ParsedCommand* cmd, FileSystem* fs, ProcessManager* pm);

#endif // COMMANDS_H
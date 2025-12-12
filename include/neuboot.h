#ifndef NEUBOOT_H
#define NEUBOOT_H

#include "file_system.h"
#include "cli.h"
#include "process_manager.h"

// 引导加载器配置
#define DEFAULT_FILES_DIR "./neuminios_files"

// 函数声明
void neuboot_start(void);
int load_files_from_directory(FileSystem* fs, const char* directory_path);
void display_boot_info(FileSystem* fs);  // 加分项：显示启动信息
size_t calculate_directory_size(const char* directory_path);

#endif // NEUBOOT_H




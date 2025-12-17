#include "../include/neuboot.h"
#include "../include/commands.h"
#include "../include/cli.h"
#include "../include/process.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

// 启动 NeuBoot 引导加载器
void neuboot_start(void) {
    printf("========================================\n");
    printf("    NeuMiniOS Boot Loader (NeuBoot)\n");
    printf("========================================\n\n");
    
    // 初始化文件系统
    FileSystem* fs = init_file_system();
    if (!fs) {
        printf("Error: Failed to initialize file system\n");
        return;
    }
    
    // ruby(init)：引导阶段初始化进程表，确保 CLI 运行前没有残留进程
    // 初始化进程表
    init_process_table();

    // Est:文件系统
    // 从linux的目录加载文件到虚拟的磁盘（磁盘镜像）
    printf("Loading files from directory: %s\n", DEFAULT_FILES_DIR);
    int files_loaded = load_files_from_directory(fs, DEFAULT_FILES_DIR);
    printf("Loaded %d files into Disk Image\n\n", files_loaded);
    
    // 显示启动信息（加分项）
    display_boot_info(fs);
    
    // 启动 CLI
    printf("\nNeuMiniOS ready. Starting CLI...\n");
    printf("Type 'exit' to quit\n\n");
    
    CLI* cli = init_cli();
    if (!cli) {
        printf("Error: Failed to initialize CLI\n");
        cleanup_process_table();
        destroy_file_system(fs);
        return;
    }
    
    // 淇：使用CLI主循环（集成命令执行系统）
    Process* pm = NULL;  // 淇：保持接口兼容，但实际不再使用
    cli_loop(cli, fs, pm);
    
    // 清理资源
    printf("\nShutting down NeuMiniOS...\n");
    destroy_cli(cli);
    cleanup_process_table();
    destroy_file_system(fs);
    printf("Goodbye!\n");
}

// Est:
// 从目录加载文件到磁盘镜像
int load_files_from_directory(FileSystem* fs, const char* directory_path) {
    if (!fs || !directory_path) return 0;
    
    DIR* dir = opendir(directory_path);
    if (!dir) {
        printf("Warning: Cannot open directory '%s'\n", directory_path);
        return 0;
    }
    
    struct dirent* entry;
    struct stat file_stat;
    char file_path[512];
    int files_loaded = 0;
    
    while ((entry = readdir(dir)) != NULL) {
        // 跳过 . 和 ..
        // 保证只往下遍历，而不会返回上级
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        // 构建完整路径
        snprintf(file_path, sizeof(file_path), "%s/%s", directory_path, entry->d_name);
        
        // 获取文件信息
        if (stat(file_path, &file_stat) != 0) {
            continue;
        }
        
        // 只处理普通文件
        if (S_ISREG(file_stat.st_mode)) {
            // 读取文件内容
            FILE* fp = fopen(file_path, "rb");
            if (fp) {
                void* file_data = malloc(file_stat.st_size);
                if (file_data) {
                    size_t bytes_read = fread(file_data, 1, file_stat.st_size, fp);
                    if (bytes_read == (size_t)file_stat.st_size) {
                        // 添加到文件系统
                        FileNode* file = add_file(fs, entry->d_name, "/", file_data, file_stat.st_size);
                        if (file) {
                            files_loaded++;
                            printf("  Loaded: %s (%zu bytes)\n", entry->d_name, file_stat.st_size);
                        } else {
                            free(file_data);
                        }
                    } else {
                        free(file_data);
                    }
                }
                fclose(fp);
            }
        }
    }
    
    closedir(dir);
    return files_loaded;
}

// 显示启动信息
void display_boot_info(FileSystem* fs) {
    if (!fs) return;
    
    printf("=== Boot Information ===\n");
    printf("Disk Image Size: %zu bytes\n", fs->total_size);
    
    // 列出所有加载的文件
    printf("\nFiles in Disk Image:\n");
    list_files(fs);
}

// // 计算目录大小（辅助函数）
// size_t calculate_directory_size(const char* directory_path) {
//     size_t total_size = 0;
//     DIR* dir = opendir(directory_path);
//     if (!dir) return 0;
//
//     struct dirent* entry;
//     struct stat file_stat;
//     char file_path[512];
//
//     while ((entry = readdir(dir)) != NULL) {
//         if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
//             continue;
//         }
//
//         snprintf(file_path, sizeof(file_path), "%s/%s", directory_path, entry->d_name);
//         if (stat(file_path, &file_stat) == 0 && S_ISREG(file_stat.st_mode)) {
//             total_size += file_stat.st_size;
//         }
//     }
//
//     closedir(dir);
//     return total_size;
// }
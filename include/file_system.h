#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include <stddef.h>

// 文件节点结构（使用链表）
typedef struct FileNode {
    char* filename;           // 文件名
    char* path;               // 文件路径（用于目录支持）
    void* data;               // 文件内容的内存指针
    size_t size;              // 文件大小（字节）
    int is_directory;         // 是否为目录（0=文件, 1=目录）
    struct FileNode* children; // 子文件/目录（用于目录层次）
    struct FileNode* next;     // 同级文件/目录（链表指针）
    struct FileNode* parent;   // 父目录指针
} FileNode;

// 文件系统结构
typedef struct {
    FileNode* root;           // 根节点
    FileNode* current_dir;    // 当前目录
    size_t total_size;        // 磁盘镜像总大小
} FileSystem;

// 函数声明
FileSystem* init_file_system(void);
void destroy_file_system(FileSystem* fs);
FileNode* add_file(FileSystem* fs, const char* filename, const char* path, void* data, size_t size);
FileNode* find_file(FileSystem* fs, const char* filename);
int delete_file(FileSystem* fs, const char* filename);
FileNode* copy_file(FileSystem* fs, const char* src_filename, const char* dest_filename);
int rename_file(FileSystem* fs, const char* old_filename, const char* new_filename);
void list_files(FileSystem* fs);
int view_file(FileSystem* fs, const char* filename);
int extract_file_to_host(FileSystem* fs, const char* filename, const char* host_path);
FileNode* create_directory(FileSystem* fs, const char* dirname);
int change_directory(FileSystem* fs, const char* dirname);
void print_file_info(FileNode* file);

#endif // FILE_SYSTEM_H



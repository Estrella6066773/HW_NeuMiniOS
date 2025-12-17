#include "../include/file_system.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

// By Est
// 初始化文件系统
FileSystem* init_file_system(void) {
    FileSystem* fs = (FileSystem*)malloc(sizeof(FileSystem));
    if (!fs) return NULL;
    
    // 创建根目录
    FileNode* root = (FileNode*)malloc(sizeof(FileNode));
    if (!root) {
        free(fs);
        return NULL;
    }
    
    root->filename = strdup("/");
    root->path = strdup("/");
    root->data = NULL;
    root->size = 0;
    root->is_directory = true;
    // 这里每次都只连接一个节点（链表）
    // 所以同级和子集都存在顺序（横向，纵向）
    root->children = NULL;
    root->next = NULL;
    root->parent = NULL;
    
    fs->root = root;
    fs->current_dir = root;
    fs->total_size = 0;
    
    return fs;
}

// 递归释放文件节点及其子节点
static void free_file_node(FileNode* node) {
    if (!node) return;

    // 先释放子节点，再处理兄弟节点，避免悬挂指针
    free_file_node(node->children);
    free_file_node(node->next);

    free(node->filename);
    free(node->path);
    if (!node->is_directory && node->data) {
        free(node->data);
    }
    free(node);
}

// 销毁文件系统
// destroy file system to free the memory.
void destroy_file_system(FileSystem* fs) {
    if (!fs) return;
    
    free_file_node(fs->root);
    
    free(fs);
}

/*
 * 向文件系统当前目录添加一个文件
 *
 * 该函数创建一个新的文件节点，分配必要的内存，并将文件添加到当前目录的子节点链表中。
 * 如果提供的路径为NULL，则使用根路径"/"作为默认路径。
 * 
 * @param fs      指向文件系统的指针，不能为NULL
 * @param filename 文件名，不能为NULL
 * @param path     文件路径，如果为NULL则使用默认根路径"/"
 * @param data     文件数据指针，不能为NULL（可以是普通数据指针或AutoSizedData指针）
 * @param size     文件长度，方法仅在复制时使用，故直接给予
 *
 * @return 指向新创建的FileNode的指针，失败返回NULL
 */
FileNode* add_file(FileSystem* fs, const char* filename, const char* path, void* data, size_t size) {
    if (!fs || !filename || !data) return NULL;
    
    FileNode* new_file = (FileNode*)malloc(sizeof(FileNode));
    if (!new_file) return NULL;
    
    new_file->filename = strdup(filename);
    new_file->path = path ? strdup(path) : strdup("/");
    new_file->data = malloc(size);
    // 数据为空时，撤销行为，然后退出
    if (!new_file->data) {
        free(new_file->filename);
        free(new_file->path);
        free(new_file);
        return NULL;
    }
    memcpy(new_file->data, data, size);
    new_file->size = size;
    new_file->is_directory = false;
    new_file->children = NULL;
    new_file->next = NULL;
    new_file->parent = fs->current_dir;
    
    // 添加到当前目录的子节点链表
    if (fs->current_dir->children == NULL) {
        fs->current_dir->children = new_file;
    } else {
        FileNode* current = fs->current_dir->children;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_file;
    }
    
    fs->total_size += size;
    return new_file;
}

// 查找文件
// search file
FileNode* find_file(FileSystem* fs, const char* filename) {
    if (!fs || !filename) return NULL;

    FileNode* current = fs->current_dir->children; // 被检查的文件，在当前目录current_dir下查找
    while (current != NULL) {
        if (strcmp(current->filename, filename) == 0 && !current->is_directory) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;
}


// 复制文件
// copy <filename>
FileNode* copy_file(FileSystem* fs, const char* src_filename, const char* dest_filename) {
    FileNode* src_file = find_file(fs, src_filename);
    if (!src_file) return NULL;
    
    return add_file(fs, dest_filename, fs->current_dir->path, src_file->data, src_file->size);
}

// 重命名文件
// rename <filename>
int rename_file(FileSystem* fs, const char* old_filename, const char* new_filename) {
    FileNode* file = find_file(fs, old_filename);
    if (!file) return -1;
    
    free(file->filename);
    file->filename = strdup(new_filename); //由于是新分配，所以要释放原有的，防止内存泄漏
    return 0;
}

// 列出当前目录的所有文件
// list
void list_files(FileSystem* fs) {
    if (!fs || !fs->current_dir) return;
    
    printf("Files in current directory:\n");
    FileNode* current = fs->current_dir->children;
    
    if (current == NULL) {
        printf("  (empty)\n");
        return;
    }
    
    while (current != NULL) {
        if (current->is_directory) {
            printf("  [DIR]  %s\n", current->filename);
        } else {
            printf("  [FILE] %s (%zu bytes)\n", current->filename, current->size);
        }
        current = current->next;
    }
}

// 查看文件内容
// view <filename>
int view_file(FileSystem* fs, const char* filename) {
    FileNode* file = find_file(fs, filename);
    // 为空
    if (!file) {
        printf("Error: File '%s' not found\n", filename);
        return -1;
    }

    // 是目录
    if (file->is_directory) {
        printf("Error: '%s' is a directory\n", filename);
        return -1;
    }
    
    // 假设是文本文件，直接打印
    printf("%.*s\n", (int)file->size, (char*)file->data);
    return 0;
}

// 删除文件，文件不内含文件，只有目录会内含文件
// delete <filename>
int delete_file(FileSystem* fs, const char* filename) {
    if (!fs || !filename) return -1;

    FileNode* prev = NULL;
    FileNode* current = fs->current_dir->children; //在当前目录下对文件进行查找

    while (current != NULL) {
        if (strcmp(current->filename, filename) == 0 && !current->is_directory) {
            // 从链表中移除
            if (prev == NULL) {
                fs->current_dir->children = current->next;
            } else {
                prev->next = current->next;
            }

            // 释放内存
            fs->total_size -= current->size;
            free(current->filename);
            free(current->path);
            free(current->data);
            free(current);

            return 0;
        }
        prev = current;
        current = current->next;
    }

    return -1; // 文件未找到
}

// 创建目录
// mkdir <directory>
FileNode* create_directory(FileSystem* fs, const char* dirname) {
    if (!fs || !dirname) return NULL;
    
    FileNode* new_dir = (FileNode*)malloc(sizeof(FileNode));
    if (!new_dir) return NULL;
    
    new_dir->filename = strdup(dirname);
    char* new_path = (char*)malloc(strlen(fs->current_dir->path) + strlen(dirname) + 2);
    sprintf(new_path, "%s%s/", fs->current_dir->path, dirname);
    new_dir->path = new_path;
    new_dir->data = NULL;
    new_dir->size = 0;
    new_dir->is_directory = true;
    new_dir->children = NULL;
    new_dir->next = NULL;
    new_dir->parent = fs->current_dir;
    
    // 添加到当前目录
    if (fs->current_dir->children == NULL) {
        fs->current_dir->children = new_dir;
    } else {
        FileNode* current = fs->current_dir->children;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_dir;
    }
    
    return new_dir;
}

// 切换目录，只能切换到上下的一层
// cd <directory>
int change_directory(FileSystem* fs, const char* dirname) {
    if (!fs || !dirname) return -1;
    
    if (strcmp(dirname, "..") == 0) {
        // 返回父目录
        if (fs->current_dir->parent != NULL) {
            fs->current_dir = fs->current_dir->parent;
            return 0;
        }
        return -1;
    }
    
    // 查找子目录
    FileNode* current = fs->current_dir->children;
    while (current != NULL) {
        if (strcmp(current->filename, dirname) == 0 && current->is_directory) {
            fs->current_dir = current;
            return 0;
        }
        current = current->next;
    }
    
    return -1; // 目录未找到
}

// 打印文件信息
void print_file_info(FileNode* file) {
    if (!file) return;
    printf("File: %s, Size: %zu bytes, Path: %s\n", 
           file->filename, file->size, file->path);
}

// 提取文件到主机系统，用于在进程管理运行程序
int extract_file_to_host(FileSystem* fs, const char* filename, const char* host_path) {
    FileNode* file = find_file(fs, filename);
    if (!file || file->is_directory) return -1;

    FILE* fp = fopen(host_path, "wb");
    if (!fp) return -1;

    size_t written = fwrite(file->data, 1, file->size, fp);
    fclose(fp);

    if (written != file->size) return -1;

    // 设置可执行权限
    chmod(host_path, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);

    return 0;
}
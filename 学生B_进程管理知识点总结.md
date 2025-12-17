# 学生B - 进程管理子系统知识点总结

## 📋 目录
1. [你需要干什么](#你需要干什么)
2. [你需要掌握什么](#你需要掌握什么)
3. [最终输出什么](#最终输出什么)
4. [与别人的接口](#与别人的接口)
5. [如何运作的](#如何运作的)

---

## 🎯 你需要干什么

### 核心任务
作为**学生B**，你负责实现**进程管理子系统**，这是NeuMiniOS操作系统的核心组件之一，权重占**20%**。

### 具体工作内容

#### 1. 实现进程表管理
- 创建一个进程表，最多管理**5个运行中的进程**
- 使用**数组结构**存储进程信息（基本要求）
- 每个进程需要记录：
  - NeuMiniOS自定义的进程ID（PID）
  - Linux系统的真实进程ID（system_pid）
  - 进程名称
  - 进程状态（运行中/空闲）

#### 2. 实现三个核心命令
- **`run <文件名>`**：创建新进程并执行可执行文件
- **`plist`**：列出所有正在运行的进程
- **`stop <进程ID>`**：停止指定的进程

#### 3. 实现进程生命周期管理
- **进程创建**：使用`fork()`创建子进程
- **进程执行**：使用`execl()`执行程序
- **进程终止**：使用`kill()`发送信号终止进程
- **进程清理**：系统退出时清理所有进程

---

## 📚 你需要掌握什么

### 1. C语言基础知识

#### 结构体定义
```c
typedef struct {
    int pid;                    // NeuMiniOS 进程 ID（唯一）
    pid_t system_pid;           // Linux 系统进程 ID
    int status;                 // 进程状态（0=空闲, 1=运行中）
    char name[MAX_PROCESS_NAME]; // 进程名称
} Process;
```

#### 静态数组管理
```c
#define MAX_PROCESSES 5
static Process process_table[MAX_PROCESSES];
static int next_pid = 1;  // 用于生成唯一PID
```

### 2. Linux系统调用

#### fork() - 创建子进程
```c
pid_t system_pid = fork();
if (system_pid == 0) {
    // 子进程代码
    execl(temp_path, program_name, (char *)NULL);
} else if (system_pid > 0) {
    // 父进程代码
    // 记录进程信息
}
```

**关键点**：
- `fork()`返回0表示在子进程中
- `fork()`返回正数表示在父进程中（返回值是子进程的PID）
- `fork()`返回-1表示创建失败

#### execl() - 执行程序
```c
execl(temp_path, program_name, (char *)NULL);
```
- 用新程序替换当前进程的代码
- 如果成功，不会返回（因为进程被替换了）
- 如果失败，返回-1并设置errno

#### kill() - 发送信号
```c
kill(process_table[i].system_pid, SIGTERM);  // 发送终止信号
kill(process_table[i].system_pid, 0);        // 检查进程是否存在
```

#### waitpid() - 等待进程结束
```c
waitpid(process_table[i].system_pid, &status, 0);
```
- 等待指定进程结束
- 回收子进程资源，避免僵尸进程

### 3. 文件操作

#### 读取二进制文件
```c
unsigned char* read_file(const char *path, size_t *out_size) {
    FILE *f = fopen(path, "rb");
    fseek(f, 0, SEEK_END);
    *out_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    unsigned char *buffer = malloc(*out_size);
    fread(buffer, 1, *out_size, f);
    fclose(f);
    return buffer;
}
```

#### 创建临时文件
```c
char temp_path[] = "/tmp/neumini_XXXXXX";
int fd = mkstemp(temp_path);  // 创建唯一临时文件
```

#### 设置文件权限
```c
chmod(path, 0755);  // 设置为可执行
```

### 4. 进程管理核心概念

#### 进程状态
- **0 = 空闲**：进程表槽位未被使用
- **1 = 运行中**：进程正在执行

#### 进程ID管理
- **NeuMiniOS PID**：你自己分配的进程编号（1, 2, 3...）
- **System PID**：Linux系统分配的真实进程ID

#### 进程表查找
```c
// 查找空闲槽位
int slot = -1;
for (int i = 0; i < MAX_PROCESSES; i++) {
    if (process_table[i].status == 0) {
        slot = i;
        break;
    }
}
```

---

## 📤 最终输出什么

### 1. 代码文件

#### 必须提交的文件
- **`include/process_manager.h`** - 头文件
  - 定义`Process`结构体
  - 声明所有函数接口
  
- **`src/process.c`** - 实现文件
  - 实现所有进程管理功能

### 2. 实现的功能

#### ✅ 基本功能（必须完成）

1. **`run`命令**
   - 从磁盘镜像提取可执行文件
   - 创建临时文件并设置可执行权限
   - 使用`fork()`创建子进程
   - 子进程执行程序
   - 父进程记录进程信息
   - 返回进程ID

2. **`plist`命令**
   - 遍历进程表
   - 显示所有运行中的进程
   - 显示格式：
     ```
     === Running Processes (max 5) ===
     PID        System PID Name                 Status
     ----------------------------------------------------------------
     1          12345      helloworld           Running
     ```

3. **`stop`命令**
   - 根据NeuMiniOS PID查找进程
   - 检查进程是否存在
   - 发送SIGTERM信号终止进程
   - 等待进程结束
   - 更新进程状态为空闲

4. **初始化和清理**
   - `init_process_table()`：系统启动时初始化
   - `cleanup_process_table()`：系统退出时清理所有进程

### 3. 测试验证

#### 测试场景
- ✅ 创建进程（`run helloworld`）
- ✅ 查看进程列表（`plist`）
- ✅ 停止进程（`stop 1`）
- ✅ 进程表满的情况（运行5个进程后，第6个应该报错）
- ✅ 停止不存在的进程（应该报错）
- ✅ 系统退出时清理所有进程

---

## 🔗 与别人的接口

### 1. 与命令系统模块（同学A负责）的接口

#### 你的函数被调用
```c
// 在 commands.c 中
int execute_run(FileSystem* fs, ProcessManager* pm, const char* filename) {
    // 1. 先调用文件系统提取文件
    extract_file_to_host(fs, filename, temp_path);
    
    // 2. 然后调用你的函数创建进程
    int process_id = create_process(filename, temp_path);
    return process_id > 0 ? 0 : -1;
}

int execute_plist(ProcessManager* pm) {
    // 直接调用你的函数
    list_processes();
    return 0;
}

int execute_stop(ProcessManager* pm, int process_id) {
    // 直接调用你的函数
    return stop_process(process_id);
}
```

#### 你需要提供的接口
```c
// 在 process_manager.h 中声明
int create_process(const char* program_name, const char* program_path);
void list_processes(void);
int stop_process(int pid);
void init_process_table(void);
void cleanup_process_table(void);
```

### 2. 与文件系统模块（同学C负责）的接口

#### 你如何使用文件系统
- **不直接调用**文件系统函数
- **通过命令系统**间接使用：
  - `execute_run()`先调用`extract_file_to_host()`提取文件
  - 然后将临时文件路径传给你的`create_process()`

#### 数据流向
```
用户输入: run helloworld
    ↓
commands.c: execute_run()
    ↓
file_system.c: extract_file_to_host()  ← 同学C负责
    ↓
process.c: create_process()  ← 你负责
    ↓
fork() + execl()
```

### 3. 与NeuBoot引导加载器（同学A负责）的接口

#### 系统启动时
```c
// 在 neuboot.c 中
void neuboot() {
    // ...
    init_process_table();  // 调用你的初始化函数
    // ...
    cli_loop();
}
```

#### 系统退出时
```c
// 在 main.c 或 cli.c 中
void cleanup() {
    cleanup_process_table();  // 调用你的清理函数
}
```

### 4. 接口总结

| 你的函数 | 被谁调用 | 作用 |
|---------|---------|------|
| `init_process_table()` | NeuBoot/系统启动 | 初始化进程表 |
| `create_process()` | `execute_run()` | 创建新进程 |
| `list_processes()` | `execute_plist()` | 列出所有进程 |
| `stop_process()` | `execute_stop()` | 停止进程 |
| `cleanup_process_table()` | 系统退出 | 清理所有进程 |

---

## ⚙️ 如何运作的

### 1. 整体架构

```
┌─────────────────────────────────────────┐
│          NeuMiniOS 系统                  │
├─────────────────────────────────────────┤
│  CLI (命令行界面)                         │
│    ↓                                     │
│  Commands (命令解析)                      │
│    ↓                                     │
│  ┌──────────────┐  ┌──────────────┐    │
│  │ 文件系统模块   │  │ 进程管理模块   │    │
│  │ (同学C)       │  │ (你负责)      │    │
│  └──────────────┘  └──────────────┘    │
│         ↓                    ↓          │
│   磁盘镜像管理           进程表管理        │
└─────────────────────────────────────────┘
```

### 2. 进程创建流程（`run`命令）

```
用户输入: run helloworld
    ↓
[1] commands.c: execute_run()
    ├─ 调用文件系统: extract_file_to_host()
    │   └─ 从磁盘镜像提取文件到 /tmp/neuminios_helloworld_XXXX
    └─ 调用进程管理: create_process("helloworld", temp_path)
        ↓
[2] process.c: create_process()
    ├─ [2.1] 查找空闲槽位
    │   └─ 遍历 process_table[]，找 status == 0 的槽位
    │
    ├─ [2.2] 读取文件到内存
    │   └─ read_file(temp_path, &size)
    │
    ├─ [2.3] 创建新的临时文件
    │   └─ mkstemp("/tmp/neumini_XXXXXX")
    │
    ├─ [2.4] 写入文件并设置权限
    │   └─ write_file() + chmod(0755)
    │
    ├─ [2.5] 创建子进程
    │   └─ fork()
    │       ├─ 子进程: execl(temp_path, "helloworld", NULL)
    │       └─ 父进程: 记录进程信息
    │           ├─ process_table[slot].pid = next_pid++
    │           ├─ process_table[slot].system_pid = system_pid
    │           ├─ process_table[slot].name = "helloworld"
    │           └─ process_table[slot].status = 1
    │
    └─ [2.6] 返回进程ID
        └─ return process_table[slot].pid
```

### 3. 进程列表流程（`plist`命令）

```
用户输入: plist
    ↓
[1] commands.c: execute_plist()
    └─ 调用: list_processes()
        ↓
[2] process.c: list_processes()
    ├─ 打印表头
    │   └─ "=== Running Processes (max 5) ==="
    │
    ├─ 遍历进程表
    │   └─ for (i = 0; i < MAX_PROCESSES; i++)
    │       └─ if (process_table[i].status == 1)
    │           └─ 打印进程信息
    │               ├─ PID
    │               ├─ System PID
    │               ├─ Name
    │               └─ Status ("Running")
    │
    └─ 打印统计信息
        └─ "Total: X process(es) running"
```

### 4. 进程停止流程（`stop`命令）

```
用户输入: stop 1
    ↓
[1] commands.c: execute_stop()
    ├─ 验证进程ID格式
    └─ 调用: stop_process(1)
        ↓
[2] process.c: stop_process(pid)
    ├─ [2.1] 查找进程
    │   └─ for (i = 0; i < MAX_PROCESSES; i++)
    │       └─ if (process_table[i].pid == pid && status == 1)
    │
    ├─ [2.2] 检查进程是否存在
    │   └─ kill(system_pid, 0)
    │       ├─ 如果进程不存在 → 更新状态为0，返回
    │       └─ 如果进程存在 → 继续
    │
    ├─ [2.3] 发送终止信号
    │   └─ kill(system_pid, SIGTERM)
    │
    ├─ [2.4] 等待进程结束
    │   └─ waitpid(system_pid, &status, 0)
    │
    ├─ [2.5] 更新进程状态
    │   └─ process_table[i].status = 0
    │
    └─ [2.6] 打印成功消息
        └─ "Process 1 stopped successfully"
```

### 5. 进程表数据结构

```
进程表（数组，最多5个）:
┌─────┬─────────────┬──────────────┬──────────────┬────────┐
│ 槽位 │ NeuMiniOS PID│ System PID  │ Name         │ Status │
├─────┼─────────────┼──────────────┼──────────────┼────────┤
│  0  │      1       │    12345     │ helloworld   │   1    │ ← 运行中
│  1  │      2       │    12346     │ helloworld   │   1    │ ← 运行中
│  2  │      0       │       0      │              │   0    │ ← 空闲
│  3  │      0       │       0      │              │   0    │ ← 空闲
│  4  │      0       │       0      │              │   0    │ ← 空闲
└─────┴─────────────┴──────────────┴──────────────┴────────┘
```

### 6. 关键函数详解

#### create_process() 完整流程
```c
int create_process(const char *program_name, const char *program_path) {
    // 步骤1: 查找空闲槽位
    int slot = -1;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].status == 0) {
            slot = i;
            break;
        }
    }
    if (slot == -1) {
        printf("[ERROR] Process table full\n");
        return -1;
    }

    // 步骤2: 读取文件到内存
    size_t size;
    unsigned char *data = read_file(program_path, &size);
    if (!data) return -1;

    // 步骤3: 创建临时文件
    char temp_path[] = "/tmp/neumini_XXXXXX";
    int fd = mkstemp(temp_path);
    close(fd);

    // 步骤4: 写入文件并设置权限
    write_file(temp_path, data, size);
    free(data);

    // 步骤5: 创建子进程
    pid_t system_pid = fork();
    if (system_pid == 0) {
        // 子进程：执行程序
        execl(temp_path, program_name, (char *)NULL);
        _exit(1);
    } else if (system_pid > 0) {
        // 父进程：记录信息
        process_table[slot].pid = next_pid++;
        process_table[slot].system_pid = system_pid;
        strcpy(process_table[slot].name, program_name);
        process_table[slot].status = 1;
        return process_table[slot].pid;
    }
    return -1;
}
```

#### stop_process() 完整流程
```c
int stop_process(int pid) {
    // 步骤1: 查找进程
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].pid == pid && process_table[i].status == 1) {
            
            // 步骤2: 检查进程是否存在
            if (kill(process_table[i].system_pid, 0) != 0) {
                // 进程已不存在，更新状态
                process_table[i].status = 0;
                return 0;
            }
            
            // 步骤3: 发送终止信号
            if (kill(process_table[i].system_pid, SIGTERM) == 0) {
                // 步骤4: 等待进程结束
                int status;
                waitpid(process_table[i].system_pid, &status, 0);
                
                // 步骤5: 更新状态
                process_table[i].status = 0;
                printf("Process %d stopped successfully\n", pid);
                return 0;
            }
        }
    }
    printf("Process %d not found\n", pid);
    return -1;
}
```

---

## 💻 实际代码详解

### 1. 头文件 (process_manager.h)

#### 完整代码
```1:27:include/process_manager.h
#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

#include <sys/types.h>

#define MAX_PROCESSES 5
#define MAX_PROCESS_NAME 256

// 淇：进程信息结构（使用数组方式）
typedef struct {
    int pid;                    // NeuMiniOS 进程 ID（唯一）
    pid_t system_pid;           // Linux 系统进程 ID
    int status;                 // 进程状态（0=空闲, 1=运行中）
    char name[MAX_PROCESS_NAME]; // 进程名称
} Process;

// 淇：进程管理函数声明
void init_process_table(void);
int create_process(const char* program_name, const char* program_path);
int stop_process(int pid);
void list_processes(void);
void cleanup_process_table(void);

// 淇：兼容性：ProcessManager类型定义（用于接口兼容）
typedef void ProcessManager;

#endif // PROCESS_MANAGER_H
```

#### 代码解释

**头文件保护**
```c
#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H
// ...
#endif
```
- **作用**：防止头文件被重复包含
- **原理**：第一次包含时定义`PROCESS_MANAGER_H`，后续包含会被跳过

**常量定义**
```c
#define MAX_PROCESSES 5
#define MAX_PROCESS_NAME 256
```
- `MAX_PROCESSES`：最多管理5个进程（基本要求）
- `MAX_PROCESS_NAME`：进程名称最大长度256字符

**Process结构体**
```c
typedef struct {
    int pid;                    // NeuMiniOS自定义的进程ID（1, 2, 3...）
    pid_t system_pid;           // Linux系统分配的真实进程ID
    int status;                 // 0=空闲槽位, 1=运行中
    char name[MAX_PROCESS_NAME]; // 进程名称（如"helloworld"）
} Process;
```
- **pid**：你自己分配的进程编号，从1开始递增
- **system_pid**：Linux系统给的进程ID，用于实际控制进程
- **status**：标记进程表槽位是否被使用
- **name**：存储进程名称，用于显示

---

### 2. 实现文件 (process.c) - 全局变量

#### 代码
```12:13:src/process.c
static Process process_table[MAX_PROCESSES];
static int next_pid = 1;
```

#### 解释
- **`process_table`**：静态数组，存储最多5个进程的信息
  - `static`关键字：只在当前文件内可见，外部无法直接访问
  - 这是进程管理的核心数据结构
  
- **`next_pid`**：用于生成唯一的NeuMiniOS进程ID
  - 初始值为1
  - 每次创建新进程时递增（`next_pid++`）
  - 确保每个进程都有唯一的PID

---

### 3. 初始化函数 (init_process_table)

#### 代码
```15:23:src/process.c
// 初始化进程表
void init_process_table(void) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        process_table[i].pid = 0;
        process_table[i].system_pid = 0;
        process_table[i].status = 0;
        strcpy(process_table[i].name, "");
    }
}
```

#### 详细解释
- **作用**：系统启动时清空进程表，将所有槽位标记为空闲
- **执行时机**：在`neuboot()`函数中系统启动时调用
- **初始化内容**：
  - `pid = 0`：表示槽位未被使用
  - `system_pid = 0`：没有系统进程ID
  - `status = 0`：标记为空闲状态
  - `name = ""`：清空进程名称

---

### 4. 文件读取函数 (read_file)

#### 代码
```25:43:src/process.c
// 读取二进制文件到内存（用于从Disk Image中提取）
unsigned char* read_file(const char *path, size_t *out_size) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    *out_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    unsigned char *buffer = malloc(*out_size);
    if (!buffer) {
        fclose(f);
        return NULL;
    }

    fread(buffer, 1, *out_size, f);
    fclose(f);
    return buffer;
}
```

#### 详细解释

**函数签名**
- **参数**：
  - `path`：文件路径（如`/tmp/neuminios_helloworld_1234`）
  - `out_size`：输出参数，返回文件大小
- **返回值**：指向文件内容的指针，失败返回`NULL`

**执行步骤**

1. **打开文件**
   ```c
   FILE *f = fopen(path, "rb");
   ```
   - `"rb"`：以二进制只读模式打开
   - 失败时返回`NULL`

2. **获取文件大小**
   ```c
   fseek(f, 0, SEEK_END);  // 移动到文件末尾
   *out_size = ftell(f);    // 获取当前位置（即文件大小）
   fseek(f, 0, SEEK_SET);  // 移回文件开头
   ```
   - 先移到末尾获取大小，再移回开头准备读取

3. **分配内存**
   ```c
   unsigned char *buffer = malloc(*out_size);
   ```
   - 动态分配内存存储文件内容
   - 使用`unsigned char`因为二进制文件可能包含任意字节

4. **读取文件**
   ```c
   fread(buffer, 1, *out_size, f);
   ```
   - 一次性读取整个文件到内存

5. **关闭文件并返回**
   ```c
   fclose(f);
   return buffer;
   ```
   - 记得关闭文件释放资源
   - 返回指向文件内容的指针（调用者需要负责`free()`）

---

### 5. 文件写入函数 (write_file)

#### 代码
```45:56:src/process.c
// 将内存中的二进制文件写入临时文件并设为可执行
int write_file(const char *path, unsigned char *data, size_t size) {
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) return -1;

    write(fd, data, size);
    close(fd);

    // 设为可执行
    chmod(path, 0755);
    return 0;
}
```

#### 详细解释

**函数作用**：将内存中的二进制数据写入文件，并设置可执行权限

**执行步骤**

1. **打开/创建文件**
   ```c
   int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
   ```
   - `O_WRONLY`：只写模式
   - `O_CREAT`：如果文件不存在则创建
   - `O_TRUNC`：如果文件存在则清空
   - `0644`：文件权限（所有者可读写，其他人只读）

2. **写入数据**
   ```c
   write(fd, data, size);
   ```
   - 将内存中的数据写入文件
   - `fd`：文件描述符
   - `data`：要写入的数据
   - `size`：数据大小

3. **关闭文件**
   ```c
   close(fd);
   ```

4. **设置可执行权限**
   ```c
   chmod(path, 0755);
   ```
   - `0755`：所有者可读可写可执行，其他人可读可执行
   - 这是可执行文件必需的权限

---

### 6. 核心函数：创建进程 (create_process)

#### 完整代码
```74:136:src/process.c
// 创建新进程（run命令）
int create_process(const char *program_name, const char *program_path) {
    // 1. 查找空闲槽位
    int slot = -1;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].status == 0) {
            slot = i;
            break;
        }
    }

    if (slot == -1) {
        printf("[ERROR] Process table full (max %d processes)\n", MAX_PROCESSES);
        return -1;
    }

    // 2. 读取二进制文件
    size_t size;
    unsigned char *data = read_file(program_path, &size);
    if (!data) {
        printf("[ERROR] Could not read program: %s\n", program_path);
        return -1;
    }

    // 3. 写入临时文件
    char temp_path[] = "/tmp/neumini_XXXXXX";
    int fd = mkstemp(temp_path);
    if (fd == -1) {
        free(data);
        return -1;
    }
    close(fd);

    if (write_file(temp_path, data, size) != 0) {
        free(data);
        return -1;
    }

    free(data);

    // 4. 使用fork和execl运行
    pid_t system_pid = fork();
    if (system_pid == 0) {
        // 子进程
        execl(temp_path, program_name, (char *)NULL);
        perror("[ERROR] execl failed");
        _exit(1);
    } else if (system_pid > 0) {
        // 父进程：记录进程信息
        process_table[slot].pid = next_pid++;
        process_table[slot].system_pid = system_pid;
        strcpy(process_table[slot].name, program_name);
        process_table[slot].status = 1;

        printf("[OK] Process %d started (NeuMiniOS PID: %d, System PID: %d)\n",
               process_table[slot].pid, process_table[slot].pid, system_pid);

        return process_table[slot].pid;
    } else {
        perror("[ERROR] fork failed");
        return -1;
    }
}
```

#### 逐步详细解释

**步骤1：查找空闲槽位**
```c
int slot = -1;
for (int i = 0; i < MAX_PROCESSES; i++) {
    if (process_table[i].status == 0) {
        slot = i;
        break;
    }
}
```
- **目的**：找到进程表中第一个空闲的槽位
- **逻辑**：遍历数组，找到`status == 0`的槽位
- **结果**：`slot`存储槽位索引，如果为-1表示进程表已满

**步骤2：读取文件到内存**
```c
size_t size;
unsigned char *data = read_file(program_path, &size);
if (!data) {
    printf("[ERROR] Could not read program: %s\n", program_path);
    return -1;
}
```
- **目的**：将可执行文件读取到内存
- **注意**：`program_path`是文件系统提取后的临时文件路径
- **错误处理**：如果读取失败，返回错误

**步骤3：创建新的临时文件**
```c
char temp_path[] = "/tmp/neumini_XXXXXX";
int fd = mkstemp(temp_path);
if (fd == -1) {
    free(data);
    return -1;
}
close(fd);
```
- **`mkstemp()`**：创建唯一的临时文件
  - `XXXXXX`会被替换为随机字符（如`/tmp/neumini_a3b2c1`）
  - 返回文件描述符
- **为什么需要新临时文件**：
  - 原文件可能被其他进程使用
  - 需要设置可执行权限
  - 确保文件唯一性

**步骤4：写入文件并设置权限**
```c
if (write_file(temp_path, data, size) != 0) {
    free(data);
    return -1;
}
free(data);
```
- 将内存中的数据写入临时文件
- 设置可执行权限
- 释放内存（重要！避免内存泄漏）

**步骤5：创建子进程（fork）**
```c
pid_t system_pid = fork();
```
- **fork()的作用**：创建一个子进程
- **返回值**：
  - `0`：在子进程中
  - `> 0`：在父进程中（返回值是子进程的PID）
  - `-1`：创建失败

**步骤6：子进程执行程序**
```c
if (system_pid == 0) {
    // 子进程
    execl(temp_path, program_name, (char *)NULL);
    perror("[ERROR] execl failed");
    _exit(1);
}
```
- **execl()的作用**：用新程序替换当前进程的代码
  - 第一个参数：程序路径
  - 第二个参数：程序名称（argv[0]）
  - `(char *)NULL`：参数列表结束
- **如果execl成功**：不会返回（进程被替换）
- **如果execl失败**：打印错误并退出
- **`_exit(1)`**：立即退出，不执行清理（因为进程即将被替换）

**步骤7：父进程记录信息**
```c
else if (system_pid > 0) {
    // 父进程：记录进程信息
    process_table[slot].pid = next_pid++;
    process_table[slot].system_pid = system_pid;
    strcpy(process_table[slot].name, program_name);
    process_table[slot].status = 1;
    
    printf("[OK] Process %d started ...\n", ...);
    return process_table[slot].pid;
}
```
- **分配NeuMiniOS PID**：`next_pid++`（1, 2, 3...）
- **保存系统PID**：Linux系统给的进程ID
- **保存进程名称**：用于显示
- **标记为运行中**：`status = 1`
- **返回进程ID**：供调用者使用

---

### 7. 核心函数：停止进程 (stop_process)

#### 完整代码
```138:174:src/process.c
// 淇：停止进程（stop命令）
int stop_process(int pid) {
    if (pid <= 0) {
        printf("Error: Invalid process ID: %d\n", pid);
        return -1;
    }
    
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].pid == pid && process_table[i].status == 1) {
            // 淇：检查进程是否仍然存在
            if (kill(process_table[i].system_pid, 0) != 0) {
                // 淇：进程已经不存在，更新状态
                process_table[i].status = 0;
                printf("Warning: Process %d (system PID %d) is no longer running\n", 
                       pid, (int)process_table[i].system_pid);
                return 0;
            }
            
            // 淇：发送终止信号
            if (kill(process_table[i].system_pid, SIGTERM) == 0) {
                // 淇：等待进程结束
                int status;
                waitpid(process_table[i].system_pid, &status, 0);
                process_table[i].status = 0;
                printf("Process %d (%s) stopped successfully\n", pid, process_table[i].name);
                return 0;
            } else {
                perror("Error: Failed to stop process");
                return -1;
            }
        }
    }
    
    printf("Error: Process %d not found or not running\n", pid);
    printf("Use 'plist' to see running processes\n");
    return -1;
}
```

#### 逐步详细解释

**步骤1：参数验证**
```c
if (pid <= 0) {
    printf("Error: Invalid process ID: %d\n", pid);
    return -1;
}
```
- 检查PID是否有效（必须是正整数）

**步骤2：查找进程**
```c
for (int i = 0; i < MAX_PROCESSES; i++) {
    if (process_table[i].pid == pid && process_table[i].status == 1) {
        // 找到进程
    }
}
```
- 遍历进程表，查找匹配的PID
- 必须同时满足：`pid`匹配 且 `status == 1`（运行中）

**步骤3：检查进程是否存在**
```c
if (kill(process_table[i].system_pid, 0) != 0) {
    // 进程已经不存在
    process_table[i].status = 0;
    return 0;
}
```
- **`kill(pid, 0)`的特殊用法**：
  - 信号0不发送任何信号
  - 只检查进程是否存在
  - 如果进程不存在，返回非0
- **处理僵尸进程**：如果进程已退出但状态未更新，自动清理

**步骤4：发送终止信号**
```c
if (kill(process_table[i].system_pid, SIGTERM) == 0) {
    // 成功发送信号
}
```
- **SIGTERM信号**：请求进程正常终止
  - 进程可以捕获此信号并执行清理
  - 比SIGKILL更温和（SIGKILL无法被捕获）
- **返回值**：0表示成功，-1表示失败

**步骤5：等待进程结束**
```c
int status;
waitpid(process_table[i].system_pid, &status, 0);
```
- **waitpid()的作用**：
  - 等待指定进程结束
  - 回收子进程资源，避免僵尸进程
  - `&status`：存储进程退出状态
  - `0`：阻塞等待，直到进程结束

**步骤6：更新状态**
```c
process_table[i].status = 0;
printf("Process %d (%s) stopped successfully\n", pid, process_table[i].name);
return 0;
```
- 将槽位标记为空闲
- 打印成功消息
- 返回0表示成功

**错误处理**
```c
printf("Error: Process %d not found or not running\n", pid);
printf("Use 'plist' to see running processes\n");
return -1;
```
- 如果找不到进程，提示用户使用`plist`查看

---

### 8. 核心函数：列出进程 (list_processes)

#### 完整代码
```176:200:src/process.c
// 淇：列出所有进程（plist命令）
void list_processes(void) {
    int running_count = 0;
    
    printf("=== Running Processes (max %d) ===\n", MAX_PROCESSES);
    printf("%-10s %-10s %-20s %s\n", "PID", "System PID", "Name", "Status");
    printf("------------------------------------------------\n");
    
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].status == 1) {
            printf("%-10d %-10d %-20s %s\n",
                   process_table[i].pid,
                   (int)process_table[i].system_pid,
                   process_table[i].name,
                   "Running");
            running_count++;
        }
    }
    
    if (running_count == 0) {
        printf("(no running processes)\n");
    } else {
        printf("Total: %d process(es) running\n", running_count);
    }
}
```

#### 详细解释

**步骤1：打印表头**
```c
printf("=== Running Processes (max %d) ===\n", MAX_PROCESSES);
printf("%-10s %-10s %-20s %s\n", "PID", "System PID", "Name", "Status");
printf("------------------------------------------------\n");
```
- **格式化输出**：
  - `%-10s`：左对齐，宽度10字符
  - 创建整齐的表格格式

**步骤2：遍历进程表**
```c
for (int i = 0; i < MAX_PROCESSES; i++) {
    if (process_table[i].status == 1) {
        // 打印进程信息
        running_count++;
    }
}
```
- 只显示`status == 1`的进程（运行中）
- 统计运行中的进程数量

**步骤3：打印进程信息**
```c
printf("%-10d %-10d %-20s %s\n",
       process_table[i].pid,
       (int)process_table[i].system_pid,
       process_table[i].name,
       "Running");
```
- 显示NeuMiniOS PID、系统PID、进程名称、状态

**步骤4：打印统计信息**
```c
if (running_count == 0) {
    printf("(no running processes)\n");
} else {
    printf("Total: %d process(es) running\n", running_count);
}
```
- 如果没有进程，显示提示信息
- 否则显示总数

---

### 9. 清理函数 (cleanup_process_table)

#### 完整代码
```202:211:src/process.c
// 清理进程表（退出时调用）
void cleanup_process_table(void) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].status == 1) {
            kill(process_table[i].system_pid, SIGKILL);
            waitpid(process_table[i].system_pid, NULL, 0);
        }
    }
    printf("[INFO] All processes cleaned up\n");
}
```

#### 详细解释

**作用**：系统退出时强制终止所有进程

**执行步骤**

1. **遍历进程表**
   ```c
   for (int i = 0; i < MAX_PROCESSES; i++) {
       if (process_table[i].status == 1) {
           // 终止进程
       }
   }
   ```

2. **发送SIGKILL信号**
   ```c
   kill(process_table[i].system_pid, SIGKILL);
   ```
   - **SIGKILL**：强制终止信号，无法被捕获或忽略
   - 确保所有进程都被终止

3. **等待进程结束**
   ```c
   waitpid(process_table[i].system_pid, NULL, 0);
   ```
   - 回收子进程资源
   - `NULL`：不关心退出状态

**为什么使用SIGKILL而不是SIGTERM？**
- 系统退出时，需要立即终止所有进程
- SIGKILL无法被捕获，确保进程一定会终止
- 避免进程卡住导致系统无法退出

---

### 10. 命令系统调用示例

#### execute_run() 函数
```99:124:src/commands.c
// 执行 run 命令
int execute_run(FileSystem* fs, ProcessManager* pm, const char* filename) {
    (void)pm;  // 不再需要pm参数，但保持接口兼容
    
    if (!fs || !filename) {
        printf("Usage: run <filename>\n");
        return -1;
    }
    
    // 提取文件到临时位置
    char temp_path[256];
    snprintf(temp_path, sizeof(temp_path), "/tmp/neuminios_%s_%d", filename, getpid());
    
    if (extract_file_to_host(fs, filename, temp_path) != 0) {
        printf("Error: Failed to extract file '%s'\n", filename);
        return -1;
    }
    
    // 使用新的create_process函数，它会处理文件读取、临时文件创建和进程启动
    int process_id = create_process(filename, temp_path);
    if (process_id > 0) {
        return 0;
    } else {
        return -1;
    }
}
```

**解释**：
1. **参数验证**：检查文件系统和文件名是否有效
2. **提取文件**：调用文件系统模块的`extract_file_to_host()`
3. **创建进程**：调用你的`create_process()`函数
4. **返回结果**：成功返回0，失败返回-1

#### execute_plist() 函数
```77:85:src/commands.c
// 淇：执行 plist 命令
int execute_plist(ProcessManager* pm) {
    (void)pm;  // 淇：不再需要pm参数，但保持接口兼容
    
    // 淇：确保进程表已初始化
    // 注意：init_process_table()应该在neuboot中调用，这里只是安全检查
    list_processes();
    return 0;
}
```

**解释**：
- 直接调用`list_processes()`显示进程列表
- `(void)pm`：忽略未使用的参数（保持接口兼容）

#### execute_stop() 函数
```87:97:src/commands.c
// 淇：执行 stop 命令
int execute_stop(ProcessManager* pm, int process_id) {
    (void)pm;  // 淇：不再需要pm参数，但保持接口兼容
    
    if (process_id <= 0) {
        printf("Error: Invalid process ID. Process ID must be a positive integer.\n");
        return -1;
    }
    
    return stop_process(process_id);
}
```

**解释**：
1. **参数验证**：检查进程ID是否有效
2. **调用停止函数**：调用你的`stop_process()`函数
3. **返回结果**：返回停止操作的结果

---

## 🎓 学习要点总结

### 核心概念
1. **进程**：正在运行的程序实例
2. **进程表**：操作系统用来跟踪所有进程的数据结构
3. **进程ID**：每个进程的唯一标识符
4. **进程状态**：运行中、空闲、已终止等

### 关键技术
1. **fork()**：创建子进程
2. **execl()**：执行程序
3. **kill()**：发送信号给进程
4. **waitpid()**：等待进程结束

### 数据结构
- 使用**链表**管理进程（加分项已实现）
- 链表节点包含：PID、系统PID、名称、状态、`next`指针

#### 数组 vs 链表（本项目实现对比）
- **容量限制**：数组固定上限`MAX_PROCESSES=5`，链表逻辑上可扩展，但仍保留5的上限以符合课程要求。
- **插入删除**：数组需寻找空槽位，删除通常置空槽；链表采用尾插、删除时直接摘链并`free`，避免碎片空洞。
- **遍历成本**：两者都是 O(n)，但链表无需关心空槽判断，逻辑更直接。
- **内存占用**：数组一次性分配固定大小；链表按需`malloc`，在进程结束后释放节点。
- **实现复杂度**：数组简单、易于调试；链表需要前驱指针管理，但更贴近真实 OS 进程链表设计。

### 工作流程
1. **创建**：查找空闲槽位 → 读取文件 → 创建临时文件 → fork → execl
2. **列表**：遍历进程表 → 显示运行中的进程
3. **停止**：查找进程 → 检查存在性 → 发送信号 → 等待结束 → 更新状态

---

## 📁 进程管理相关文件位置与执行流程

### 1. 进程管理相关文件清单

#### 核心文件（你负责的文件）
| 文件路径 | 作用 | 关键内容 |
|---------|------|---------|
| `include/process_manager.h` | 进程管理头文件 | Process结构体定义、函数声明 |
| `src/process.c` | 进程管理实现文件 | 所有进程管理功能的实现 |

#### 调用进程管理的文件（其他同学负责）
| 文件路径 | 作用 | 调用关系 |
|---------|------|---------|
| `src/main.c` | 程序入口 | 调用`neuboot_start()` |
| `src/neuboot.c` | 系统启动引导 | 调用`init_process_table()`和`cleanup_process_table()` |
| `src/cli.c` | 命令行界面 | 调用`execute_command()` |
| `src/commands.c` | 命令执行系统 | 调用`create_process()`, `list_processes()`, `stop_process()` |
| `include/commands.h` | 命令系统头文件 | 声明`execute_run()`, `execute_plist()`, `execute_stop()` |

### 2. 进程管理代码在文件中的具体位置

#### `include/process_manager.h` - 头文件
```9:16:include/process_manager.h
typedef struct Process {
    int pid;
    pid_t system_pid;
    int status;
    char name[MAX_PROCESS_NAME];
    struct Process* next;
} Process;
```

```18:22:include/process_manager.h
void init_process_table(void);
int create_process(const char* program_name, const char* program_path);
int stop_process(int pid);
void list_processes(void);
void cleanup_process_table(void);
```

#### `src/process.c` - 实现文件
- **全局变量**（第12-14行）：
  - `process_list`: 链表头指针
  - `process_count`: 进程计数
  - `next_pid`: 下一个PID

- **辅助函数**（第16-30行）：
  - `find_process()`: 在链表中查找进程

- **初始化函数**（第32-44行）：
  - `init_process_table()`: 初始化进程表

- **文件操作函数**（第46-77行）：
  - `read_file()`: 读取二进制文件
  - `write_file()`: 写入文件并设置权限
  - `run_program()`: 运行程序（辅助函数）

- **核心功能函数**（第95-254行）：
  - `create_process()`: 创建新进程（第95-167行）
  - `stop_process()`: 停止进程（第169-211行）
  - `list_processes()`: 列出所有进程（第213-237行）
  - `cleanup_process_table()`: 清理进程表（第239-254行）

#### `src/commands.c` - 命令执行
- **`execute_run()`**（第100-124行）：
  - 调用文件系统提取文件
  - 调用`create_process()`创建进程

- **`execute_plist()`**（第78-85行）：
  - 直接调用`list_processes()`

- **`execute_stop()`**（第88-97行）：
  - 验证进程ID
  - 调用`stop_process()`

- **`execute_command()`**（第160-279行）：
  - 命令分发函数
  - 根据命令类型调用相应的`execute_*`函数

#### `src/neuboot.c` - 系统启动
- **`neuboot_start()`**（第13-58行）：
  - 第26行：调用`init_process_table()`初始化进程表
  - 第50行：启动CLI循环
  - 第55行：调用`cleanup_process_table()`清理进程

#### `src/cli.c` - 命令行界面
- **`cli_loop()`**（第54-97行）：
  - 第64行：读取用户输入
  - 第76行：解析命令
  - 第79行：调用`execute_command()`执行命令

#### `src/main.c` - 程序入口
- **`main()`**（第4-10行）：
  - 第8行：调用`neuboot_start()`启动系统

### 3. 进程管理执行流程详解

#### 3.1 系统启动流程（进程管理初始化）

```
程序启动
    ↓
main.c: main()
    ↓
neuboot.c: neuboot_start()
    ├─ 初始化文件系统
    ├─ [关键] init_process_table()  ← process.c:32
    │   └─ 清空进程链表
    │   └─ 重置 process_count = 0
    │   └─ 重置 next_pid = 1
    ├─ 加载文件到磁盘镜像
    └─ cli.c: cli_loop()  ← 进入命令行循环
```

**代码位置**：
- `src/main.c:8` → `src/neuboot.c:26` → `src/process.c:33`

#### 3.2 `run`命令执行流程（创建进程）

```
用户输入: run helloworld
    ↓
cli.c: cli_loop()
    ├─ read_input()          ← 读取用户输入
    ├─ parse_command()        ← 解析命令
    └─ execute_command()      ← 分发命令
        ↓
commands.c: execute_command()
    ├─ 识别命令为 "run"
    └─ execute_run(fs, pm, "helloworld")
        ↓
commands.c: execute_run()  (第100-124行)
    ├─ [步骤1] 构建临时文件路径
    │   └─ "/tmp/neuminios_helloworld_XXXX"
    ├─ [步骤2] 调用文件系统提取文件
    │   └─ extract_file_to_host(fs, "helloworld", temp_path)
    │       ← 文件系统模块（同学C负责）
    └─ [步骤3] 调用进程管理创建进程
        └─ create_process("helloworld", temp_path)
            ↓
process.c: create_process()  (第95-167行)
    ├─ [步骤1] 检查进程数量限制
    │   └─ if (process_count >= MAX_PROCESSES)
    ├─ [步骤2] 读取二进制文件
    │   └─ read_file(program_path, &size)  ← 第46-64行
    ├─ [步骤3] 创建临时文件
    │   └─ mkstemp("/tmp/neumini_XXXXXX")
    ├─ [步骤4] 写入文件并设置权限
    │   └─ write_file(temp_path, data, size)  ← 第66-77行
    ├─ [步骤5] 创建子进程
    │   └─ fork()
    │       ├─ 子进程: execl(temp_path, "helloworld", NULL)
    │       └─ 父进程: 继续执行
    └─ [步骤6] 父进程记录信息
        ├─ malloc(sizeof(Process))  ← 创建链表节点
        ├─ 设置进程信息（pid, system_pid, name, status）
        ├─ 追加到链表尾部
        └─ process_count++
```

**代码位置**：
- `src/cli.c:79` → `src/commands.c:216` → `src/commands.c:100` → `src/process.c:96`

#### 3.3 `plist`命令执行流程（列出进程）

```
用户输入: plist
    ↓
cli.c: cli_loop()
    ├─ read_input()
    ├─ parse_command()
    └─ execute_command()
        ↓
commands.c: execute_command()
    ├─ 识别命令为 "plist"
    └─ execute_plist(pm)
        ↓
commands.c: execute_plist()  (第78-85行)
    └─ list_processes()
        ↓
process.c: list_processes()  (第213-237行)
    ├─ 打印表头
    ├─ 遍历进程链表
    │   └─ for (Process* curr = process_list; curr; curr = curr->next)
    │       └─ if (curr->status == 1)
    │           └─ 打印进程信息
    └─ 打印统计信息
```

**代码位置**：
- `src/cli.c:79` → `src/commands.c:194` → `src/commands.c:78` → `src/process.c:214`

#### 3.4 `stop`命令执行流程（停止进程）

```
用户输入: stop 1
    ↓
cli.c: cli_loop()
    ├─ read_input()
    ├─ parse_command()
    └─ execute_command()
        ↓
commands.c: execute_command()
    ├─ 识别命令为 "stop"
    ├─ 验证进程ID格式（第203-213行）
    └─ execute_stop(pm, 1)
        ↓
commands.c: execute_stop()  (第88-97行)
    ├─ 验证进程ID有效性
    └─ stop_process(1)
        ↓
process.c: stop_process()  (第169-211行)
    ├─ [步骤1] 查找进程
    │   └─ find_process(pid, &prev)  ← 第16-30行
    │       └─ 遍历链表查找匹配的PID
    ├─ [步骤2] 检查进程是否存在
    │   └─ kill(system_pid, 0)
    │       └─ 如果不存在，从链表移除节点并返回
    ├─ [步骤3] 发送终止信号
    │   └─ kill(system_pid, SIGTERM)
    ├─ [步骤4] 等待进程结束
    │   └─ waitpid(system_pid, &status, 0)
    └─ [步骤5] 从链表移除并释放
        ├─ 更新链表指针
        ├─ free(node)
        └─ process_count--
```

**代码位置**：
- `src/cli.c:79` → `src/commands.c:197` → `src/commands.c:88` → `src/process.c:170`

#### 3.5 系统退出流程（进程清理）

```
用户输入: exit
    ↓
cli.c: cli_loop()
    ├─ execute_command()
    └─ 返回 -2（退出标志）
        ↓
cli_loop() 退出循环
    ↓
neuboot.c: neuboot_start()
    └─ cleanup_process_table()
        ↓
process.c: cleanup_process_table()  (第239-254行)
    ├─ 遍历进程链表
    │   └─ while (curr)
    │       ├─ 如果进程运行中
    │       │   ├─ kill(system_pid, SIGKILL)  ← 强制终止
    │       │   └─ waitpid(system_pid, NULL, 0)  ← 等待结束
    │       ├─ 保存下一个节点指针
    │       └─ free(curr)  ← 释放节点内存
    ├─ 重置链表头指针
    └─ 重置计数器
```

**代码位置**：
- `src/cli.c:80` → `src/neuboot.c:55` → `src/process.c:240`

### 4. 进程管理函数调用关系图

```
系统启动阶段:
main.c:main()
  └─ neuboot.c:neuboot_start()
      └─ process.c:init_process_table()  ← 初始化

命令执行阶段:
cli.c:cli_loop()
  └─ commands.c:execute_command()
      ├─ commands.c:execute_run()
      │   └─ process.c:create_process()  ← 创建进程
      ├─ commands.c:execute_plist()
      │   └─ process.c:list_processes()  ← 列出进程
      └─ commands.c:execute_stop()
          └─ process.c:stop_process()  ← 停止进程

系统退出阶段:
neuboot.c:neuboot_start()
  └─ process.c:cleanup_process_table()  ← 清理所有进程
```

### 5. 关键函数调用顺序总结

#### 创建进程（run命令）
1. `cli_loop()` → 读取输入
2. `parse_command()` → 解析命令
3. `execute_command()` → 分发命令
4. `execute_run()` → 提取文件
5. `create_process()` → 创建进程
   - `read_file()` → 读取文件
   - `write_file()` → 写入临时文件
   - `fork()` → 创建子进程
   - `execl()` → 执行程序（子进程）

#### 列出进程（plist命令）
1. `cli_loop()` → 读取输入
2. `parse_command()` → 解析命令
3. `execute_command()` → 分发命令
4. `execute_plist()` → 调用列表函数
5. `list_processes()` → 遍历链表并显示

#### 停止进程（stop命令）
1. `cli_loop()` → 读取输入
2. `parse_command()` → 解析命令
3. `execute_command()` → 分发命令
4. `execute_stop()` → 验证参数
5. `stop_process()` → 停止进程
   - `find_process()` → 查找进程
   - `kill()` → 发送信号
   - `waitpid()` → 等待结束

---

## 📝 代码位置参考

### 你的核心文件
- **头文件**: `include/process_manager.h`
- **实现文件**: `src/process.c`

### 关键代码位置
- 进程链表定义: `src/process.c:12`
- PID生成: `src/process.c:14`
- 进程查找函数: `src/process.c:16-30`
- 进程创建: `src/process.c:95-167`
- 进程停止: `src/process.c:169-211`
- 进程列表: `src/process.c:213-237`
- 进程清理: `src/process.c:239-254`

### 接口调用位置
- `execute_run()`: `src/commands.c:100-124`
- `execute_plist()`: `src/commands.c:78-85`
- `execute_stop()`: `src/commands.c:88-97`
- `init_process_table()`: `src/neuboot.c:26`
- `cleanup_process_table()`: `src/neuboot.c:55`

---

## ✅ 检查清单

### 功能检查
- [ ] `run`命令能创建进程
- [ ] `plist`命令能显示进程列表
- [ ] `stop`命令能停止进程
- [ ] 进程表满时能正确处理
- [ ] 停止不存在的进程时能报错
- [ ] 系统退出时能清理所有进程

### 代码质量
- [ ] 代码有注释
- [ ] 错误处理完善
- [ ] 内存管理正确（malloc/free配对）
- [ ] 函数职责清晰

### 测试验证
- [ ] 能运行多个进程（最多5个）
- [ ] 每个进程有唯一PID
- [ ] 进程能正常执行并输出
- [ ] 进程能正常停止

---

**祝你学习顺利！** 🚀

如有问题，请参考：
- `进程管理测试指南.md` - 详细的测试步骤
- `完成度报告/ruby_进程管理模块实现报告.md` - 实现报告
- `指导文档.md` - 项目总体要求


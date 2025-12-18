# NeuMiniOS 操作系统项目

## 项目简介

NeuMiniOS 是一个运行在 Linux 主机操作系统之上的小型操作系统，使用 C 语言实现。本项目是操作系统课程的作业项目。

## 项目结构

```
neuminios/
├── include/              # 头文件目录
│   ├── cli.h            # CLI 相关定义
│   ├── process.h        # 进程管理相关定义
│   ├── file_system.h    # 文件系统相关定义
│   ├── commands.h       # 命令执行相关定义
│   └── neuboot.h        # 引导加载器相关定义
├── src/                 # 源文件目录
│   ├── main.c          # 主程序入口
│   ├── cli.c           # CLI 实现
│   ├── process.c       # 进程管理实现
│   ├── file_system.c   # 文件系统实现
│   ├── commands.c      # 命令执行实现
│   └── neuboot.c       # 引导加载器实现
├── neuminios_files/    # NeuMiniOS 文件目录（可执行文件和数据文件）
│   ├── helloworld.c    # Hello World 源代码
│   └── datafile.txt    # 测试数据文件
├── Makefile            # 编译配置文件
└── README.md          # 本文件
```

## 编译和运行

### 前置要求

- Linux 操作系统
- GCC 编译器
- Make 工具

### 编译步骤

1. **编译 Hello World 程序**（如果还没有编译）：
```bash
cd neuminios_files
gcc -o helloworld helloworld.c
cd ..
```

2. **编译 NeuMiniOS**：
```bash
make
```

或者手动编译：
```bash
gcc -Wall -Wextra -std=c11 -g -I./include -c src/main.c -o obj/main.o
gcc -Wall -Wextra -std=c11 -g -I./include -c src/cli.c -o obj/cli.o
gcc -Wall -Wextra -std=c11 -g -I./include -c src/process.c -o obj/process.o
gcc -Wall -Wextra -std=c11 -g -I./include -c src/file_system.c -o obj/file_system.o
gcc -Wall -Wextra -std=c11 -g -I./include -c src/commands.c -o obj/commands.o
gcc -Wall -Wextra -std=c11 -g -I./include -c src/neuboot.c -o obj/neuboot.o
gcc obj/*.o -o neuminios
```

3. **运行 NeuMiniOS**：
```bash
./neuminios
```

或者使用 Makefile：
```bash
make run
```

### 清理编译文件

```bash
make clean
```

## 使用方法

### 启动系统

运行 `./neuminios` 后，系统会：
1. 显示 NeuBoot 引导加载器启动信息
2. 从 `neuminios_files/` 目录加载所有文件到内存磁盘镜像
3. 显示启动信息和文件列表
4. 启动 CLI，显示 `>` 提示符

### 可用命令

| 命令 | 描述 | 示例 |
|------|------|------|
| `list` | 列出当前目录所有文件 | `> list` |
| `view <file>` | 查看文件内容 | `> view datafile.txt` |
| `delete <file>` | 删除文件 | `> delete datafile.txt` |
| `copy <src> <dest>` | 复制文件 | `> copy datafile.txt backup.txt` |
| `rename <old> <new>` | 重命名文件 | `> rename backup.txt newfile.txt` |
| `plist` | 列出所有运行进程 | `> plist` |
| `stop <pid>` | 停止进程 | `> stop 1` |
| `run <file>` | 运行可执行文件 | `> run helloworld` |
| `cd <dir>` | 切换目录（加分项） | `> cd mydir` |
| `mkdir <dir>` | 创建目录（加分项） | `> mkdir mydir` |
| `exit` | 退出系统 | `> exit` |

### 示例操作流程

```bash
# 1. 启动系统后，列出所有文件
> list

# 2. 查看数据文件内容
> view datafile.txt

# 3. 复制文件
> copy datafile.txt backup.txt

# 4. 运行 Hello World 程序
> run helloworld

# 5. 查看运行中的进程
> plist

# 6. 停止进程（假设进程 ID 是 1）
> stop 1

# 7. 列出文件验证操作
> list

# 8. 退出系统
> exit
```

## 开发说明

### 添加新文件到系统

1. 将可执行文件或数据文件放入 `neuminios_files/` 目录
2. 如果是可执行文件，确保已编译并具有执行权限
3. 重启 NeuMiniOS，文件会自动加载

### 添加新命令

1. 在 `include/commands.h` 中添加函数声明
2. 在 `src/commands.c` 中实现命令函数
3. 在 `execute_command()` 函数中添加命令分发逻辑

### 调试

项目使用 `-g` 标志编译，可以使用 GDB 调试：

```bash
gdb ./neuminios
```

## 功能特性

### 基本功能
- ✅ 命令行界面 (CLI)
- ✅ 进程管理（最多 5 个进程）
- ✅ 文件管理系统（内存磁盘镜像）
- ✅ NeuBoot 引导加载器
- ✅ 所有基本命令实现

### 加分功能（可选）
- ⭐ 命令历史记录
- ⭐ 目录层次结构（cd, mkdir）
- ⭐ 启动信息显示
- ⭐ 链表进程管理（替代数组）

## 注意事项

1. **文件权限**：确保 `neuminios_files/` 目录中的可执行文件具有执行权限
2. **临时文件**：`run` 命令会在 `/tmp/` 目录创建临时文件
3. **进程限制**：系统最多同时管理 5 个进程
4. **内存管理**：所有文件存储在内存中，系统关闭后数据会丢失
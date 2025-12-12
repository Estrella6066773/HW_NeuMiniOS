# NeuMiniOS Makefile

CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g -D_POSIX_C_SOURCE=200809L
INCLUDES = -I./include
SRCDIR = src
OBJDIR = obj
BINDIR = .

# 源文件
SOURCES = $(SRCDIR)/main.c \
          $(SRCDIR)/cli.c \
          $(SRCDIR)/process.c \
          $(SRCDIR)/file_system.c \
          $(SRCDIR)/commands.c \
          $(SRCDIR)/neuboot.c

# 目标文件
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

# 可执行文件
TARGET = neuminios

.PHONY: all clean directories

all: directories $(TARGET)

directories:
	@mkdir -p $(OBJDIR)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(BINDIR)/$(TARGET)
	@echo "Build complete: $(TARGET)"

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(OBJDIR) $(TARGET)
	@echo "Clean complete"

# 运行目标（需要先编译）
run: $(TARGET)
	./$(TARGET)

# 帮助信息
help:
	@echo "Available targets:"
	@echo "  make          - Build the project"
	@echo "  make clean    - Remove build files"
	@echo "  make run      - Build and run the project"
	@echo "  make help     - Show this help message"
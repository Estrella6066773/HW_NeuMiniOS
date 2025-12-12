#include "../include/neuboot.h"
#include <stdio.h>

int main(int argc, char* argv[]) {
    (void)argc;  // 未使用参数
    (void)argv;  // 未使用参数
    // 启动 NeuBoot 引导加载器
    neuboot_start();
    return 0;
}
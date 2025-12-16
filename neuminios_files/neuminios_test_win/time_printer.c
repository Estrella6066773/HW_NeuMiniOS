#include <stdio.h>
#include <time.h>
#include <unistd.h>
/*
 * 简单的周期打印脚本：每分钟输出当前时间。
 * 运行后使用 Ctrl+C 结束。
 */
int main(void) {
    while (1) {
        time_t now = time(NULL);
        struct tm* lt = localtime(&now);
        if (lt) {
            printf("[%04d-%02d-%02d %02d:%02d:%02d] tick\n",
                   lt->tm_year + 1900,
                   lt->tm_mon + 1,
                   lt->tm_mday,
                   lt->tm_hour,
                   lt->tm_min,
                   lt->tm_sec);
            fflush(stdout);
        } else {
            perror("localtime");
            return 1;
        }
        sleep(60);
    }
    return 0;
}


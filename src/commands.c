#include <stdio.h>
#include "commands.h"

void cmd_chown(const char *owner, const char *path) {
    printf("[chown] owner=%s, path=%s\n", owner, path);
}

void cmd_grep(const char *keyword, const char *path, int showLineNumber) {
    printf("[grep] keyword=%s, path=%s, -n=%d\n", keyword, path, showLineNumber);
}

void cmd_mv(const char *srcPath, const char *destPath) {
    printf("[mv] src=%s, dest=%s\n", srcPath, destPath);
}

void cmd_rm(const char *path, int recursive, int force) {
    printf("[rm] path=%s, recursive=%d, force=%d\n", path, recursive, force);
}
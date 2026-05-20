#ifndef COMMANDS_H
#define COMMANDS_H

void cmd_chown(const char *owner, const char *path);
void cmd_grep(const char *keyword, const char *path, int showLineNumber);
void cmd_mv(const char *srcPath, const char *destPath);
void cmd_rm(const char *path, int recursive, int force);

#endif
#ifndef COMMANDS_H
#define COMMANDS_H

/* Juheon */
void cmd_cd(int argc, char *argv[]);
void cmd_ls(int argc, char *argv[]);
void cmd_mkdir(int argc, char *argv[]);
void cmd_cat(int argc, char *argv[]);

/* Jihwan */
void cmd_chown(const char *owner, const char *path);
void cmd_grep(const char *keyword, const char *path, int showLineNumber);
void cmd_mv(const char *srcPath, const char *destPath);
void cmd_rm(const char *path, int recursive, int force);
void cmd_pwd(void);

#endif
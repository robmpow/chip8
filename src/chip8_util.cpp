#include "chip8_util.h"

const char* file_typenames[] = {"block device",
                                "character device",
                                "directory",
                                "pipe",
                                "symlink",
                                "regular file",
                                "socket",
                                "unkown",};

const char* reg_file = file_typenames[5];

static const char* fileType(mode_t file_stat){
    switch(file_stat & S_IFMT){
        case S_IFBLK:  return file_typenames[0];
        case S_IFCHR:  return file_typenames[1];
        case S_IFDIR:  return file_typenames[2];
        case S_IFIFO:  return file_typenames[3];
        case S_IFLNK:  return file_typenames[4];
        case S_IFREG:  return file_typenames[5];
        case S_IFSOCK: return file_typenames[6];
        default:       return file_typenames[7];
    }
}

const char* fileExists(char* file_path){
    struct stat buff;
    int exists = stat(file_path, &buff);
    return (exists != -1)? fileType(buff.st_mode) : 0;
}
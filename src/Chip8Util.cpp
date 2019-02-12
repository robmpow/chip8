#include "Chip8Util.hpp"

namespace Chip8Util{

const char* fileTypenames[] = {"block device",
                               "character device",
                               "directory",
                               "pipe",
                               "symlink",
                               "regular file",
                               "socket",
                               "unkown",};

const char* regFile = fileTypenames[5];

static const char* fileType(const mode_t& t_fileStat){
    switch(t_fileStat & S_IFMT){
        case S_IFBLK:  return fileTypenames[0];
        case S_IFCHR:  return fileTypenames[1];
        case S_IFDIR:  return fileTypenames[2];
        case S_IFIFO:  return fileTypenames[3];
        case S_IFLNK:  return fileTypenames[4];
        case S_IFREG:  return fileTypenames[5];
        case S_IFSOCK: return fileTypenames[6];
        default:       return fileTypenames[7];
    }
}

const char* fileExists(const char* t_filePath){
    struct stat buff;
    int exists = stat(t_filePath, &buff);
    return (exists != -1)? fileType(buff.st_mode) : 0;
}

} // namespace Chip8Util
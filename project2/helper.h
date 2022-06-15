#ifndef __HELPER_H__
#define __HELPER_H__

ssize_t my_read(int fd, void* buf, size_t count, char* file, int line);

ssize_t my_write(int fd, const void *buf, size_t count, char *file, int line);

#endif

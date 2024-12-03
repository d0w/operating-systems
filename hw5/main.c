#include "fs.h"
#include <stdlib.h>
#include <stdio.h>


int main(int argc, char **argv) {
    // make file system
    if (make_fs("disk") == -1) {
        printf("make_fs failed\n");
        return -1;
    }

    // mount file system
    if (mount_fs("disk") == -1) {
        printf("mount_fs failed\n");
        return -1;
    }

    // create files
    if (fs_create("file1") == -1) {
        printf("fs_create failed\n");
        return -1;
    }
    if (fs_create("file2") == -1) {
        printf("fs_create failed\n");
        return -1;
    }
    if (fs_create("file3") == -1) {
        printf("fs_create failed\n");
        return -1;
    }

    // list files
    char **files;
    if (fs_listfiles(&files) == -1) {
        printf("fs_listfiles failed\n");
        return -1;
    }

    // print files
    int i = 0;
    while (files[i] != NULL) {
        printf("file: %s\n", files[i]);
        i++;
    }

    // open file
    int fs = fs_open("file1");
    if (fs == -1) {
        printf("fs_open failed\n");
        return -1;
    }

    // write to file
    char *buf = "hello world";
    if (fs_write(fs, buf, 11) == -1) {
        printf("fs_write failed\n");
        return -1;
    }

    // read from file
    char *readBuf = malloc(10);
    if (fs_read(fs, readBuf, 6) == -1) {
        printf("fs_read failed\n");
        return -1;
    }
    printf("read: %s\n", readBuf);

    // // close file
    // if (fs_close(fs) == -1) {
    //     printf("fs_close failed\n");
    //     return -1;
    // }

    // // check error conditions
    // if (fs_read(fs, readBuf, 6) != -1) {
    //     printf("fs_read error check failed\n");
    //     return -1;
    // }

    // free files
    free(files);

    // close file system
    if (umount_fs("disk") == -1) {
        printf("umount_fs failed\n");
        return -1;
    }

    return 0;
}
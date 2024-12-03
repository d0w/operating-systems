// #include "fs.h"
// #include <stdlib.h>
// #include <stdio.h>


// int main(int argc, char **argv) {
//     // make file system
//     if (make_fs("disk") == -1) {
//         printf("make_fs failed\n");
//         return -1;
//     }

//     // mount file system
//     if (mount_fs("disk") == -1) {
//         printf("mount_fs failed\n");
//         return -1;
//     }

//     // create files
//     if (fs_create("file1") == -1) {
//         printf("fs_create failed\n");
//         return -1;
//     }
//     if (fs_create("file2") == -1) {
//         printf("fs_create failed\n");
//         return -1;
//     }
//     if (fs_create("file3") == -1) {
//         printf("fs_create failed\n");
//         return -1;
//     }

//     // list files
//     char **files;
//     if (fs_listfiles(&files) == -1) {
//         printf("fs_listfiles failed\n");
//         return -1;
//     }

//     // print files
//     int i = 0;
//     while (files[i] != NULL) {
//         printf("file: %s\n", files[i]);
//         i++;
//     }

//     // open file
//     int fs = fs_open("file1");
//     if (fs == -1) {
//         printf("fs_open failed\n");
//         return -1;
//     }

//     // write to file
//     char *buf = "hello world";
//     if (fs_write(fs, buf, 11) == -1) {
//         printf("fs_write failed\n");
//         return -1;
//     }

//     // read from file
//     char *readBuf = malloc(10);
//     if (fs_read(fs, readBuf, 6) == -1) {
//         printf("fs_read failed\n");
//         return -1;
//     }
//     printf("read: %s\n", readBuf);

//     // // close file
//     // if (fs_close(fs) == -1) {
//     //     printf("fs_close failed\n");
//     //     return -1;
//     // }

//     // // check error conditions
//     // if (fs_read(fs, readBuf, 6) != -1) {
//     //     printf("fs_read error check failed\n");
//     //     return -1;
//     // }

//     // free files
//     free(files);

//     // close file system
//     if (umount_fs("disk") == -1) {
//         printf("umount_fs failed\n");
//         return -1;
//     }

//     return 0;
// }

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "fs.h"

void test_create_delete_create() {
    assert(make_fs("testdisk") == 0);
    assert(mount_fs("testdisk") == 0);

    // Create a 15MB file
    printf("Creating 15MB file\n");
    assert(fs_create("file15MB") == 0);
    int fd15MB = fs_open("file15MB");
    assert(fd15MB >= 0);
    char buffer15MB[2 * 1024 * 1024];
    memset(buffer15MB, 'A', sizeof(buffer15MB));
    printf("size of buffer = %ld\n", sizeof(buffer15MB));
    int ret = fs_write(fd15MB, buffer15MB, sizeof(buffer15MB));
    printf("Writing 15MB file: ret = %d\n", ret);
    assert(ret == sizeof(buffer15MB));
    assert(fs_close(fd15MB) == 0);

    // Create a 1MB file
    assert(fs_create("file1MB") == 0);
    int fd1MB = fs_open("file1MB");
    assert(fd1MB >= 0);
    char buffer1MB[1 * 1024 * 1024];
    memset(buffer1MB, 'A', sizeof(buffer1MB));
    ret = fs_write(fd1MB, buffer1MB, sizeof(buffer1MB));
    printf("Writing 1MB file: ret = %d\n", ret);
    assert(ret == sizeof(buffer1MB));
    assert(fs_close(fd1MB) == 0);

    // Delete the 1MB file
    assert(fs_delete("file1MB") == 0);

    // Create a new 1MB file
    assert(fs_create("file1MB_new") == 0);
    int fd1MB_new = fs_open("file1MB_new");
    assert(fd1MB_new >= 0);
    ret = fs_write(fd1MB_new, buffer1MB, sizeof(buffer1MB));
    printf("Writing new 1MB file: ret = %d\n", ret);
    assert(ret == sizeof(buffer1MB));
    assert(fs_close(fd1MB_new) == 0);

    assert(umount_fs("testdisk") == 0);
}

int main() {
    test_create_delete_create();
    printf("test_create_delete_create passed\n");
    return 0;
}
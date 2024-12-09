#ifndef FS_H
#define FS_H
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "disk.h"

#include <sys/types.h>

#define DISK_BLOCKS 8192
#define DATA_DISK_BLOCKS 4096
#define BLOCK_SIZE 4096
#define MAX_FILE_SIZE 16777216
#define MAX_FILES 64
#define MAX_NAME_LENGTH 15
#define BLOCK_FILE_END 9999
#define BLOCK_INVALID 10000
#define MAX_FILE_DESCRIPTORS 32

typedef struct SuperBlock {
    unsigned int totalBlocks;
    unsigned int fatStart;
    unsigned int fatBlocks;
    unsigned int rootBlocks;
    unsigned int rootStart;
    unsigned int dataStart;
} SuperBlock;

// file allocation table
typedef struct FAT {
    unsigned int table[DISK_BLOCKS];
} FAT;

typedef struct DirectoryEntry {
    char fileName[MAX_NAME_LENGTH + 1];
    unsigned int fileStart;
    unsigned int fileSize;
} DirectoryEntry;

typedef struct Directory {
    DirectoryEntry entries[MAX_FILES];
} Directory;


typedef struct FileDescriptor {
    int open;
    unsigned int fileOffset;
    // unsigned int fileEnd;
    DirectoryEntry *directoryEntry;
} FileDescriptor;


/**
 * Creates a new file system on the virtual disk with name disk_name
 * @param disk_name
 * @return 0 on success, -1 if the disk disk_name could not be created, opened, or properly initialized
 */
int make_fs(char *disk_name);

/**
 * Mounts file system
 * @param disk_name
 * @return 0 on success, -1 when the disk disk_name could not be opened or when the disk does not contain a valid file system
 */
int mount_fs(char *disk_name);


/**
 * Unmounts file system
 * @param disk_name
 * @return 0 on success, -1 when the disk disk_name could not be closed or when data could not be saved to the disk
 */
int umount_fs(char *disk_name);


/**
 * Opens a file
 * @param name
 * @return file descriptor on success, -1 on failure
 */
int fs_open(char *name);

/**
 * Closes a file
 * @param fildes
 * @return 0 on success, -1 on failure
 */
int fs_close(int fildes);

/**
 * Creates a new file in the root directory
 * @param name
 * @return 0 on success, -1 on failure
 */
int fs_create(char *name);

/**
 * Deletes a file
 * @param name
 * @return 0 on success, -1 on failure
 */
int fs_delete(char *name);

/**
 * Reads nbyte bytes of data from the file referenced by fildes into the buffer pointed to by buf
 * @param fildes
 * @param buf
 * @param nbyte
 * @return number of bytes read on success, -1 on failure
 */
int fs_read(int fildes, void *buf, size_t nbyte);

/**
 * Writes nbyte bytes of data to the file referenced by fildes from the buffer pointed to by buf
 * @param fildes
 * @param buf
 * @param nbyte
 * @return number of bytes written on success, -1 on failure
 */
int fs_write(int fildes, void *buf, size_t nbyte);

/**
 * Returns current filesize pointed to by fildes
 * @param fildes
 * @return filesize on success, -1 on failure
 */
int fs_get_filesize(int fildes);

/**
 * Inserts names of all files in the root directory into files
 * @param files
 * @return 0 on success, -1 on failure
 */
int fs_listfiles(char ***files);


/**
 * Sets the file pointer associated with fildes to the argument offset
 * @param fildes
 * @param offset
 * @return 0 on success, -1 on failure
 */
int fs_lseek(int fildes, off_t offset);

/**
 * Truncates file associatd with fildes to length bytes
 * @param fildes
 * @param length
 * @return 0 on success, -1 on failure
 */
int fs_truncate(int fildes, off_t length);

#endif
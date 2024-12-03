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
#define BLOCK_FILE_END 0
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

/*
This function creates a fresh (and empty) file system on the virtual disk with name disk_name.
As part of this function, you should first invoke make_disk(disk_name) to create a new disk.
Then, open this disk and write/initialize the necessary meta-information for your file system so
that it can be later used (mounted). The function returns 0 on success, and -1 if the disk
disk_name could not be created, opened, or properly initialized.
*/
int make_fs(char *disk_name);

/*
This function mounts a file system that is stored on a virtual disk with name disk_name. With
the mount operation, a file system becomes "ready for use." You need to open the disk and
then load the meta-information that is necessary to handle the file system operations that are
discussed below. The function returns 0 on success, and -1 when the disk disk_name could not
be opened or when the disk does not contain a valid file system (that you previously created
with make_fs).
*/
int mount_fs(char *disk_name);


/*
This function unmounts your file system from a virtual disk with name disk_name. As part of
this operation, you need to write back all meta-information so that the disk persistently reflects
all changes that were made to the file system (such as new files that are created, data that is
written, ...). You should also close the disk. The function returns 0 on success, and -1 when the
disk disk_name could not be closed or when data could not be written to the disk (this should
not happen).
It is important to observe that your file system must provide persistent storage. That is, assume
that you have created a file system on a virtual disk and mounted it. Then, you create a few
files and write some data to them. Finally, you unmount the file system. At this point, all data
must be written onto the virtual disk. Another program that mounts the file system at a later
point in time must see the previously created files and the data that was written. This means
that whenever umount_fs is called, all meta-information and file data (that you could
temporarily have only in memory; depending on your implementation) must be written out to
disk.
In addition to the management routines listed above, you are supposed to implement the
following file system functions (which are very similar to the corresponding Linux file system
operations). These file system functions require that a file system was previously mounted
 */
int umount_fs(char *disk_name);

/*
The file specified by name is opened for reading and writing, and the file descriptor
corresponding to this file is returned to the calling function. If successful, fs_open returns a
non-negative integer, which is a file descriptor that can be used to subsequently access this
file. Note that the same file (file with the same name) can be opened multiple times. When this
happens, your file system is supposed to provide multiple, independent file descriptors. Your
library must support a maximum of 32 file descriptors that can be open simultaneously.
fs_open returns -1 on failure. It is a failure when the file with name cannot be found (i.e., it
has not been created previously or is already deleted). It is also a failure when there are
already 32 file descriptors active. When a file is opened, the file offset (seek pointer) is set to 0
(the beginning of the file)
*/
int fs_open(char *name);

/*
The file descriptor fildes is closed. A closed file descriptor can no longer be used to access the
corresponding file. Upon successful completion, a value of 0 is returned. In case the file
descriptor fildes does not exist or is not open, the function returns -1.
*/
int fs_close(int fildes);


/*
This function creates a new file with name name in the root directory of your file system. The
file is initially empty. The maximum length for a file name is 15 characters. Also, there can be at
most 64 files in the directory. Upon successful completion, a value of 0 is returned. fs_create
returns -1 on failure. It is a failure when the file with name already exists, when the file name is
too long (it exceeds 15 characters), or when there are already 64 files present in the root
directory. Note that to access a file that is created, it has to be subsequently opened.
*/
int fs_create(char *name);

/*
This function deletes the file with name name from the root directory of your file system and
frees all data blocks and meta-information that correspond to that file. The file that is being
deleted must not be open. That is, there cannot be any open file descriptor that refers to the file
name. When the file is open at the time that fs_delete is called, the call fails and the file is not
deleted. Upon successful completion, a value of 0 is returned. fs_delete returns -1 on failure. It
is a failure when the file with name does not exist. It is also a failure when the file is currently
open (i.e., there exists at least one open file descriptor that is associated with this file)
*/
int fs_delete(char *name);

/*
This function attempts to read nbyte bytes of data from the file referenced by the descriptor
fildes into the buffer pointed to by buf. The function assumes that the buffer buf is large enough
to hold at least nbyte bytes. When the function attempts to read past the end of the file, it reads
all bytes until the end of the file. Upon successful completion, the number of bytes that were
actually read is returned. This number could be smaller than nbyte when attempting to read
past the end of the file (when trying to read while the file pointer is at the end of the file, the
function returns zero). In case of failure, the function returns -1. It is a failure when the file
descriptor fildes is not valid. The read function implicitly increments the file pointer by the
number of bytes that were actually read.
*/
int fs_read(int fildes, void *buf, size_t nbyte);

/*
This function attempts to write nbyte bytes of data to the file referenced by the descriptor fildes
from the buffer pointed to by buf. The function assumes that the buffer buf holds at least nbyte
bytes. When the function attempts to write past the end of the file, the file is automatically
extended to hold the additional bytes. It is possible that the disk runs out of space while
performing a write operation. In this case, the function attempts to write as many bytes as
possible (i.e., to fill up the entire space that is left). The maximum file size is 16M (which is,
4,096 blocks, each 4K). Upon successful completion, the number of bytes that were actually
written is returned. This number could be smaller than nbyte when the disk runs out of space
(when writing to a full disk, the function returns zero). In case of failure, the function returns -1.
It is a failure when the file descriptor fildes is not valid. The write function implicitly increments
the file pointer by the number of bytes that were actually written.
*/
int fs_write(int fildes, void *buf, size_t nbyte);

/*
This function returns the current size of the file referenced by the file descriptor fildes. In case
fildes is invalid, the function returns -1.
*/
int fs_get_filesize(int fildes);

/*
This function creates and populates an array of all filenames currently known to the file system.
To terminate the array, your implementation should add a NULL pointer after the last element in
the array. On success the function returns 0, in the case of an error the function returns -1
*/
int fs_listfiles(char ***files);

/*
This function sets the file pointer (the offset used for read and write operations) associated with
the file descriptor fildes to the argument offset. It is an error to set the file pointer beyond the
end of the file. To append to a file, one can set the file pointer to the end of a file, for example,
by calling fs_lseek(fd, fs_get_filesize(fd));. Upon successful completion, a value
of 0 is returned. fs_lseek returns -1 on failure. It is a failure when the file descriptor fildes is
invalid, when the requested offset is larger than the file size, or when offset is less than zero.
 */
int fs_lseek(int fildes, off_t offset);

/*
This function causes the file referenced by fildes to be truncated to length bytes in size. If the
file was previously larger than this new size, the extra data is lost and the corresponding data
blocks on disk (if any) must be freed. It is not possible to extend a file using fs_truncate.
When the file pointer is larger than the new length, then it is also set to length (the end of the
file). Upon successful completion, a value of 0 is returned. fs_lseek returns -1 on failure. It is a
failure when the file descriptor fildes is invalid or the requested length is larger than the file
size.
*/
int fs_truncate(int fildes, off_t length);

#endif
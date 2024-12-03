#include "fs.h"

static SuperBlock *superblock;
static FAT *fat;
static Directory *rootDirectory;
static FileDescriptor fileDescriptors[MAX_FILE_DESCRIPTORS];
static int MOUNTED = 0;


int make_fs(char *disk_name) {
    // attempt to make and open disk
    if (make_disk(disk_name) == -1) {
        return -1;
    }
    if (open_disk(disk_name) == -1) {
        return -1;
    }

    // malloc
    superblock = malloc(sizeof(SuperBlock));
    fat = malloc(sizeof(FAT));
    rootDirectory = malloc(sizeof(Directory));

    // initialize superblock
    superblock->totalBlocks = DISK_BLOCKS;
    superblock->fatStart = 1;
    superblock->fatBlocks = (sizeof(FAT) + BLOCK_SIZE - 1) / BLOCK_SIZE; 
    superblock->rootStart = superblock->fatStart + superblock->fatBlocks;
    superblock->rootBlocks = (sizeof(Directory) + BLOCK_SIZE - 1) / BLOCK_SIZE; 
    superblock->dataStart = superblock->rootStart + superblock->rootBlocks + 3;

    // Initialize FAT
    memset(fat->table, 0, sizeof(fat->table));

    // Initialize root directory
    memset(rootDirectory->entries, 0, sizeof(rootDirectory->entries));

    // Write superblock to disk
    if (block_write(0, (char *)superblock) == -1) {
        return -1;
    }
    int i;
    // Write FAT to disk
    for (i = 0; i < superblock->fatBlocks; i++) {
        if (block_write(superblock->fatStart + i, ((char *)fat) + i * BLOCK_SIZE)  == -1) {
            return -1;
        }
    }

    // Write root directory to disk
    for (i = 0; i < superblock->rootBlocks; i++) {
        if (block_write(superblock->rootStart + i, ((char *)rootDirectory) + i * BLOCK_SIZE) == -1) {
            return -1;
        }
    }

    // // printf("rootstart: %d\n", superblock->rootStart);
    // // printf("rootblocks: %d\n", superblock->rootBlocks);
    // // printf("datastart: %d\n", superblock->dataStart);
    // // printf("fatstart: %d\n", superblock->fatStart);
    // // printf("fatblocks: %d\n", superblock->fatBlocks);

    // // initialize file allocation table 
    // memset(fat->table, 0, sizeof(fat->table));
    // // init root dir
    // memset(rootDirectory->entries, 0, sizeof(rootDirectory->entries));

    // // write initialized data to disk
    // if (block_write(0, ) == -1) {
    //     return -1;
    // }
    // // char buff[BLOCK_SIZE];
    // // block_read(0, buff);
    // // printf("superblock: %d\n", ((SuperBlock *)buff)->totalBlocks);

    // int i;
    // for (i = 0; i < superblock->fatBlocks; i++) {
    //     if (block_write(superblock->fatStart + i, ((char *)&fat) + i * BLOCK_SIZE) == -1) {
    //         return -1;
    //     }
    // }
    // for (i = 0; i < superblock->rootBlocks; i++) {
    //     if (block_write(superblock->rootStart + i, ((char *)&rootDirectory) + i * BLOCK_SIZE) == -1) {
    //         return -1;
    //     }
    // }

    // close
    if (close_disk() == -1) {
        return -1;
    }

    return 0;
}

int mount_fs(char *disk_name) {
    // open disk
    if (!disk_name || MOUNTED) {
        return -1;
    }
    if (open_disk(disk_name) == -1) {
        return -1;
    }

    // read metadata from disk to superblock, root directory, and FAT
    if (block_read(0, (char *)superblock) == -1) {
        return -1;
    }
    int i;
    for (i = 0; i < superblock->rootBlocks; i++) {
        if (block_read(superblock->rootStart + i, ((char *)rootDirectory) + i * BLOCK_SIZE) == -1) {
            return -1;
        }
    }
    for (i = 0; i < superblock->fatBlocks; i++) {
        if (block_read(superblock->fatStart + i, ((char *)fat) + i * BLOCK_SIZE) == -1) {
            return -1;
        }
    }


    MOUNTED = 1;

    return 0;
}

int umount_fs(char *disk_name) {
    if (!MOUNTED || !disk_name) {
        return -1;
    }

    // save metadata to disk
    if (block_write(0, (char *)superblock) == -1) {
        return -1;
    }
    int i;
    for (i = 0; i < superblock->rootBlocks; i++) {
        if (block_write(superblock->rootStart + i, ((char *)rootDirectory) + i * BLOCK_SIZE) == -1) {
            return -1;
        }
    }
    for (i = 0; i < superblock->fatBlocks; i++) {
        if (block_write(superblock->fatStart + i, ((char *)fat) + i * BLOCK_SIZE) == -1) {
            return -1;
        }
    }

    // reset file descriptors since we are unmounting
    memset(fileDescriptors, 0, sizeof(fileDescriptors));

    // close disk
    if (close_disk() == -1) {
        return -1;
    }

    MOUNTED = 0;
    return 0;

}

int fs_open(char *name) {
    if (!MOUNTED) {
        return -1;
    }

    // find file descriptor entry 
    int i, j;
    for (i = 0; i < MAX_FILES; i++) {
        // see if file exists in root directory
        if (strcmp(rootDirectory->entries[i].fileName, name) == 0) {
            // check if file exists, then add to file descriptors
            for (j = 0; j < MAX_FILES; j++) {
                if (!fileDescriptors[j].open) {
                    fileDescriptors[j].open = 1;
                    fileDescriptors[j].fileOffset = 0;
                    fileDescriptors[j].directoryEntry = &(rootDirectory->entries[i]);
                    return j;
                }
            }
        }
    }
    return -1;
}

int fs_close(int fildes) {
    if (!MOUNTED) {
        return -1;
    }
    if (fildes < 0 || fildes >= MAX_FILES) {
        return -1;
    }

    if (!fileDescriptors[fildes].open) {
        return -1;
    }

    // find file descriptor entry and close it
    fileDescriptors[fildes].open = 0;
    return 0;
}

int fs_create(char *name) {
    if (!MOUNTED) {
        return -1;
    }
    if (strlen(name) > MAX_NAME_LENGTH) {
        return -1;
    }

    // check if file exists
    int i;
    for (i = 0; i < MAX_FILES; i++) {
        if (strcmp(rootDirectory->entries[i].fileName, name) == 0) {
            return -1;
        }
    }

    // find empty entry in root directory and add file to that
    for (i = 0; i < MAX_FILES; i++) {
        if (strcmp(rootDirectory->entries[i].fileName, "") == 0) {
            strcpy(rootDirectory->entries[i].fileName, name);
            rootDirectory->entries[i].fileStart = 0;
            rootDirectory->entries[i].fileSize = 0;
            return 0;
        }
    }
    return -1;


}


int fs_delete(char *name) {
    if (!MOUNTED) {
        return -1;
    }
    if (strlen(name) > MAX_NAME_LENGTH) {
        return -1;
    }

    // find file
    int i;
    for (i = 0; i < MAX_FILES; i++) {
        if (strcmp(rootDirectory->entries[i].fileName, name) == 0) {
            // if found, check if open 
            if (fileDescriptors[i].open) {
                return -1;
            }

            // remove file from FAT 
            unsigned int block = rootDirectory->entries[i].fileStart;
            while (block != BLOCK_FILE_END) {
                // traverse linnked list and remove block from FAT each time
                unsigned int nextBlock = fat->table[block];
                fat->table[block] = 0;
                block = nextBlock;
            }

            // remove directory entry info from root directory
            memset(&(rootDirectory->entries[i]), 0, sizeof(DirectoryEntry));
            return 0;
        }
    }

    return -1;
}

int fs_read(int filedes, void *buf, size_t nbyte) {
    // error checking: mounted, file is open and valid fd
    if (!MOUNTED) {
        return -1;
    }
    if (filedes < 0 || filedes >= MAX_FILES) {
        return -1;
    }
    if (!fileDescriptors[filedes].open) {
        return -1;
    }

    // get file descriptor
    FileDescriptor *fd = &fileDescriptors[filedes];
    fd->open = 1;

    // get root directory entry and file start
    DirectoryEntry *entry = fd->directoryEntry;
    unsigned int offset = fd->fileOffset;
    unsigned int fileStart = entry->fileStart;

    unsigned int bytesRead = 0;
    while (bytesRead < nbyte) {
        // get first block, block offset (from start of first block), and position within block
        unsigned int block = fileStart;
        unsigned int blockOffset = offset / BLOCK_SIZE;
        unsigned int blockPosition = offset % BLOCK_SIZE;

        // go to correct block
        int i = 0;
        for (i = 0; i < blockOffset; i++) {
            block = fat->table[block];
        }

        // read from block
        char blockBuf[BLOCK_SIZE];
        if (block_read(superblock->dataStart + block, blockBuf) == -1) {
            return -1;
        }

        // figure out how many bytes we have left to read
        unsigned int bytesLeft = BLOCK_SIZE - blockPosition;
        // if we have more bytes to read than bytes left in block, only read up to bytes left
        if (bytesLeft > nbyte - bytesRead) {
            bytesLeft = nbyte - bytesRead;
        }
        // if approaching end of file, only read until end of file
        if (bytesLeft > entry->fileSize - offset) {
            bytesLeft = entry->fileSize - offset;
        }

        // copy data to buffer
        // 00..bytesRead write blockdata...blockPosition however many left bytes
        memcpy(buf + bytesRead, blockBuf + blockPosition, bytesLeft);
        
        // continue iteration
        bytesRead += bytesLeft;
        offset += bytesLeft;
    }
    // move file offset in file descriptor
    fd->fileOffset += bytesRead;
    return bytesRead;
}

int fs_write(int fildes, void *buf, size_t nbyte) {
    if (!MOUNTED) {
        return -1;
    }
    if (fildes < 0 || fildes >= MAX_FILES) {
        return -1;
    }   

    // get file descriptor and directory entry
    FileDescriptor *fd = &fileDescriptors[fildes];
    if (!fd->open) {
        return -1;
    }
    DirectoryEntry *entry = fd->directoryEntry;

    // get file start and offset
    unsigned int fileStart = entry->fileStart;
    unsigned int offset = fd->fileOffset;
    
    unsigned int bytesWritten = 0;

    while (bytesWritten < nbyte) {
        // get block, offset and position
        unsigned int block = fileStart;
        unsigned int blockOffset = offset / BLOCK_SIZE;
        unsigned int blockPosition = offset % BLOCK_SIZE;

        // traverse to offset
        int i = 0;
        for (i = 0; i < blockOffset; i++) {
            if (block == BLOCK_FILE_END) {
                // if we reach the end of the file or empty block, allocate a new block
                int j;
                for (j = 0; j < DISK_BLOCKS; j++) {
                    if (fat->table[j] == 0) {
                        fat->table[block] = j;
                        block = j;
                        break;
                    }
                }
                // if nothing found, return early
                if (block == BLOCK_FILE_END) {
                    return bytesWritten;
                }
            }
            block = fat->table[block];
        }

        // get current block data to write to end of existing data
        char blockBuf[BLOCK_SIZE];
        if (block_read(superblock->dataStart + block, blockBuf) == -1) {
            return -1;
        }

        // get bytes left to write
        unsigned int bytesLeft = BLOCK_SIZE - blockPosition;
        // update bytes left to be only up to data wanted
        if (bytesLeft > nbyte - bytesWritten) {
            bytesLeft = nbyte - bytesWritten;
        }
        
        memcpy(blockBuf + blockPosition, buf + bytesWritten, bytesLeft);
        if (block_write(superblock->dataStart + block, blockBuf) == -1) {
            return -1;
        }
        bytesWritten += bytesLeft;
        offset += bytesLeft;

    }

    // set file offset and file size
    fd->fileOffset += bytesWritten;
    if (fd->fileOffset > entry->fileSize) {
        entry->fileSize = fd->fileOffset;
    }
    return bytesWritten;
}

int fs_get_filesize(int fildes) {
    if (!MOUNTED) {
        return -1;
    }
    if (fildes < 0 || fildes >= MAX_FILES) {
        return -1;
    }

    // get file descriptor
    FileDescriptor fd = fileDescriptors[fildes];
    if (!fd.open) {
        return -1;
    }

    return fd.directoryEntry->fileSize;
}

int fs_listfiles(char ***files) {
    if (!MOUNTED) {
        return -1;
    }

    *files = malloc((MAX_FILES + 1) * sizeof(char *));

    int count = 0;
    int i;
    for (i = 0; i < MAX_FILES; i++) {
        // if entry
        if (strcmp(rootDirectory->entries[i].fileName, "") != 0) {
            (*files)[count] = strdup(rootDirectory->entries[i].fileName);
            if ((*files)[count] == NULL) {
                // something went wrong so abort
                int j;
                for (j = 0; j < count; j++) {
                    free((*files)[j]);
                }
                free(*files);
                return -1;
            }
        }
        count++;
    }

    // add in last entry as specified
    (*files)[count] = NULL;

    return 0;
}

int fs_lseek(int fildes, off_t offset) {
    if (!MOUNTED) {
        return -1;
    }
    if (fildes < 0 || fildes >= MAX_FILES) {
        return -1;
    }

    // get fd
    FileDescriptor *fd = &fileDescriptors[fildes];
    if (!fd->open) {
        return -1;
    }

    // check if offset valid
    if (offset > fd->directoryEntry->fileSize || offset < 0) {
        return -1;
    }

    // move offset
    fileDescriptors[fildes].fileOffset = offset;

    return 0;
}

int fs_truncate(int fildes, off_t length) {
    if (!MOUNTED) {
        return -1;
    }
    if (fildes < 0 || fildes >= MAX_FILES) {
        return -1;
    }

    // get fd and directory entry
    FileDescriptor *fd = &fileDescriptors[fildes];
    if (!fd->open) {
        return -1;
    }
    DirectoryEntry *entry = fd->directoryEntry;

    if (length > entry->fileSize || length < 0) {
        return -1;
    }

    // start from first block and keep traversing to length
    unsigned int block = entry->fileStart;
    unsigned int blockOffset = length / BLOCK_SIZE;
    // unsigned int blockPosition = length % BLOCK_SIZE;

    int i;
    for (i = 0; i < blockOffset; i++) {
        block = fat->table[block];
    }

    // free blocks after length
    while (block != BLOCK_FILE_END) {
        unsigned int nextBlock = fat->table[block];
        fat->table[block] = 0;
        block = nextBlock;
    }

    // update file size and set fd if needed
    entry->fileSize = length;
    if (fd->fileOffset > length) {
        fd->fileOffset = length;
    }

    return 0;

}
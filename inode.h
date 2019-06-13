#ifndef INODE_H
#define INODE_H

typedef struct Inode
{
    char fileName[128];
    int fileSize;
    int dataBlockPointers[128];
    char nothing[380];

} Inode;

#endif
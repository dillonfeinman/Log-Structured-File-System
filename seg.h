#ifndef SEG_H
#define SEG_H

#include <string>

typedef struct Seg
{
    std::string driveFileName; //Storing string for "./DRIVE/SEGMENT#"
    char data[1024][1024]; //All the blocks in the segment --> [block number in segment][index in block]
    int currentDataBlock;
    int currentSegment;
} Seg;

#endif

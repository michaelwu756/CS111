#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include "ext2_fs.h"

int fileSystemFD = -1;

char *buf;
int buf_size = 256;

struct ext2_super_block superblock;
uint32_t totalNumBlocks;
uint32_t totalNumInodes;
uint32_t blockSize;
uint16_t inodeSize;
uint32_t blocksPerGroup;
uint32_t inodesPerGroup;
uint32_t firstInode;

void checkForError(int result, char *message)
{
  if(result==-1)
  {
    fprintf(stderr, "Error %s: %s\n", message, strerror(errno));
    exit(2);
  }
}

void printUsage(char *progName)
{
  fprintf(stderr, "Usage: %s FILESYSTEM\n", progName);
  exit(1);
}

void freeMemory()
{
  close(fileSystemFD);
  if (buf)
    free(buf);
}

//ith block/inode is at byte (i-1)/8, bit (i-1)%8 where bits are indexed with 0 as lowest order bit
//and 7 as highest order bit in both bitmaps

void scanFreeBlockEntries(uint32_t blockBitmap, uint32_t totalBlocksInGroup)
{
  uint8_t *bitmap = malloc(blockSize);

  ssize_t numRead = pread(fileSystemFD, bitmap, blockSize, blockBitmap*blockSize);
  checkForError(numRead, "reading block bitmap");

  uint32_t i;
  for (i = 0; i<totalBlocksInGroup; i++)
  {
    uint8_t currentByte = bitmap[i/8];
    if(((currentByte>>(i%8))& 0x1) == 0)//0 means free
    {
      memset(buf, 0, buf_size);
      sprintf(buf, "BFREE,%u\n",i+1);
      checkForError(write(STDOUT_FILENO,buf,strlen(buf)), "printing free block");
    }
  }
  free(bitmap);
}

void scanFreeInodeEntries(uint32_t inodeBitmap, uint32_t totalInodesInGroup)
{
  uint8_t *bitmap = malloc(blockSize);

  ssize_t numRead = pread(fileSystemFD, bitmap, blockSize, inodeBitmap*blockSize);
  checkForError(numRead, "reading inode bitmap");

  uint32_t i;
  for (i = 0; i<totalInodesInGroup; i++)
  {
    uint8_t currentByte = bitmap[i/8];
    if(((currentByte>>(i%8))& 0x1) == 0)//0 means free
    {
      memset(buf, 0, buf_size);
      sprintf(buf, "IFREE,%u\n",i+1);
      checkForError(write(STDOUT_FILENO,buf,strlen(buf)), "printing free inode");
    }
  }
  free(bitmap);
}

void superblockSummary() {
  ssize_t numRead = pread(fileSystemFD, &superblock, sizeof(struct ext2_super_block), 1024);
  checkForError(numRead, "reading superblock");

  totalNumBlocks=superblock.s_blocks_count;
  totalNumInodes=superblock.s_inodes_count;
  blockSize=EXT2_MIN_BLOCK_SIZE<<superblock.s_log_block_size;
  inodeSize=superblock.s_inode_size;
  blocksPerGroup=superblock.s_blocks_per_group;
  inodesPerGroup=superblock.s_inodes_per_group;
  firstInode=superblock.s_first_ino;

  memset(buf, 0, buf_size);
  sprintf(buf, "SUPERBLOCK,%u,%u,%u,%u,%u,%u,%u\n",
	  totalNumBlocks,
	  totalNumInodes,
	  blockSize,
	  inodeSize,
	  blocksPerGroup,
	  inodesPerGroup,
	  firstInode);

  checkForError(write(STDOUT_FILENO, buf, strlen(buf)),"printing superblock data");
}

void groupSummary()
{
  uint32_t numGroups = 1+(totalNumBlocks-1)/blocksPerGroup;
  struct ext2_group_desc *groupDescriptorTable = malloc(numGroups*sizeof(struct ext2_group_desc));
  ssize_t numRead = pread(fileSystemFD, groupDescriptorTable, numGroups*sizeof(struct ext2_group_desc), 1024+sizeof(struct ext2_super_block));
  checkForError(numRead, "reading group descriptor table");

  ssize_t i;
  for (i = 0; i < numGroups; i++)
  {
    uint32_t groupNumber = i;

    uint32_t totalBlocksInGroup;
    if (i == numGroups-1 && totalNumBlocks%blocksPerGroup != 0)
      totalBlocksInGroup = totalNumBlocks%blocksPerGroup;
    else
      totalBlocksInGroup = blocksPerGroup;

    uint32_t totalInodesInGroup;
    if (i == numGroups-1 && totalNumInodes%inodesPerGroup != 0)
      totalInodesInGroup = totalNumInodes%inodesPerGroup;
    else
      totalInodesInGroup = inodesPerGroup;

    uint16_t freeBlockCount = groupDescriptorTable[i].bg_free_blocks_count;
    uint16_t freeInodesCount = groupDescriptorTable[i].bg_free_inodes_count;
    uint32_t blockBitmapNumber = groupDescriptorTable[i].bg_block_bitmap;
    uint32_t inodeBitmapNumber = groupDescriptorTable[i].bg_inode_bitmap;
    uint32_t inodeTableNumber = groupDescriptorTable[i].bg_inode_table;

    memset(buf, 0, buf_size);
    sprintf(buf, "GROUP,%u,%u,%u,%u,%u,%u,%u,%u\n",
	    groupNumber,
	    totalBlocksInGroup,
	    totalInodesInGroup,
	    freeBlockCount,
	    freeInodesCount,
	    blockBitmapNumber,
	    inodeBitmapNumber,
	    inodeTableNumber);

    checkForError(write(STDOUT_FILENO, buf, strlen(buf)), "printing group descriptor data");

    scanFreeBlockEntries(blockBitmapNumber, totalBlocksInGroup);
    scanFreeInodeEntries(inodeBitmapNumber, totalInodesInGroup);
  }
}

int main(int argc, char * argv[])
{
  if (argc < 2)
    printUsage(argv[0]);

  fileSystemFD=open(argv[1], O_RDONLY);
  checkForError(fileSystemFD, "opening filesystem image");

  buf=malloc(buf_size*sizeof(char));

  atexit(freeMemory);

  superblockSummary();
  groupSummary();

  exit(0);
}

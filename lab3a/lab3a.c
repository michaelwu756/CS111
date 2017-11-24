#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include "ext2_fs.h"

int fileSystemFD = -1;

char *buf;
int buf_size = 512;

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

void directoryBlockSummary(uint32_t parentInode, uint32_t blockNum, uint32_t logicalBlockNum)
{
  uint32_t byteOffset=0;
  uint32_t logicalByteOffset;
  uint32_t inodeNumber;
  uint16_t entryLength;
  uint8_t nameLength;
  char *name;

  while(byteOffset!=blockSize)
  {
    struct ext2_dir_entry directoryEntry;
    checkForError(pread(fileSystemFD, &directoryEntry, sizeof(struct ext2_dir_entry), blockNum*blockSize+byteOffset), "reading directory entry");
    logicalByteOffset=logicalBlockNum*blockSize+byteOffset;
    inodeNumber=directoryEntry.inode;
    entryLength=directoryEntry.rec_len;
    nameLength=directoryEntry.name_len;
    name=malloc((nameLength+1)*sizeof(char));
    strncpy(name, directoryEntry.name, nameLength);
    name[nameLength]='\0';

    memset(buf, 0, buf_size);
    sprintf(buf, "DIRENT,%u,%u,%u,%u,%u,'%s'\n",
            parentInode,
            logicalByteOffset,
            inodeNumber,
            entryLength,
            nameLength,
            name);
    if(inodeNumber!=0)
      checkForError(write(STDOUT_FILENO,buf,strlen(buf)), "printing directory entry");
    free(name);
    byteOffset+=entryLength;
  }
}

int directoryIndirectBlockSummary(int indirectionLevel, uint32_t parentInode, uint32_t blockNum, uint32_t logicalBlockNum)
{
  if(indirectionLevel==0)
  {
    directoryBlockSummary(parentInode, blockNum, logicalBlockNum);
    return 1;
  }
  int stride=1;
  int i;
  for(i=0; i<indirectionLevel-1;i++)
    stride*=blockSize/4;
  int traversedBlocks=0;
  uint32_t referencedBlockNum;
  uint32_t offset;
  for(offset=0; offset<blockSize; offset+=4)
  {
    checkForError(pread(fileSystemFD, &referencedBlockNum, sizeof(uint32_t), blockNum*blockSize+offset), "reading indirect block entry");
    if(referencedBlockNum!=0)
      directoryIndirectBlockSummary(indirectionLevel-1, parentInode, referencedBlockNum, logicalBlockNum+traversedBlocks);
    traversedBlocks+=stride;
  }
  return traversedBlocks;
}

int indirectBlockSummary(int indirectionLevel, uint32_t parentInode, uint32_t blockNum, uint32_t logicalBlockNum)
{
  if(indirectionLevel==0)
    return 1;
  int stride=1;
  int i;
  for(i=0; i<indirectionLevel-1;i++)
    stride*=blockSize/4;
  int traversedBlocks=0;
  uint32_t referencedBlockNum;
  uint32_t offset;
  for(offset=0; offset<blockSize; offset+=4)
  {
    checkForError(pread(fileSystemFD, &referencedBlockNum, sizeof(uint32_t), blockNum*blockSize+offset), "reading indirect block entry");
    if(referencedBlockNum!=0)
    {
      memset(buf, 0, buf_size);
      sprintf(buf, "INDIRECT,%u,%u,%u,%u,%u\n",
              parentInode,
              indirectionLevel,
              logicalBlockNum+traversedBlocks,
              blockNum,
              referencedBlockNum);
      checkForError(write(STDOUT_FILENO,buf,strlen(buf)), "printing indirect block entry");
      indirectBlockSummary(indirectionLevel-1, parentInode, referencedBlockNum, logicalBlockNum+traversedBlocks);
    }
    traversedBlocks+=stride;
  }
  return traversedBlocks;
}

void inodeSummary(uint32_t inodeTable, uint32_t inodeIndex, uint32_t groupNumber)
{
  struct ext2_inode inode;
  checkForError(pread(fileSystemFD, &inode, inodeSize, inodeTable*blockSize+inodeIndex*inodeSize), "reading inode");

  uint32_t inodeNumber=groupNumber*inodesPerGroup+inodeIndex+1;
  uint16_t fileTypeVal=(inode.i_mode>>12)&0xF;
  char fileType='?';
  if(fileTypeVal==0x4)
    fileType='d';
  else if(fileTypeVal==0x8)
    fileType='f';
  else if(fileTypeVal==0xA)
    fileType='s';
  uint16_t mode=inode.i_mode&0xFFF;
  uint16_t owner=inode.i_uid;
  uint16_t group=inode.i_gid;
  uint16_t linkCount=inode.i_links_count;
  time_t ctime=(time_t)inode.i_ctime;
  time_t mtime=(time_t)inode.i_mtime;
  time_t atime=(time_t)inode.i_atime;
  struct tm *ctimeStruct=gmtime(&ctime);
  struct tm *mtimeStruct=gmtime(&mtime);
  struct tm *atimeStruct=gmtime(&atime);
  char changeTime[256];
  char modificationTime[256];
  char accessTime[256];
  sprintf(changeTime, "%02d/%02d/%02d %02d:%02d:%02d", ctimeStruct->tm_mon+1, ctimeStruct->tm_mday, ctimeStruct->tm_year%100, ctimeStruct->tm_hour, ctimeStruct->tm_min, ctimeStruct->tm_sec);
  sprintf(modificationTime, "%02d/%02d/%02d %02d:%02d:%02d", mtimeStruct->tm_mon+1, mtimeStruct->tm_mday, mtimeStruct->tm_year%100, mtimeStruct->tm_hour, mtimeStruct->tm_min, mtimeStruct->tm_sec);
  sprintf(accessTime, "%02d/%02d/%02d %02d:%02d:%02d", atimeStruct->tm_mon+1, atimeStruct->tm_mday, atimeStruct->tm_year%100, atimeStruct->tm_hour, atimeStruct->tm_min, atimeStruct->tm_sec);
  uint64_t fileSize=((uint64_t)(inode.i_dir_acl)<<32)|inode.i_size;
  uint32_t numBlocks=inode.i_blocks;
  uint32_t *blockNum=inode.i_block;

  memset(buf, 0, buf_size);
  sprintf(buf, "INODE,%u,%c,%03o,%u,%u,%u,%s,%s,%s,%lu,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u\n",
          inodeNumber,
          fileType,
          mode,
          owner,
          group,
          linkCount,
          changeTime,
          modificationTime,
          accessTime,
          fileSize,
          numBlocks,
          blockNum[0],
          blockNum[1],
          blockNum[2],
          blockNum[3],
          blockNum[4],
          blockNum[5],
          blockNum[6],
          blockNum[7],
          blockNum[8],
          blockNum[9],
          blockNum[10],
          blockNum[11],
          blockNum[12],
          blockNum[13],
          blockNum[14]);
  if(mode!=0 && linkCount!=0)
  {
    checkForError(write(STDOUT_FILENO,buf,strlen(buf)), "printing inode summary");
    if(fileType=='d')
    {
      int logicalBlock;
      for(logicalBlock=0;logicalBlock<12; logicalBlock++)
        if(blockNum[logicalBlock]!=0)
          directoryBlockSummary(inodeNumber, blockNum[logicalBlock], logicalBlock);
      int i;
      for(i=0;i<3;i++)
        if(blockNum[12+i]!=0)
          logicalBlock+=directoryIndirectBlockSummary(1+i, inodeNumber, blockNum[12+i], logicalBlock);
    }
    else
    {
      int logicalBlock=12;
      int i;
      for(i=0;i<3;i++)
        if(blockNum[12+i]!=0)
          logicalBlock+=indirectBlockSummary(1+i, inodeNumber, blockNum[12+i], logicalBlock);
    }
  }
}

//ith block/inode is at byte (i-1)/8, bit (i-1)%8 where bits are indexed with 0 as lowest order bit
//and 7 as highest order bit in both bitmaps

void scanBlockBitmap(uint32_t blockBitmap, uint32_t totalBlocksInGroup)
{
  uint8_t *bitmap = malloc(blockSize);
  checkForError(pread(fileSystemFD, bitmap, blockSize, blockBitmap*blockSize), "reading block bitmap");

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

void scanInodeBitmap(uint32_t inodeBitmap, uint32_t totalInodesInGroup, uint32_t inodeTable, uint32_t groupNumber)
{
  uint8_t *bitmap = malloc(blockSize);
  checkForError(pread(fileSystemFD, bitmap, blockSize, inodeBitmap*blockSize), "reading inode bitmap");

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
    else
      inodeSummary(inodeTable,i,groupNumber);
  }
  free(bitmap);
}

void superblockSummary()
{
  checkForError(pread(fileSystemFD, &superblock, sizeof(struct ext2_super_block), 1024), "reading superblock");

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
  checkForError(pread(fileSystemFD, groupDescriptorTable, numGroups*sizeof(struct ext2_group_desc), 1024+sizeof(struct ext2_super_block)), "reading group descriptor table");

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

    scanBlockBitmap(blockBitmapNumber, totalBlocksInGroup);
    scanInodeBitmap(inodeBitmapNumber, totalInodesInGroup, inodeTableNumber, groupNumber);
  }
}

int main(int argc, char * argv[])
{
  if (argc != 2)
    printUsage(argv[0]);

  if(argv[1][0]=='-')
    printUsage(argv[0]);

  fileSystemFD=open(argv[1], O_RDONLY);
  checkForError(fileSystemFD, "opening filesystem image");

  buf=malloc(buf_size*sizeof(char));

  atexit(freeMemory);

  superblockSummary();
  groupSummary();

  exit(0);
}

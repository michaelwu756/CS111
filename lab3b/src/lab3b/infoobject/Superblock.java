package lab3b.infoobject;

public class Superblock {
    private int totalblocks;
    private int totalInodes;
    private int blockSize;
    private int inodeSize;
    private int blocksPerGroup;
    private int inodesPerGroup;
    private int firstNonReservedInode;

    public Superblock(int totalblocks, int totalInodes, int blockSize, int inodeSize, int blocksPerGroup, int inodesPerGroup, int firstNonReservedInode) {
        this.totalblocks = totalblocks;
        this.totalInodes = totalInodes;
        this.blockSize = blockSize;
        this.inodeSize = inodeSize;
        this.blocksPerGroup = blocksPerGroup;
        this.inodesPerGroup = inodesPerGroup;
        this.firstNonReservedInode = firstNonReservedInode;
    }

    public int getTotalblocks() {
        return totalblocks;
    }

    public int getTotalInodes() {
        return totalInodes;
    }

    public int getBlockSize() {
        return blockSize;
    }

    public int getInodeSize() {
        return inodeSize;
    }

    public int getBlocksPerGroup() {
        return blocksPerGroup;
    }

    public int getInodesPerGroup() {
        return inodesPerGroup;
    }

    public int getFirstNonReservedInode() {
        return firstNonReservedInode;
    }
}
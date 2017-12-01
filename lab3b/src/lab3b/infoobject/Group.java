package lab3b.infoobject;

public class Group {
    private int groupNumber;
    private int totalBlocks;
    private int totalInodes;
    private int freeBlocks;
    private int freeInodes;
    private int blockBitmapBlockNum;
    private int inodeBitmapBlockNum;
    private int firstInodeBlockNum;

    public Group(int groupNumber, int totalBlocks, int totalInodes, int freeBlocks, int freeInodes, int blockBitmapBlockNum, int inodeBitmapBlockNum, int firstInodeBlockNum) {
        this.groupNumber = groupNumber;
        this.totalBlocks = totalBlocks;
        this.totalInodes = totalInodes;
        this.freeBlocks = freeBlocks;
        this.freeInodes = freeInodes;
        this.blockBitmapBlockNum = blockBitmapBlockNum;
        this.inodeBitmapBlockNum = inodeBitmapBlockNum;
        this.firstInodeBlockNum = firstInodeBlockNum;
    }

    public int getGroupNumber() {
        return groupNumber;
    }

    public int getTotalBlocks() {
        return totalBlocks;
    }

    public int getTotalInodes() {
        return totalInodes;
    }

    public int getFreeBlocks() {
        return freeBlocks;
    }

    public int getFreeInodes() {
        return freeInodes;
    }

    public int getBlockBitmapBlockNum() {
        return blockBitmapBlockNum;
    }

    public int getInodeBitmapBlockNum() {
        return inodeBitmapBlockNum;
    }

    public int getFirstInodeBlockNum() {
        return firstInodeBlockNum;
    }
}
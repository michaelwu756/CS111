package lab3b.infoobject;

public class Indirect {
    private int parentInode;
    private int indirectionLevel;
    private int logicalBlockOffset;
    private int scannedBlockNum;
    private int referencedBlock;

    public Indirect(int parentInode, int indirectionLevel, int logicalBlockOffset, int scannedBlockNum, int referencedBlock) {
        this.parentInode = parentInode;
        this.indirectionLevel = indirectionLevel;
        this.logicalBlockOffset = logicalBlockOffset;
        this.scannedBlockNum = scannedBlockNum;
        this.referencedBlock = referencedBlock;
    }

    public int getParentInode() {
        return parentInode;
    }

    public int getIndirectionLevel() {
        return indirectionLevel;
    }

    public int getLogicalBlockOffset() {
        return logicalBlockOffset;
    }

    public int getScannedBlockNum() {
        return scannedBlockNum;
    }

    public int getReferencedBlock() {
        return referencedBlock;
    }
}
package lab3b.infoobject;

public class Inode {
    private int inodeNumber;
    private String fileType;
    private String mode;
    private int owner;
    private int group;
    private int linkCount;
    private String changeTime;
    private String modificationTime;
    private String accessTime;
    private int fileSize;
    private int numBlocks;
    private int[] directBlockNum;
    private int singleIndirectBlockNum;
    private int doubleIndirectBlockNum;
    private int tripleIndirectBlockNum;

    public Inode(int inodeNumber, String fileType, String mode, int owner, int group, int linkCount, String changeTime, String modificationTime, String accessTime, int fileSize, int numBlocks, int directBlockNum1, int directBlockNum2, int directBlockNum3, int directBlockNum4, int directBlockNum5, int directBlockNum6, int directBlockNum7, int directBlockNum8, int directBlockNum9, int directBlockNum10, int directBlockNum11, int directBlockNum12, int singleIndirectBlockNum, int doubleIndirectBlockNum, int tripleIndirectBlockNum) {
        this.inodeNumber = inodeNumber;
        this.fileType = fileType;
        this.mode = mode;
        this.owner = owner;
        this.group = group;
        this.linkCount = linkCount;
        this.changeTime = changeTime;
        this.modificationTime = modificationTime;
        this.accessTime = accessTime;
        this.fileSize = fileSize;
        this.numBlocks = numBlocks;
        this.directBlockNum = new int[12];
        this.directBlockNum[0] = directBlockNum1;
        this.directBlockNum[1] = directBlockNum2;
        this.directBlockNum[2] = directBlockNum3;
        this.directBlockNum[3] = directBlockNum4;
        this.directBlockNum[4] = directBlockNum5;
        this.directBlockNum[5] = directBlockNum6;
        this.directBlockNum[6] = directBlockNum7;
        this.directBlockNum[7] = directBlockNum8;
        this.directBlockNum[8] = directBlockNum9;
        this.directBlockNum[9] = directBlockNum10;
        this.directBlockNum[10] = directBlockNum11;
        this.directBlockNum[11] = directBlockNum12;
        this.singleIndirectBlockNum = singleIndirectBlockNum;
        this.doubleIndirectBlockNum = doubleIndirectBlockNum;
        this.tripleIndirectBlockNum = tripleIndirectBlockNum;
    }

    public int getInodeNumber() {
        return inodeNumber;
    }

    public String getFileType() {
        return fileType;
    }

    public String getMode() {
        return mode;
    }

    public int getOwner() {
        return owner;
    }

    public int getGroup() {
        return group;
    }

    public int getLinkCount() {
        return linkCount;
    }

    public String getChangeTime() {
        return changeTime;
    }

    public String getModificationTime() {
        return modificationTime;
    }

    public String getAccessTime() {
        return accessTime;
    }

    public int getFileSize() {
        return fileSize;
    }

    public int getNumBlocks() {
        return numBlocks;
    }

    public int getDirectBlockNum(int index) {
        return directBlockNum[index];
    }

    public int getSingleIndirectBlockNum() {
        return singleIndirectBlockNum;
    }

    public int getDoubleIndirectBlockNum() {
        return doubleIndirectBlockNum;
    }

    public int getTripleIndirectBlockNum() {
        return tripleIndirectBlockNum;
    }
}
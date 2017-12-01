package lab3b.infoobject;

public class DirEnt {
    private int parentInodeNumber;
    private int logicalByteOffset;
    private int referencedInode;
    private int entryLength;
    private int nameLength;
    private String name;

    public DirEnt(int parentInodeNumber, int logicalByteOffset, int referencedInode, int entryLength, int nameLength, String name) {
        this.parentInodeNumber = parentInodeNumber;
        this.logicalByteOffset = logicalByteOffset;
        this.referencedInode = referencedInode;
        this.entryLength = entryLength;
        this.nameLength = nameLength;
        this.name = name;
    }

    public int getParentInodeNumber() {
        return parentInodeNumber;
    }

    public int getLogicalByteOffset() {
        return logicalByteOffset;
    }

    public int getReferencedInode() {
        return referencedInode;
    }

    public int getEntryLength() {
        return entryLength;
    }

    public int getNameLength() {
        return nameLength;
    }

    public String getName() {
        return name;
    }
}

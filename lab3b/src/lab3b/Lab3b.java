package lab3b;

import lab3b.infoobject.*;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.util.ArrayList;
import java.util.List;

public class Lab3b {
    private static final int SIZE_OF_INT32 = 4;

    private List<Bfree> bfreeList;
    private List<DirEnt> dirEntList;
    private List<Group> groupList;
    private List<Ifree> ifreeList;
    private List<Indirect> indirectList;
    private List<Inode> inodeList;
    private List<Superblock> superblockList;


    public Lab3b(String fileName) {
        try {
            BufferedReader reader = new BufferedReader(new FileReader(new File(System.getProperty("user.dir"), fileName)));
            String line;
            while ((line = reader.readLine()) != null) {
                String[] parts = line.split(",");
                switch (parts[0]) {
                    case "BFREE":
                        bfreeList.add(new Bfree(Integer.parseInt(parts[1])));
                        break;
                    case "DIRENT":
                        dirEntList.add(new DirEnt(
                                Integer.parseInt(parts[1]),
                                Integer.parseInt(parts[2]),
                                Integer.parseInt(parts[3]),
                                Integer.parseInt(parts[4]),
                                Integer.parseInt(parts[5]),
                                parts[6]
                        ));
                        break;
                    case "GROUP":
                        groupList.add(new Group(
                                Integer.parseInt(parts[1]),
                                Integer.parseInt(parts[2]),
                                Integer.parseInt(parts[3]),
                                Integer.parseInt(parts[4]),
                                Integer.parseInt(parts[5]),
                                Integer.parseInt(parts[6]),
                                Integer.parseInt(parts[7]),
                                Integer.parseInt(parts[8])
                        ));
                        break;
                    case "IFREE":
                        ifreeList.add(new Ifree(Integer.parseInt(parts[1])));
                        break;
                    case "INDIRECT":
                        indirectList.add(new Indirect(
                                Integer.parseInt(parts[1]),
                                Integer.parseInt(parts[2]),
                                Integer.parseInt(parts[3]),
                                Integer.parseInt(parts[4]),
                                Integer.parseInt(parts[5])
                        ));
                        break;
                    case "INODE":
                        inodeList.add(new Inode(
                                Integer.parseInt(parts[1]),
                                parts[2],
                                parts[3],
                                Integer.parseInt(parts[4]),
                                Integer.parseInt(parts[5]),
                                Integer.parseInt(parts[6]),
                                parts[7],
                                parts[8],
                                parts[9],
                                Integer.parseInt(parts[10]),
                                Integer.parseInt(parts[11]),
                                Integer.parseInt(parts[12]),
                                Integer.parseInt(parts[13]),
                                Integer.parseInt(parts[14]),
                                Integer.parseInt(parts[15]),
                                Integer.parseInt(parts[16]),
                                Integer.parseInt(parts[17]),
                                Integer.parseInt(parts[18]),
                                Integer.parseInt(parts[19]),
                                Integer.parseInt(parts[20]),
                                Integer.parseInt(parts[21]),
                                Integer.parseInt(parts[22]),
                                Integer.parseInt(parts[23]),
                                Integer.parseInt(parts[24]),
                                Integer.parseInt(parts[25]),
                                Integer.parseInt(parts[26])
                        ));
                        break;
                    case "SUPERBLOCK":
                        superblockList.add(new Superblock(
                                Integer.parseInt(parts[1]),
                                Integer.parseInt(parts[2]),
                                Integer.parseInt(parts[3]),
                                Integer.parseInt(parts[4]),
                                Integer.parseInt(parts[5]),
                                Integer.parseInt(parts[6]),
                                Integer.parseInt(parts[7])
                        ));
                        break;
                }
            }
            reader.close();
        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
            System.exit(2);
        }
    }

    private static void printUsage() {
        System.err.println("Usage: java -cp CLASSPATH lab3b.Lab3b FileSystemSummary.csv");
        System.exit(1);
    }

    public static void main(String[] args) {
        if (args.length != 1 || args[0].charAt(0) == '-')
            printUsage();

        Lab3b lab = new Lab3b(args[0]);
        lab.checkFileSystem();

        System.exit(0);
    }

    public void checkFileSystem() {
        checkInvalidOrReservedBlocks();
    }

    private void checkInvalidOrReservedBlocks() {
        List<Integer> reservedList = new ArrayList<>();

        Superblock superblock = superblockList.get(0);
        Group group = groupList.get(0);
        int firstDataBlock = group.getFirstInodeBlockNum()
                + (int) Math.ceil((double) (superblock.getInodeSize() * group.getTotalInodes()) / superblock.getBlockSize());

        for (int i = 1; i < firstDataBlock; i++)
            reservedList.add(i);

        int addressesInBlock = superblock.getBlockSize() / SIZE_OF_INT32;

        inodeList.forEach(inode -> {
            for (int i = 0; i < 12; i++) {
                if (reservedList.contains(inode.getDirectBlockNum(i)))
                    System.out.println("RESERVED BLOCK " + inode.getDirectBlockNum(i) + " IN INODE " + inode.getInodeNumber() + " AT OFFSET " + i);
                if (inode.getDirectBlockNum(i) < 0 || inode.getDirectBlockNum(i) >= group.getTotalBlocks())
                    System.out.println("INVALID BLOCK " + inode.getDirectBlockNum(i) + " IN INODE " + inode.getInodeNumber() + " AT OFFSET " + i);
            }
            if (reservedList.contains(inode.getSingleIndirectBlockNum()))
                System.out.println("RESERVED INDIRECT BLOCK " + inode.getSingleIndirectBlockNum() + " IN INODE " + inode.getInodeNumber() + " AT OFFSET 12");
            if (inode.getSingleIndirectBlockNum() < 0 || inode.getSingleIndirectBlockNum() >= group.getTotalBlocks())
                System.out.println("INVALID INDIRECT BLOCK " + inode.getSingleIndirectBlockNum() + " IN INODE " + inode.getInodeNumber() + " AT OFFSET 12");

            int offset = 12 + addressesInBlock;
            if (reservedList.contains(inode.getDoubleIndirectBlockNum()))
                System.out.println("RESERVED DOUBLE INDIRECT BLOCK " + inode.getDoubleIndirectBlockNum() + " IN INODE " + inode.getInodeNumber() + " AT OFFSET " + offset);
            if (inode.getDoubleIndirectBlockNum() < 0 || inode.getDoubleIndirectBlockNum() >= group.getTotalBlocks())
                System.out.println("INVALID DOUBLE INDIRECT BLOCK " + inode.getDoubleIndirectBlockNum() + " IN INODE " + inode.getInodeNumber() + " AT OFFSET " + offset);

            offset += addressesInBlock * addressesInBlock;
            if (reservedList.contains(inode.getTripleIndirectBlockNum()))
                System.out.println("RESERVED TRIPLE INDIRECT BLOCK " + inode.getTripleIndirectBlockNum() + " IN INODE " + inode.getInodeNumber() + " AT OFFSET " + offset);
            if (inode.getTripleIndirectBlockNum() < 0 || inode.getTripleIndirectBlockNum() >= group.getTotalBlocks())
                System.out.println("INVALID TRIPLE INDIRECT BLOCK " + inode.getTripleIndirectBlockNum() + " IN INODE " + inode.getInodeNumber() + " AT OFFSET " + offset);
        });

        indirectList.forEach(indirect -> {
            String referencedIndirectionLevel = "";
            if (indirect.getIndirectionLevel() == 1)
                referencedIndirectionLevel = "DIRECT";
            if (indirect.getIndirectionLevel() == 2)
                referencedIndirectionLevel = "SINGLE INDIRECT";
            if (indirect.getIndirectionLevel() == 3)
                referencedIndirectionLevel = "DOUBLE INDIRECT";

            if (reservedList.contains(indirect.getReferencedBlock()) || indirect.getReferencedBlock() == 0)
                System.out.println("RESERVED " + referencedIndirectionLevel + " BLOCK " + indirect.getReferencedBlock() + " IN INODE " + indirect.getParentInode() + " AT OFFSET " + indirect.getLogicalBlockOffset());
            if (indirect.getReferencedBlock() < 0 || indirect.getReferencedBlock() >= group.getTotalBlocks())
                System.out.println("INVALID " + referencedIndirectionLevel + " BLOCK " + indirect.getReferencedBlock() + " IN INODE " + indirect.getParentInode() + " AT OFFSET " + indirect.getLogicalBlockOffset());
        });
    }


}
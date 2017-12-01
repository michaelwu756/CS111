package lab3b;

import lab3b.infoobject.*;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.util.ArrayList;
import java.util.List;
import java.util.stream.Collectors;

public class Lab3b {
    private static final int SIZE_OF_INT32 = 4;

    private List<Bfree> bfreeList;
    private List<DirEnt> dirEntList;
    private List<Group> groupList;
    private List<Ifree> ifreeList;
    private List<Indirect> indirectList;
    private List<Inode> inodeList;
    private List<Superblock> superblockList;

    private int firstDataBlock;
    private int totalBlocks;
    private int addressesInBlock;


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
        Superblock superblock = superblockList.get(0);
        Group group = groupList.get(0);
        firstDataBlock = group.getFirstInodeBlockNum()
                + (int) Math.ceil((double) (superblock.getInodeSize() * group.getTotalInodes()) / superblock.getBlockSize());
        totalBlocks=group.getTotalBlocks();
        addressesInBlock = superblockList.get(0).getBlockSize() / SIZE_OF_INT32;
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

    private void checkFileSystem() {
        checkInvalidOrReservedBlocks();
        checkUnreferencedBlocks();
        checkReferencedBlocksOnFreeList();
        checkMultipleReferencedBlocks();
    }

    private void checkInvalidOrReservedBlocks() {
        List<Integer> reservedList = new ArrayList<>();
        for (int i = 1; i < firstDataBlock; i++)
            reservedList.add(i);

        inodeList.forEach(inode -> {
            for (int i = 0; i < 12; i++) {
                if (reservedList.contains(inode.getDirectBlockNum(i)))
                    System.out.println("RESERVED BLOCK " + inode.getDirectBlockNum(i) + " IN INODE " + inode.getInodeNumber() + " AT OFFSET " + i);
                if (inode.getDirectBlockNum(i) < 0 || inode.getDirectBlockNum(i) >= totalBlocks)
                    System.out.println("INVALID BLOCK " + inode.getDirectBlockNum(i) + " IN INODE " + inode.getInodeNumber() + " AT OFFSET " + i);
            }
            int offset=12;
            if (reservedList.contains(inode.getSingleIndirectBlockNum()))
                System.out.println("RESERVED INDIRECT BLOCK " + inode.getSingleIndirectBlockNum() + " IN INODE " + inode.getInodeNumber() + " AT OFFSET "+offset);
            if (inode.getSingleIndirectBlockNum() < 0 || inode.getSingleIndirectBlockNum() >= totalBlocks)
                System.out.println("INVALID INDIRECT BLOCK " + inode.getSingleIndirectBlockNum() + " IN INODE " + inode.getInodeNumber() + " AT OFFSET "+offset);

            offset+= addressesInBlock;
            if (reservedList.contains(inode.getDoubleIndirectBlockNum()))
                System.out.println("RESERVED DOUBLE INDIRECT BLOCK " + inode.getDoubleIndirectBlockNum() + " IN INODE " + inode.getInodeNumber() + " AT OFFSET " + offset);
            if (inode.getDoubleIndirectBlockNum() < 0 || inode.getDoubleIndirectBlockNum() >= totalBlocks)
                System.out.println("INVALID DOUBLE INDIRECT BLOCK " + inode.getDoubleIndirectBlockNum() + " IN INODE " + inode.getInodeNumber() + " AT OFFSET " + offset);

            offset += addressesInBlock * addressesInBlock;
            if (reservedList.contains(inode.getTripleIndirectBlockNum()))
                System.out.println("RESERVED TRIPLE INDIRECT BLOCK " + inode.getTripleIndirectBlockNum() + " IN INODE " + inode.getInodeNumber() + " AT OFFSET " + offset);
            if (inode.getTripleIndirectBlockNum() < 0 || inode.getTripleIndirectBlockNum() >= totalBlocks)
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
            if (indirect.getReferencedBlock() < 0 || indirect.getReferencedBlock() >= totalBlocks)
                System.out.println("INVALID " + referencedIndirectionLevel + " BLOCK " + indirect.getReferencedBlock() + " IN INODE " + indirect.getParentInode() + " AT OFFSET " + indirect.getLogicalBlockOffset());
        });
    }

    private void checkUnreferencedBlocks() {
        List<Integer> unreferencedBlocks = new ArrayList<>();
        for (int i = firstDataBlock; i < totalBlocks; i++)
            unreferencedBlocks.add(i);

        bfreeList.forEach(bfree -> unreferencedBlocks.remove(Integer.valueOf(bfree.getBlockNum())));

        inodeList.forEach(inode -> {
            for (int i = 0; i < 12; i++)
                unreferencedBlocks.remove(Integer.valueOf(inode.getDirectBlockNum(i)));
            unreferencedBlocks.remove(Integer.valueOf(inode.getSingleIndirectBlockNum()));
            unreferencedBlocks.remove(Integer.valueOf(inode.getDoubleIndirectBlockNum()));
            unreferencedBlocks.remove(Integer.valueOf(inode.getTripleIndirectBlockNum()));
        });

        indirectList.forEach(indirect -> unreferencedBlocks.remove(Integer.valueOf(indirect.getReferencedBlock())));

        unreferencedBlocks.forEach(blockNum -> System.out.println("UNREFERENCED BLOCK " + blockNum));
    }

    private void checkReferencedBlocksOnFreeList() {
        List<Integer> freeList = bfreeList.stream().map(Bfree::getBlockNum).collect(Collectors.toList());
        inodeList.forEach(inode -> {
            for (int i = 0; i < 12; i++)
                if (freeList.contains(inode.getDirectBlockNum(i)))
                    System.out.println("ALLOCATED BLOCK " + inode.getDirectBlockNum(i) + " ON FREELIST");
            if (freeList.contains(inode.getSingleIndirectBlockNum()))
                System.out.println("ALLOCATED BLOCK " + inode.getSingleIndirectBlockNum() + " ON FREELIST");
            if (freeList.contains(inode.getDoubleIndirectBlockNum()))
                System.out.println("ALLOCATED BLOCK " + inode.getDoubleIndirectBlockNum() + " ON FREELIST");
            if (freeList.contains(inode.getTripleIndirectBlockNum()))
                System.out.println("ALLOCATED BLOCK " + inode.getTripleIndirectBlockNum() + " ON FREELIST");
        });

        indirectList.forEach(indirect -> {
            if (freeList.contains(indirect.getReferencedBlock()))
                System.out.println("ALLOCATED BLOCK " + indirect.getReferencedBlock() + " ON FREELIST");
        });
    }

    private void checkMultipleReferencedBlocks()
    {
        List<Integer> blocksReferenced = new ArrayList<>();
        List<Integer> duplicateBlocks = new ArrayList<>();

        inodeList.forEach(inode -> {
            for (int i = 0; i < 12; i++)
                if (inode.getDirectBlockNum(i)>=firstDataBlock && inode.getDirectBlockNum(i)<totalBlocks) {
                    if(blocksReferenced.contains(inode.getDirectBlockNum(i)))
                        duplicateBlocks.add(inode.getDirectBlockNum(i));
                    else
                        blocksReferenced.add(inode.getDirectBlockNum(i));
                }
            if (inode.getSingleIndirectBlockNum()>=firstDataBlock && inode.getSingleIndirectBlockNum()<totalBlocks) {
                if(blocksReferenced.contains(inode.getSingleIndirectBlockNum()))
                    duplicateBlocks.add(inode.getSingleIndirectBlockNum());
                else
                    blocksReferenced.add(inode.getSingleIndirectBlockNum());
            }
            if (inode.getDoubleIndirectBlockNum()>=firstDataBlock && inode.getDoubleIndirectBlockNum()<totalBlocks) {
                if(blocksReferenced.contains(inode.getDoubleIndirectBlockNum()))
                    duplicateBlocks.add(inode.getDoubleIndirectBlockNum());
                else
                    blocksReferenced.add(inode.getDoubleIndirectBlockNum());
            }
            if (inode.getTripleIndirectBlockNum()>=firstDataBlock && inode.getTripleIndirectBlockNum()<totalBlocks) {
                if(blocksReferenced.contains(inode.getTripleIndirectBlockNum()))
                    duplicateBlocks.add(inode.getTripleIndirectBlockNum());
                else
                    blocksReferenced.add(inode.getTripleIndirectBlockNum());
            }
        });

        indirectList.forEach(indirect -> {
            if (indirect.getReferencedBlock()>=firstDataBlock && indirect.getReferencedBlock()<totalBlocks) {
                if(blocksReferenced.contains(indirect.getReferencedBlock()))
                    duplicateBlocks.add(indirect.getReferencedBlock());
                else
                    blocksReferenced.add(indirect.getReferencedBlock());
            }
        });

        inodeList.forEach(inode -> {
            for (int i = 0; i < 12; i++)
                if(duplicateBlocks.contains(inode.getDirectBlockNum(i)))
                    System.out.println("DUPLICATE BLOCK "+inode.getDirectBlockNum(i)+" IN INODE "+inode.getInodeNumber()+" AT OFFSET "+i);

            int offset=12;
            if(duplicateBlocks.contains(inode.getSingleIndirectBlockNum()))
                System.out.println("DUPLICATE INDIRECT BLOCK "+inode.getSingleIndirectBlockNum()+" IN INODE "+inode.getInodeNumber()+" AT OFFSET "+offset);

            offset+=addressesInBlock;
            if(duplicateBlocks.contains(inode.getDoubleIndirectBlockNum())))
                System.out.println("DUPLICATE DOUBLE INDIRECT BLOCK "+inode.getDoubleIndirectBlockNum()+" IN INODE "+inode.getInodeNumber()+" AT OFFSET "+offset);

            offset+=addressesInBlock*addressesInBlock;
            if(duplicateBlocks.contains(inode.getTripleIndirectBlockNum()))
                System.out.println("DUPLICATE TRIPLE INDIRECT BLOCK "+inode.getTripleIndirectBlockNum()+" IN INODE "+inode.getInodeNumber()+" AT OFFSET "+offset);
        });

        indirectList.forEach(indirect -> {
            String referencedIndirectionLevel = "";
            if (indirect.getIndirectionLevel() == 1)
                referencedIndirectionLevel = "DIRECT";
            if (indirect.getIndirectionLevel() == 2)
                referencedIndirectionLevel = "SINGLE INDIRECT";
            if (indirect.getIndirectionLevel() == 3)
                referencedIndirectionLevel = "DOUBLE INDIRECT";
            if (duplicateBlocks.contains(indirect.getReferencedBlock()))
                System.out.println("DUPLICATE "+referencedIndirectionLevel+" BLOCK "+indirect.getReferencedBlock()+" IN INODE "+indirect.getParentInode()+" AT OFFSET "+indirect.getLogicalBlockOffset());
        });

    }

}
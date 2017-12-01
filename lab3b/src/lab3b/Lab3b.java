package lab3b;

import lab3b.infoobject.*;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.util.List;

public class Lab3b
{
    private List<Bfree> bfreeList;
    private List<DirEnt> dirEntList;
    private List<Group> groupList;
    private List<Ifree> ifreeList;
    private List<Indirect> indirectList;
    private List<Inode> inodeList;
    private List<Superblock> superblockList;

    public Lab3b(String fileName)
    {
        try {
           BufferedReader reader = new BufferedReader(new FileReader(new File(System.getProperty("user.dir"), fileName)));
           String line;
           while((line=reader.readLine())!=null) {
               String[] parts = line.split(",");
               switch(parts[0]){
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
        }
        catch(Exception e) {
            System.err.println("Error: "+e.getMessage());
            System.exit(2);
        }
    }

    public void checkFileSystem()
    {
    }

    private static void printUsage()
    {
        System.err.println("Usage: java -cp CLASSPATH lab3b.Lab3b FileSystemSummary.csv");
        System.exit(1);
    }
    public static void main(String[] args)
    {
        if(args.length!=1 || args[0].charAt(0)=='-')
            printUsage();

        Lab3b lab = new Lab3b(args[0]);
        lab.checkFileSystem();

        System.exit(0);
    }
}
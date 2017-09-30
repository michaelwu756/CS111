#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

void segFaultHandler(int sig)
{
  fprintf(stderr, "signal %i handled segfault\n", sig);
  exit(4);
}

int main(int argc, char* argv[])
{
  
  char c;
  static struct option long_options[] =
  {
    {"input", required_argument, 0, 'i'},
    {"output", required_argument, 0, 'o'},
    {"segfault", no_argument, 0, 's'},
    {"catch", no_argument, 0, 'c'},
    {0, 0, 0, 0}
  };

  int fd=0;
  int fault=0;
  while((c=getopt_long(argc, argv, "", long_options, 0)) != -1)
  {
    switch(c){
      case 'i':
	fd=open(optarg, O_RDONLY);
	if(fd==-1)
	{
	  fprintf(stderr, "Input file %s error: %s\n", optarg, strerror(errno));
	  exit(2);
	}
	close(0);
	dup(fd);
	close(fd);
	break;
      case 'o':
	fd=open(optarg, O_WRONLY|O_CREAT);
	if(fd==-1)
        {
	  fprintf(stderr, "Output file %s error: %s\n", optarg, strerror(errno));
          exit(3);
        }
	close(1);
	dup(fd);
	close(fd);
	break;
      case 's':
	fault=1;
	break;
      case 'c':
	signal(SIGSEGV, segFaultHandler);
	break;
      default:
	fprintf(stderr, "Usage: %s [--input=FILE --output=FILE --segfault --catch]\n", argv[0]);
	exit(1);
    }
  }


  if(fault)
  {
    char* faultPtr = NULL;
    *faultPtr='a';
  }
  while(read(0, &c, 1)!=0)
  {
    int numWritten = 0;
    while(numWritten!=1)
      numWritten=write(1, &c, 1);
  }
  exit(0);
}

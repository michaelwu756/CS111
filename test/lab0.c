#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

void segFaultHandler(int sig)
{
  fprintf(stderr, "handled segfault");
  exit(3);
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

  int fd0=0;
  int fd1=1;
  int fault=0;
  while((c=getopt_long(argc, argv, "", long_options, 0)) != -1)
  {
    switch(c){
      case 'i':
	fd0=open(optarg, O_RDONLY);
	if(fd0==-1)
	{
	  perror("Error");
	  exit(1);
	}
	break;
      case 'o':
	fd1=open(optarg, O_WRONLY|O_CREAT);
	if(fd1==-1)
        {
          perror("Error");
          exit(2);
        }
	break;
      case 's':
	fault=1;
	break;
      case 'c':
	signal(SIGSEGV, segFaultHandler);
	break;
    }
  }


  if(fault)
  {
    char* faultPtr = NULL;
    *faultPtr='a';
  }
  while(read(fd0, &c, 1)!=0)
  {
    write(fd1, &c, 1);
  }
  exit(0);
}

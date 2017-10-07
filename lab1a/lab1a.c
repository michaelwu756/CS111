#include<termios.h>
#include<errno.h>
#include<string.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>

struct termios originalTerminalAttributes;
void resetTerminal()
{
  tcsetattr(STDIN_FILENO, TCSANOW, &originalTerminalAttributes);
}

int main(int argc, char *argv[])
{
  char opt;
  static struct option long_options[] =
  {
    {"shell", no_argument, 0, 's'},
    {0, 0, 0, 0}
  };

  int pipefd[2];
  int pipe2fd[2];
  pid_t childpid=0;
  while((opt=getopt_long(argc, argv, "", long_options, 0)) != -1)
  {
    switch(c){
      case 's':
	if(pipe(pipefd)==-1)
	{
	  fprintf(stderr, "Error making pipe 1: %s", strerror(errno));
	  exit(1);
        }
	if(pipe(pipe2fd)==-1)
	{
	  fprintf(stderr, "Error making pipe 2: %s", strerror(errno));
	  exit(1);
        }
	pid_t pid=fork();
	if(pid==-1)
	{
	  fprintf(stderr, "Error forking: %s", strerror(errno));
	  exit(1);
	}
	else if(pid==0)
	{
	  close(STDIN_FILENO);
	  dup(pipefd[0]);
	  close(pipefd[0]);
	  close(pipefd[1]);

	  close(STDOUT_FILENO);
	  dup(pipe2fd[1]);
	  close(pipe2fd[0]);
	  close(pipe2fd[1]);
	}
	else
        {
	  close(pipefd[0]);
	  close(pipe2fd[1]);
	  childpid=pid;
        }
	break;
      default:
	fprintf(stderr, "Usage: %s [--shell]\n", argv[0]);
	exit(1);
    }
  }
  
  if(tcgetattr(STDIN_FILENO, &originalTerminalAttributes)<0)
  {
    fprintf(stderr, "Error getting terminal attributes: %s", strerror(errno));
    exit(1);
  }
  atexit(resetTerminal);

  struct termios newTerminalAttributes=originalTerminalAttributes;

  newTerminalAttributes.c_iflag=ISTRIP;
  newTerminalAttributes.c_oflag=0;
  newTerminalAttributes.c_lflag=0;

  if(tcsetattr(STDIN_FILENO, TCSANOW, &newTerminalAttributes)<0)
  {
    fprintf(stderr, "Error setting terminal attributes: %s", strerror(errno));
    exit(1);
  }  

  char buf[10];
  int numRead;

  while((numRead = read(STDIN_FILENO, buf, 10))>0)
  {
    int i;
    for(i=0; i<numRead; i++)
    {
      char c = buf[i];
      int numWritten=0;
      if(c=='\004')
	exit(0);
      else if(c=='\r' || c=='\n')
	do{
	  numWritten+=write(STDOUT_FILENO, "\r\n", 2);
	}while(numWritten!=2);
      else
	do{
	  numWritten+=write(STDOUT_FILENO, &c, 1);
	}while(numWritten!=1);
    }
  }
  return 0;
}

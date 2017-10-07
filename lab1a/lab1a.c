#include<termios.h>
#include<errno.h>
#include<string.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>

struct termios originalTerminalAttributes;
void resetTerminal()
{
  tcsetattr(STDIN_FILENO, TCSANOW, &originalTerminalAttributes);
}

int main(int argc, char *argv[])
{
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

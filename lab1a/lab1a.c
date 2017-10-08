#include<termios.h>
#include<errno.h>
#include<string.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<getopt.h>
#include<poll.h>
#include<signal.h>

struct termios originalTerminalAttributes;
void resetTerminal()
{
  tcsetattr(STDIN_FILENO, TCSANOW, &originalTerminalAttributes);
}

void checkForError(int result, char* message)
{
  if(result==-1)
  {
    fprintf(stderr, "Error %s: %s", message, strerror(errno));
    exit(1);
  }
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
    switch(opt){
      case 's':
	checkForError(pipe(pipefd), "making pipe 1");
	checkForError(pipe(pipe2fd), "making pipe 2");
	
	pid_t pid=fork();
	checkForError(pid, "forking");
	if(pid==0)
	{
	  checkForError(close(STDIN_FILENO), "closing stdin");
	  checkForError(dup(pipefd[0]), "duplicating pipefd[0]");
	  checkForError(close(pipefd[0]), "closing pipefd[0]");
	  checkForError(close(pipefd[1]), "closing pipefd[1]");

	  checkForError(close(STDOUT_FILENO), "closing stdout");
	  checkForError(close(STDERR_FILENO), "closing stderr");
	  checkForError(dup(pipe2fd[1]), "duplicating pipe2fd[1]");
	  checkForError(dup(pipe2fd[1]), "duplicating pipe2fd[1]");
	  checkForError(close(pipe2fd[0]), "closing pipe2fd[0]");
	  checkForError(close(pipe2fd[1]), "closing pipe2fd[1]");

	  checkForError(execl("/bin/bash", "/bin/bash", (char*) NULL), "executing");
	}
	else
        {
	  checkForError(close(pipefd[0]), "closing pipefd[0]");
	  checkForError(close(pipe2fd[1]), "closing pipe2fd[1]");
	  childpid=pid;
        }
	break;
      default:
	fprintf(stderr, "Usage: %s [--shell]\n", argv[0]);
	exit(1);
    }
  }
  
  checkForError(tcgetattr(STDIN_FILENO, &originalTerminalAttributes), "getting terminal attributes");
  atexit(resetTerminal);

  struct termios newTerminalAttributes=originalTerminalAttributes;

  newTerminalAttributes.c_iflag=ISTRIP;
  newTerminalAttributes.c_oflag=0;
  newTerminalAttributes.c_lflag=0;

  checkForError(tcsetattr(STDIN_FILENO, TCSANOW, &newTerminalAttributes), "setting terminal attributes");

  char buf[10];
  int numRead;
  char shellBuf[256];
  int shellRead;
  int i;
  if(childpid!=0)
  {
    struct pollfd pollingArr[2]={{STDIN_FILENO,POLLIN,0},{pipe2fd[0],POLLIN,0}};
    while(1)
    {
      int pollResult = poll(pollingArr, 2, 0);
      checkForError(pollResult, "polling");
      if (pollResult>0)
      {
	if(pollingArr[0].revents == POLLIN)
	{
	  numRead=read(STDIN_FILENO, buf, 10);
	  checkForError(numRead, "reading from keyboard");
	  for(i=0; i<numRead; i++)
	  {
	    char c = buf[i];
	    if (c=='\003')
	    {
	      checkForError(kill(childpid, SIGINT), "killing shell");
	      checkForError(write(STDOUT_FILENO, "^C", 2), "writing from keyboard to stdout");
	    }
	    else if(c=='\004')
	    {
	      checkForError(write(STDOUT_FILENO, "^D", 2), "writing from keyboard to stdout");
	      checkForError(close(pipefd[1]), "closing pipefd[1]");
	    }
	    else if (c=='\r' || c=='\n')
	    {
	      checkForError(write(STDOUT_FILENO, "\r\n", 2), "writing from keyboard to stdout");
	      checkForError(write(pipefd[1], "\n", 1), "writing from keyboard to shell");    
	    }
	    else
	    {
	      checkForError(write(STDOUT_FILENO, &buf[i], 1), "writing from keyboard to stdout");
	      checkForError(write(pipefd[1], &buf[i], 1), "writing from keyboard to shell");    
	    }
	  }
	}
	if(pollingArr[1].revents == POLLIN)
	{
	  shellRead=read(pipe2fd[0], shellBuf, 256);
	  checkForError(shellRead, "reading from shell");
	  for(i=0; i<shellRead; i++)
	  {
	    char c = shellBuf[i];
	    if(c=='\n')  
	      checkForError(write(STDOUT_FILENO, "\r\n", 2), "writing from shell to stdout");
	    else
	      checkForError(write(STDOUT_FILENO, &c, 1), "writing from shell to stdout");
	  }
	}
      }
    }
  }
  else
  {
    while(1)
    {
      numRead=read(STDIN_FILENO, buf, 10);
      checkForError(numRead, "reading from keyboard");
      for(i=0; i<numRead; i++)
      {
	char c = buf[i];
	if(c=='\004')
	  exit(0);
	else if(c=='\r' || c=='\n')
	  checkForError(write(STDOUT_FILENO, "\r\n", 2), "writing to screen");
	else
	  checkForError(write(STDOUT_FILENO, &c, 2), "writing to screen");
      }
    }
  }
  return 0;
}

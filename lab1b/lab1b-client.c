#include<termios.h>
#include<errno.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<getopt.h>
#include<poll.h>
#include<signal.h>
#include<sys/socket.h>
#include<sys/stat.h>
#include<string.h>
#include<fcntl.h>
#include<netinet/in.h>
#include<arpa/inet.h>


struct termios originalTerminalAttributes;
int socketfd;
void resetTerminal()
{
  if(tcsetattr(STDIN_FILENO, TCSANOW, &originalTerminalAttributes)==-1)
    fprintf(stderr, "\r\nError restoring terminal attributes: %s\r\n", strerror(errno));
}

void closeSocket()
{
  if(close(socketfd)==-1)
    fprintf(stderr, "\r\nError closing socket: %s\r\n", strerror(errno));
}

void checkForError(int result, char* message)
{
  if(result==-1)
  {
    fprintf(stderr, "\r\nError %s: %s\r\n", message, strerror(errno));
    exit(1);
  }
}

void sigpipeHandler(int sig)
{
  fprintf(stderr, "Caught sigpipe signal %d", sig);
  exit(0);
}

void printUsage(char *progName)
{
  fprintf(stderr, "Usage: %s --port=N [--log=FILE]\n", progName);
  exit(1);
}

int main(int argc, char *argv[])
{
  char opt;
  static struct option long_options[] =
  {
    {"port", required_argument, 0, 'p'},
    {"log", required_argument, 0, 'l'},
    {0, 0, 0, 0}
  };

  struct sockaddr_in sockaddr;
  sockaddr.sin_port=0;
  int logfd=-1;
  while((opt=getopt_long(argc, argv, "", long_options, 0)) != -1)
  {
    switch(opt){
      case 'p':
	sockaddr.sin_port = atoi(optarg);
	if(signal(SIGPIPE, sigpipeHandler) == SIG_ERR)
	{
	  fprintf(stderr, "Error registering sigpipe handler: %s", strerror(errno));
	  exit(1);
	}
	break;
      case 'l':
	logfd=open(optarg, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
	checkForError(logfd, "opening logfile");
	break;
      default:
	printUsage(argv[0]);
    }
  }
  if(sockaddr.sin_port==0)
    printUsage(argv[0]);

  sockaddr.sin_family=AF_INET;
  checkForError(inet_aton("127.0.0.1", &(sockaddr.sin_addr)), "getting internet address");
  
  socketfd=socket(AF_INET,SOCK_STREAM,0);
  checkForError(socketfd,"opening socket");
  checkForError(connect(socketfd, (struct sockaddr *)&sockaddr, sizeof(sockaddr)), "connecting to socket");
  atexit(closeSocket);

  checkForError(tcgetattr(STDIN_FILENO, &originalTerminalAttributes), "getting terminal attributes");
  atexit(resetTerminal);

  struct termios newTerminalAttributes=originalTerminalAttributes;

  newTerminalAttributes.c_iflag=ISTRIP;
  newTerminalAttributes.c_oflag=0;
  newTerminalAttributes.c_lflag=0;

  checkForError(tcsetattr(STDIN_FILENO, TCSANOW, &newTerminalAttributes), "setting terminal attributes");

  char buf[256];
  int numRead;
  int i;
  struct pollfd pollingArr[2]={{STDIN_FILENO,POLLIN,0},{socketfd,POLLIN,0}};
  while(1)
  {
    int pollResult = poll(pollingArr, 2, 0);
    checkForError(pollResult, "polling");
    if (pollResult>0)
    {
      if(pollingArr[0].revents & POLLIN)
      {
	numRead=read(STDIN_FILENO, buf, 256);
	checkForError(numRead, "reading from keyboard");
	for(i=0; i<numRead; i++)
	{
	  char c = buf[i];
	  if(c=='\r' || c=='\n')
	    checkForError(write(STDOUT_FILENO, "\r\n", 2), "writing from keyboard to stdout");
	  else
	    checkForError(write(STDOUT_FILENO, &c, 1), "writing from keyboard to stdout");
	  checkForError(write(socketfd, &c, 1), "writing from keyboard to socket");
	}
      }
      if(pollingArr[1].revents & POLLIN)
      {
	numRead=read(socketfd, buf, 256);
	checkForError(numRead, "reading from socket");
	if(numRead==0)
	  exit(0);
	for(i=0; i<numRead; i++)
	{
	  char c = buf[i];
	  if(c=='\n')
	    checkForError(write(STDOUT_FILENO, "\r\n", 2), "writing from socket to stdout");
	  else
	    checkForError(write(STDOUT_FILENO, &c, 1), "writing from socket to stdout");
	}
      }
      else if(pollingArr[1].revents & (POLLHUP|POLLERR))
	exit(0);
    }
  }

  return 0;
}

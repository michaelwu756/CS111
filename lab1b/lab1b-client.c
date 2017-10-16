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
#include<mcrypt.h>

struct termios originalTerminalAttributes;
int socketfd;
int logfd=-1;
MCRYPT tdEncrypt;
MCRYPT tdDecrypt;
char *IVEncrypt;
char *IVDecrypt;

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

void closeLog()
{
  if(close(logfd)==-1)
    fprintf(stderr, "\r\nError closing log: %s\r\n", strerror(errno));
}

void closeEncryptionDescriptors()
{
  int result;
  result = mcrypt_module_close(tdEncrypt);
  if(result<0)
    mcrypt_perror(result);
  result = mcrypt_module_close(tdDecrypt);
  if(result<0)
    mcrypt_perror(result);
  free(IVEncrypt);
  free(IVDecrypt);
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
  fprintf(stderr, "Usage: %s --port=N [--log=FILE --encrypt=KEYFILE]\n", progName);
  exit(1);
}

void logMessage(char *prefix, int numBytes, char *message)
{
  if(logfd==-1)
    return;
  char logBuf[256];
  int numToWrite = sprintf(logBuf, "%s %d bytes: ", prefix, numBytes);
  if(numToWrite<0)
    fprintf(stderr, "\r\nError with sprintf\r\n");
  checkForError(write(logfd, logBuf, numToWrite), "writing to log");
  checkForError(write(logfd, message, numBytes), "writing to log");
  checkForError(write(logfd, "\n", 1), "writing to log");
}

int main(int argc, char *argv[])
{
  char opt;
  static struct option long_options[] =
  {
    {"port", required_argument, 0, 'p'},
    {"log", required_argument, 0, 'l'},
    {"encrypt", required_argument, 0, 'e'},
    {0, 0, 0, 0}
  };

  int encrypt=0;
  int i;
  struct sockaddr_in sockaddr;
  sockaddr.sin_port=0;
  while((opt=getopt_long(argc, argv, "", long_options, 0)) != -1)
  {
    switch(opt){
      case 'p':
        sockaddr.sin_port = htons(atoi(optarg));
        break;
      case 'l':
        logfd=open(optarg, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
        checkForError(logfd, "opening logfile");
        atexit(closeLog);
        break;
      case 'e':
        encrypt=1;
        int keyfilefd = open(optarg, O_RDONLY);
        checkForError(keyfilefd, "opening keyfile");
        char *keyBuf[32];
        int keylen = read(keyfilefd, keyBuf, 32);
        checkForError(keylen, "reading key from keyfile");
        checkForError(close(keyfilefd), "closing keyfile");

        tdEncrypt = mcrypt_module_open("twofish", NULL, "cfb", NULL);
        if (tdEncrypt==MCRYPT_FAILED)
        {
          fprintf(stderr, "Error opening encryption module");
          exit(1);
        }

        tdDecrypt = mcrypt_module_open("twofish", NULL, "cfb", NULL);
        if (tdDecrypt==MCRYPT_FAILED)
        {
          fprintf(stderr, "Error opening decryption module");
          exit(1);
        }

        IVEncrypt = malloc(mcrypt_enc_get_iv_size(tdEncrypt));
        if (IVEncrypt == NULL)
        {
          fprintf(stderr, "Error allocating IV memory");
          exit(1);
        }

        IVDecrypt = malloc(mcrypt_enc_get_iv_size(tdDecrypt));
        if (IVDecrypt == NULL)
        {
          fprintf(stderr, "Error allocating IV memory");
          exit(1);
        }

        for (i=0; i< mcrypt_enc_get_iv_size(tdEncrypt); i++) {
          IVEncrypt[i]='a';
        }

        for (i=0; i< mcrypt_enc_get_iv_size(tdEncrypt); i++) {
          IVDecrypt[i]='b';
        }

        i=mcrypt_generic_init(tdEncrypt, keyBuf, keylen, IVEncrypt);
        if (i<0) {
          mcrypt_perror(i);
          exit(1);
        }

        i=mcrypt_generic_init(tdDecrypt, keyBuf, keylen, IVDecrypt);
        if (i<0) {
          mcrypt_perror(i);
          exit(1);
        }

        atexit(closeEncryptionDescriptors);
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

  if(signal(SIGPIPE, sigpipeHandler) == SIG_ERR)
  {
    fprintf(stderr, "Error registering sigpipe handler: %s", strerror(errno));
    exit(1);
  }

  char buf[256];
  int numRead;
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
        }
        if(encrypt==1)
          mcrypt_generic(tdEncrypt, buf, numRead);
        logMessage("SENT", numRead, buf);
        for(i=0; i<numRead; i++)
        {
          char c = buf[i];
          checkForError(write(socketfd, &c, 1), "writing from keyboard to socket");
        }
      }
      if(pollingArr[1].revents & POLLIN)
      {
        numRead=read(socketfd, buf, 256);
        checkForError(numRead, "reading from socket");
        if(encrypt==1)
          mdecrypt_generic(tdDecrypt, buf, numRead);
        logMessage("RECIEVED", numRead, buf);
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

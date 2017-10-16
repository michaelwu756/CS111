#include<errno.h>
#include<string.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<getopt.h>
#include<poll.h>
#include<signal.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<mcrypt.h>
#include<fcntl.h>

MCRYPT tdEncrypt;
MCRYPT tdDecrypt;
char *IVEncrypt;
char *IVDecrypt;
int socketfd;
int connectedfd;
pid_t childpid=0;
void closeSocket()
{
  if(close(socketfd)==-1)
    fprintf(stderr, "\nError closing socket: %s\n", strerror(errno));
}

void closeConnected()
{
  if(close(connectedfd)==-1)
    fprintf(stderr, "\nError closing connection: %s\n", strerror(errno));
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
    fprintf(stderr, "\nError %s: %s\n", message, strerror(errno));
    exit(1);
  }
}

void waitForChild()
{
  int status;
  checkForError(waitpid(childpid, &status, 0), "waiting for child process to finish");
  fprintf(stderr, "\nSHELL EXIT SIGNAL=%d STATUS=%d\n", status&0x007f, status>>8);
}

void sigpipeHandler(int sig)
{
  fprintf(stderr, "Caught sigpipe signal %d", sig);
  exit(0);
}

void printUsage(char *progName)
{
  fprintf(stderr, "Usage: %s --port=N [--encrypt=KEYFILE]\n", progName);
  exit(1);
}

int main(int argc, char *argv[])
{
  char opt;
  static struct option long_options[] =
  {
    {"port", required_argument, 0, 'p'},
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

        for (i=0; i< mcrypt_enc_get_iv_size(tdEncrypt); i++)
          IVEncrypt[i]='b';

        for (i=0; i< mcrypt_enc_get_iv_size(tdEncrypt); i++)
          IVDecrypt[i]='a';
       
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
  sockaddr.sin_addr.s_addr = INADDR_ANY;

  socklen_t sockaddrsize=sizeof(sockaddr);
  socketfd=socket(AF_INET,SOCK_STREAM,0);
  checkForError(socketfd,"opening socket");
  checkForError(bind(socketfd, (struct sockaddr *)&sockaddr, sockaddrsize), "binding to socket");
  checkForError(listen(socketfd, 1), "listening to socket");
  atexit(closeSocket);

  struct pollfd socketPollingArr[1]={{socketfd,POLLIN,0}};
  checkForError(poll(socketPollingArr, 1, -1), "polling for incoming connection");
  if(socketPollingArr[0].revents & POLLIN)
  {
    connectedfd=accept(socketfd, (struct sockaddr *)&sockaddr, &sockaddrsize);
  }
  else
  {
    fprintf(stderr, "Error polling, POLLIN not recieved\n");
    exit(1);
  }
  atexit(closeConnected);

  int pipefd[2];
  int pipe2fd[2];
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
    atexit(waitForChild);
  }

  if(signal(SIGPIPE, sigpipeHandler) == SIG_ERR)
  {
    fprintf(stderr, "Error registering sigpipe handler: %s\n", strerror(errno));
    exit(1);
  }

  char buf[256];
  int numRead;
  struct pollfd pollingArr[2]={{connectedfd,POLLIN,0},{pipe2fd[0],POLLIN,0}};
  while(1)
  {
    int pollResult = poll(pollingArr, 2, 0);
    checkForError(pollResult, "polling");
    if (pollResult>0)
    {
      if(pollingArr[0].revents & POLLIN)
      {
        numRead=read(connectedfd, buf, 256);
        checkForError(numRead, "reading from socket");
        if(numRead==0)
          checkForError(close(pipefd[1]), "closing pipefd[1]");
        if(encrypt==1)
          mdecrypt_generic(tdDecrypt, buf, numRead);
        for(i=0; i<numRead; i++)
        {
          char c = buf[i];
          if (c=='\003')
            checkForError(kill(childpid, SIGINT), "killing shell");
          else if(c=='\004')
            checkForError(close(pipefd[1]), "closing pipefd[1]");
          else if (c=='\r' || c=='\n')
            checkForError(write(pipefd[1], "\n", 1), "writing from socket to shell");
          else
            checkForError(write(pipefd[1], &c, 1), "writing from socket to shell");
        }
      }
      else if (pollingArr[0].revents & (POLLHUP|POLLERR))
        checkForError(close(pipefd[1]), "closing pipefd[1]");
      if(pollingArr[1].revents & POLLIN)
      {
        numRead=read(pipe2fd[0], buf, 256);
        checkForError(numRead, "reading from shell");
        if(numRead==0)
          exit(0);
        if(encrypt==1)
          mcrypt_generic(tdEncrypt, buf, numRead);
        for(i=0; i<numRead; i++)
        {
          char c = buf[i];
          checkForError(write(connectedfd, &c, 1), "writing from shell to stdout");
        }
      }
      else if(pollingArr[1].revents & (POLLHUP|POLLERR))
        exit(0);
    }
  }
  return 0;
}

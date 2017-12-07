#include<mraa/aio.h>
#include<mraa/types.h>
#include<signal.h>
#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<errno.h>
#include<sys/timerfd.h>
#include<fcntl.h>
#include<getopt.h>
#include<string.h>
#include<poll.h>
#include<math.h>
#include<ctype.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<netdb.h>
#include<openssl/ssl.h>
#include<openssl/evp.h>
#include<openssl/err.h>
#include<openssl/x509_vfy.h>
const int B = 4275;
const int R0 = 100000;

int timerfd;
char scale;
int period;
int logfd;
int stopped=0;
int running=1;
char *id;
char *host;
SSL_CTX *ctx;
SSL* ssl;
int socketfd;
void closeSSL()
{
  if(ctx != NULL)
    SSL_CTX_free(ctx);

  if(ssl !=NULL)
  {
    SSL_shutdown(ssl);
    SSL_free(ssl);
  }

  ERR_free_strings();
  EVP_cleanup();

  if(close(socketfd)==-1)
    fprintf(stderr, "\r\nError closing socket: %s\r\n", strerror(errno));
}
void checkForError(int result, char *message)
{
  if(result==-1)
  {
    fprintf(stderr, "Error %s: %s\n", message, strerror(errno));
    exit(2);
  }
}

void handleOpenSSLFailure()
{
  fprintf(stderr, "Error using OpenSSL");
  exit(2);
}

void printUsage(char *progName)
{
  fprintf(stderr, "Usage: %s --log=FILE --id=NINE_DIGIT_ID --host=HOSTNAME PORT [--scale=F|C] [--period==N]\n", progName);
  exit(1);
}

void setTimerPeriod(int timerfd, int per)
{
  struct itimerspec time={{per,0},{per,0}};
  checkForError(timerfd_settime(timerfd, 0, &time, NULL), "changing timer period");
}

void shutdownProgram()
{
  time_t rawtime;
  checkForError(time(&rawtime),"getting raw time");
  struct tm *locTime=localtime(&rawtime);
  char writeBuf[50];
  sprintf(writeBuf, "%02d:%02d:%02d SHUTDOWN\n",locTime->tm_hour,locTime->tm_min,locTime->tm_sec);
  if(SSL_write(ssl, writeBuf, strlen(writeBuf))<=0) handleOpenSSLFailure();
  checkForError(write(logfd, writeBuf, strlen(writeBuf)), "writing to log");
  running=0;
}

void parse(char *parseBuf, int *parseLength)
{
  int end;
  for(end=0; parseBuf[end]!='\n'; end++)
  {
    if(parseBuf[end]=='\0')
      return;
  }
  char command[end+1];
  int i;
  for(i=0; i<end; i++)
    command[i]=parseBuf[i];
  command[i]='\0';
  int remainder=strlen(parseBuf)-end-1;
  for(i=0; i<remainder; i++)
  {
    parseBuf[i]=parseBuf[i+end+1];
  }
  parseBuf[i]='\0';
  *parseLength-=end+1;
  checkForError(write(logfd, command, strlen(command)), "writing to log");
  checkForError(write(logfd, "\n", 1), "writing to log");

  if(strcmp(command, "OFF")==0)
    shutdownProgram();
  else if(strcmp(command, "STOP")==0)
    stopped=1;
  else if(strcmp(command, "START")==0)
    stopped=0;
  else if(strcmp(command, "SCALE=F")==0)
    scale='F';
  else if(strcmp(command, "SCALE=C")==0)
    scale='C';
  else if(strncmp(command, "PERIOD=", 7)==0)
  {
    int arg=atoi(command+7);
    if(arg<=0)
      fprintf(stderr, "Argument must be integer greater than 0\n");
    else
    {
      period=arg;
      setTimerPeriod(timerfd,arg);
    }
  }
  else
    fprintf(stderr, "Unrecognized Command\n");
}

double getTempReading(mraa_aio_context aioFd)
{
  int a=mraa_aio_read(aioFd);
  double R=1023.0/a-1.0;
  R*=R0;
  double result = 1.0/(log(R/R0)/B+1/298.15)-273.15;
  if(scale=='F')
  {
    result*=1.8;
    result+=32;
  }
  return result;
}

void generateReport(mraa_aio_context aioFd)
{
  time_t rawtime;
  checkForError(time(&rawtime),"getting raw time");
  struct tm *locTime=localtime(&rawtime);
  char writeBuf[50];
  sprintf(writeBuf, "%02d:%02d:%02d %.1f\n",locTime->tm_hour,locTime->tm_min,locTime->tm_sec, getTempReading(aioFd));
  if(stopped==0)
  {
    if(SSL_write(ssl, writeBuf, strlen(writeBuf))<=0) handleOpenSSLFailure();
    checkForError(write(logfd, writeBuf, strlen(writeBuf)), "writing to log");
  }
}

int main(int argc, char *argv[])
{
  static struct option long_options[] =
    {
      {"period", required_argument, 0, 'p'},
      {"scale", required_argument, 0, 's'},
      {"log", required_argument, 0, 'l'},
      {"id", required_argument, 0, 'i'},
      {"host", required_argument, 0, 'h'},
      {0, 0, 0, 0}
    };

  period=1;
  scale='F';
  logfd=-1;
  host=NULL;
  id=NULL;
  signed char c;
  while((c=getopt_long(argc, argv, "", long_options, 0)) != -1)
  {
    switch(c){
      case 'p':
        period=atoi(optarg);
        break;
      case 's':
        scale=optarg[0];
        break;
      case 'l':
        logfd=open(optarg, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
        checkForError(logfd, "opening logfile");
        break;
      case 'i':
        id=optarg;
        break;
      case 'h':
        host=optarg;
        break;
      default:
        printUsage(argv[0]);
        break;
    }
  }
  if(period<=0 || (scale!='F' && scale!='C') || logfd==-1 || id==NULL || host==NULL || strlen(id)!=9 || argv[optind]==NULL)
    printUsage(argv[0]);

  int i;
  for(i=0; i<9; i++)
    if(!isdigit(id[i]))
      printUsage(argv[0]);

  for(i=0; i<(signed int)strlen(argv[optind]); i++)
    if(!isdigit(argv[optind][i]))
      printUsage(argv[0]);

  if(atoi(argv[optind])<=0)
    printUsage(argv[0]);

  struct addrinfo *result;
  int s;
  s=getaddrinfo(host, argv[optind], NULL, &result);
  if(s!=0)
  {
    fprintf(stderr, "Error resolving host: %s\n", gai_strerror(s));
    exit(2);
  }

  struct addrinfo *rp;
  for (rp = result; rp != NULL; rp = rp->ai_next)
  {
    socketfd = socket(rp->ai_family, rp->ai_socktype,rp->ai_protocol);
    if (socketfd == -1)
      continue;

    if (connect(socketfd, rp->ai_addr, rp->ai_addrlen) != -1)
      break;

    close(socketfd);
  }

  if(rp==NULL)
  {
    fprintf(stderr, "Error: could not connect\n");
    exit(2);
  }
  freeaddrinfo(result);

  checkForError(fcntl(socketfd, F_SETFL, fcntl(socketfd, F_GETFL, 0) | O_NONBLOCK), "changing socket to nonblocking");

  ctx = NULL;
  ssl = NULL;
  X509_VERIFY_PARAM *param;
  SSL_library_init();
  SSL_load_error_strings();
  OpenSSL_add_all_algorithms();

  const SSL_METHOD* method = SSLv23_method();
  if(method==NULL) handleOpenSSLFailure();
  ctx = SSL_CTX_new(method);
  if(ctx==NULL) handleOpenSSLFailure();
  SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
  SSL_CTX_set_verify_depth(ctx, 4);
  SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3);
  if(SSL_CTX_load_verify_locations(ctx, "lab4c_server.crt", NULL)!=1) handleOpenSSLFailure();

  ssl=SSL_new(ctx);
  if(ssl==NULL) handleOpenSSLFailure();
  param = SSL_get0_param(ssl);
  X509_VERIFY_PARAM_set_hostflags(param, 0);
  X509_VERIFY_PARAM_set1_host(param, host, 0);
  if(SSL_set_fd(ssl, socketfd)==-1) handleOpenSSLFailure();
  if(SSL_set_tlsext_host_name(ssl, host)!=1) handleOpenSSLFailure();
  if(SSL_connect(ssl)!=1) handleOpenSSLFailure();

  X509* cert = SSL_get_peer_certificate(ssl);
  if(cert==NULL) handleOpenSSLFailure();
  if(SSL_get_verify_result(ssl)!=X509_V_OK) handleOpenSSLFailure();
  atexit(closeSSL);

  timerfd=timerfd_create(CLOCK_MONOTONIC, 0);
  checkForError(timerfd, "creating timer");
  setTimerPeriod(timerfd, period);

  mraa_aio_context adc_a0= mraa_aio_init(1);
  if(adc_a0==NULL)
  {
    fprintf(stderr, "Cannot init AIN0, try running as root or verify grove connection\n");
    exit(2);
  }

  char idBuf[13];
  sprintf(idBuf, "ID=%s\n", id);
  if(SSL_write(ssl, idBuf, strlen(idBuf))<=0) handleOpenSSLFailure();
  checkForError(write(logfd, idBuf, strlen(idBuf)), "writing ID to log");
  generateReport(adc_a0);

  struct pollfd pollingArr[2]={{timerfd,POLLIN,0},{socketfd,POLLIN,0}};
  char *parseBuf=malloc(sizeof(char));
  if(parseBuf==NULL)
  {
    fprintf(stderr, "Error malloc failed");
    exit(2);
  }
  *parseBuf='\0';
  int parseLength=0;
  int allocSize=1;
  while(running==1)
  {
    int pollResult = poll(pollingArr, 2, 0);
    checkForError(pollResult, "polling");
    if(pollResult>0)
    {
      if(pollingArr[0].revents & POLLIN)
      {
        char readBuf[256];
        checkForError(read(timerfd, readBuf, O_NONBLOCK), "reading timerfd");
        generateReport(adc_a0);
      }
      if(pollingArr[1].revents & POLLIN)
      {
        char readBuf[256];
        int numRead=SSL_read(ssl, readBuf, 255);
        if(numRead<=0) handleOpenSSLFailure();
        parseLength+=numRead;
        while(parseLength>=allocSize)
        {
          allocSize*=2;
          parseBuf=realloc(parseBuf, allocSize*sizeof(char));
        }
        if(parseBuf==NULL)
        {
          fprintf(stderr, "Error realloc failed");
          exit(2);
        }
        strncat(parseBuf, readBuf, numRead);
        for(i=0; i<numRead; i++)
        {
          if(readBuf[i]=='\n')
            parse(parseBuf, &parseLength);
        }
      }
    }
  }

  checkForError(close(logfd), "closing logfile");
  free(parseBuf);
  mraa_aio_close(adc_a0);
  return 0;
}

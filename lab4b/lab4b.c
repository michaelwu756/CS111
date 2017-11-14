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
const int B = 4275;
const int R0 = 100000;

int timerfd;
char scale;
int period;
int logfd;
int stopped=0;
void checkForError(int result, char *message)
{
  if(result==-1)
  {
    fprintf(stderr, "Error %s: %s\n", message, strerror(errno));
    exit(1);
  }
}

void printUsage(char *progName)
{
  fprintf(stderr, "Usage: %s [--scale=F|C] [--period==N] [--log=FILE]\n", progName);
  exit(1);
}

void sig_handler(int sig)
{
  fprintf(stderr, "Caught sigint signal number %d\n", sig);
  exit(1);
}

void setTimerPeriod(int timerfd, int per)
{
  struct itimerspec time={{per,0},{per,0}};
  checkForError(timerfd_settime(timerfd, 0, &time, NULL), "changing timer period");
}

void parse(char *parseBuf)
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
  if(strcmp(command, "OFF")==0)
  {
  }
  else if(strcmp(command, "STOP")==0)
  {
    stopped=1;
    if(logfd!=-1)
      checkForError(write(logfd, "STOP\n", 5), "writing to log");
  }
  else if(strcmp(command, "START")==0)
  {
    stopped=0;
    if(logfd!=-1)
      checkForError(write(logfd, "START\n", 6), "writing to log");
  }
  else if(strcmp(command, "SCALE=F")==0)
  {
    scale='F';
    if(logfd!=-1)
      checkForError(write(logfd, "SCALE=F\n", 8), "writing to log");
  }
  else if(strcmp(command, "SCALE=C")==0)
  {
    scale='C';
    if(logfd!=-1)
      checkForError(write(logfd, "SCALE=C\n", 8), "writing to log");
  }
  else if(strncmp(command, "PERIOD=", 7)==0)
  {
    int arg=atoi(command+7);
    if(arg<=0)
      fprintf(stderr, "Argument must be integer greater than 0\n");
    else
    {
      period=arg;
      setTimerPeriod(timerfd,arg);
      char writeBuf[256];
      sprintf(writeBuf, "PERIOD=%d\n", arg);
      if(logfd!=-1)
        checkForError(write(logfd, writeBuf, strlen(writeBuf)), "writing to log");
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

int main(int argc, char *argv[])
{
  static struct option long_options[] =
    {
      {"period", required_argument, 0, 'p'},
      {"scale", required_argument, 0, 's'},
      {"log", required_argument, 0, 'l'},
      {0, 0, 0, 0}
    };

  period=1;
  scale='F';
  logfd=-1;
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
        logfd=open(optarg, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
        checkForError(logfd, "opening logfile");
        break;
      default:
        printUsage(argv[0]);
        break;
    }
  }
  if(period<=0 || (scale!='F' && scale!='C'))
    printUsage(argv[0]);

  signal(SIGINT, sig_handler);

  timerfd=timerfd_create(CLOCK_MONOTONIC, 0);
  checkForError(timerfd, "creating timer");
  setTimerPeriod(timerfd, period);

  mraa_aio_context adc_a0= mraa_aio_init(1);
  if(adc_a0==NULL)
  {
    fprintf(stderr, "Nothing connected to a1\n");
    exit(1);
  }

  struct pollfd pollingArr[2]={{timerfd,POLLIN,0},{STDIN_FILENO,POLLIN,0}};
  char *parseBuf=malloc(sizeof(char));
  if(parseBuf==NULL)
  {
    fprintf(stderr, "Error malloc failed");
    exit(1);
  }
  *parseBuf='\0';
  int parseLength=1;
  int i;
  while(1)
  {
    int pollResult = poll(pollingArr, 2, 0);
    checkForError(pollResult, "polling");
    if(pollResult>0)
    {
      if(pollingArr[0].revents & POLLIN)
      {
        char readBuf[256];
        checkForError(read(timerfd, readBuf, O_NONBLOCK), "reading timerfd");
        time_t rawtime;
        checkForError(time(&rawtime),"getting raw time");
        struct tm *locTime=localtime(&rawtime);
        char writeBuf[50];
        sprintf(writeBuf, "%02d:%02d:%02d %.1f\n",locTime->tm_hour,locTime->tm_min,locTime->tm_sec, getTempReading(adc_a0));
        if(!stopped)
        {
          checkForError(write(STDOUT_FILENO, writeBuf, strlen(writeBuf)),"writing to stdout");
          if(logfd!=-1)
            checkForError(write(logfd, writeBuf, strlen(writeBuf)), "writing to log");
        }
      }
      if(pollingArr[1].revents & POLLIN)
      {
        char readBuf[256];
        int numRead=read(STDIN_FILENO, readBuf, 255);
        checkForError(numRead, "reading from keyboard");
        int newParseLength = parseLength+numRead;
        parseBuf=realloc(parseBuf, newParseLength*sizeof(char));
        if(parseBuf==NULL)
        {
          fprintf(stderr, "Error realloc failed");
          exit(1);
        }
        strncat(parseBuf, readBuf, numRead);
        for(i=0; i<numRead; i++)
        {
          if(readBuf[i]=='\n')
            parse(parseBuf);
        }
      }
    }
  }

  if(logfd!=-1)
    checkForError(close(logfd), "closing logfile");
  free(parseBuf);
  mraa_aio_close(adc_a0);
  return 0;
}

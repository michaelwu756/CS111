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
const int B = 4275;
const int R0 = 100000;

int timerfd;
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

void setPeriod(int timerfd, int period)
{
  struct itimerspec time={{period,0},{period,0}};
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
  }
  else if(strcmp(command, "START")==0)
  {
  }
  else if(strcmp(command, "SCALE=F")==0)
  {
  }
  else if(strcmp(command, "SCALE=C")==0)
  {
  }
  else if(strncmp(command, "PERIOD=", 7)==0)
  {
    int arg=atoi(command+7);
    if(arg<=0)
      fprintf(stderr, "Argument must be integer greater than 0\n");
    else
      setPeriod(timerfd,arg);
  }
  else
    fprintf(stderr, "Unrecognized Command\n");
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

  int period=1;
  char scale='F';
  int logfd=-1;
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
  printf("scale:%c period:%d\n",scale, period);
  if(period<=0 || (scale!='F' && scale!='C'))
    printUsage(argv[0]);

  signal(SIGINT, sig_handler);

  timerfd=timerfd_create(CLOCK_MONOTONIC, 0);
  checkForError(timerfd, "creating timer");
  setPeriod(timerfd, period);

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
        sprintf(writeBuf, "%02d:%02d:%02d \n",locTime->tm_hour,locTime->tm_min,locTime->tm_sec);
        checkForError(write(STDOUT_FILENO, writeBuf, strlen(writeBuf)),"writing to stdout");
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
  /*
  mraa_aio_context adc_a0;
  uint16_t adc_value = 0;
  float adc_value_float = 0.0;
  mraa_result_t r = MRAA_SUCCESS;

  adc_a0 = mraa_aio_init(1);
  if(adc_a0==NULL)
  {
    fprintf(stderr, "Nothing connected to a%d\n", i);
    exit(1);
  }


  int running = 0;
  while (running == 0) {
    adc_value = mraa_aio_read(adc_a0);
    adc_value_float = mraa_aio_read_float(adc_a0);
    fprintf(stdout, "ADC A0 read %X - %d\n", adc_value, adc_value);
    fprintf(stdout, "ADC A0 read float - %.5f (Ctrl+C to exit)\n", adc_value_float);
  }
  r = mraa_aio_close(adc_a0);
  if (r != MRAA_SUCCESS) {
    mraa_result_print(r);
    }*/

  checkForError(close(logfd), "closing logfile");
  free(parseBuf);
  return 0;
}

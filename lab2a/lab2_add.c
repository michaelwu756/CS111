#include<getopt.h>
#include<stdlib.h>
#include<stdio.h>
#include<time.h>
#include<pthread.h>
#include<string.h>
#include<errno.h>

int iterations;
void add(long long *pointer, long long value) {
  long long sum = *pointer + value;
  *pointer = sum;
}

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
  fprintf(stderr, "Usage: %s --threads=N --iterations==N\n", progName);
  exit(1);
}

void *threadMain(void *arg)
{
  int i;
  for(i=0; i<iterations; i++)
    add((long long *)arg,1);
  for(i=0; i<iterations; i++)
    add((long long *)arg,-1);
  return NULL;
}

void printCSV(char *testType, int threads, int iterations, struct timespec *startTime, struct timespec *endTime, int counter)
{
  struct timespec elapsedTime = {(*endTime).tv_sec-(*startTime).tv_sec,(*endTime).tv_nsec-(*startTime).tv_nsec};
  long long elapsedTimeNsec = elapsedTime.tv_sec*1000000000+elapsedTime.tv_nsec;
  long long operations = iterations*threads*2;
  printf("%s,%d,%d,%d,%d,%d,%d\n",testType,threads,iterations,operations,elapsedTimeNsec,elapsedTimeNsec/operations,counter);
}

int main(int argc, char  *argv[])
{
  static struct option long_options[] =
  {
    {"threads", required_argument, 0, 't'},
    {"iterations", required_argument, 0, 'i'},
    {0, 0, 0, 0}
  };
  
  char c;
  char *testType="add-none";
  int threads=0;
  iterations=0;
  while((c=getopt_long(argc, argv, "", long_options, 0)) != -1)
  {
    switch(c){
      case 't':
        threads=atoi(optarg);
        break;
      case 'i':
        iterations=atoi(optarg);
        break;
      default:
        printUsage(argv[0]);
        break;
    }
  }

  if(threads==0 || iterations==0)
    printUsage(argv[0]);

  long long counter=0;

  struct timespec startTime;
  checkForError(clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &startTime), "getting start time");

  pthread_t thread[threads];
  int i;
  for(i=0; i<threads; i++)
    checkForError(pthread_create(&thread[i],NULL,threadMain,&counter),"making thread");
  for(i=0; i<threads; i++)
    pthread_join(thread[i],NULL);
  
  struct timespec endTime;
  checkForError(clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &endTime), "getting end time");

  printCSV(testType, threads, iterations, &startTime, &endTime, counter);
}
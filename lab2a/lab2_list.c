#include<getopt.h>
#include<stdlib.h>
#include<stdio.h>
#include<time.h>
#include<pthread.h>
#include<string.h>
#include<errno.h>
#include<signal.h>
#include"SortedList.h"

int iterations;
int opt_yield=0;
char testType='\0';
pthread_mutex_t mutex;
int syncLock=0;
SortedList_t list;
SortedListElement_t *elementArr;
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
  fprintf(stderr, "Usage: %s [--threads=N] [--iterations==N] [--yield=[idl]] [--sync=m|s]\n", progName);
  exit(1);
}

void corruptedList()
{
  fprintf(stderr, "Error corrupted list\n");
  exit(2);
}

void segfaultHandler(int sig)
{
  fprintf(stderr, "Error caught segfault signal number %d\n", sig);
  exit(2);
}

void acquireLock()
{
  if(testType=='m')
    pthread_mutex_lock(&mutex);
  if(testType=='s')
    while(__sync_lock_test_and_set(&syncLock, 1)!=0)
      ;
}

void releaseLock()
{
  if(testType=='m')
    pthread_mutex_unlock(&mutex);
  if(testType=='s')
    __sync_lock_release(&syncLock);
}

void *threadMain(void *arg)
{
  int threadNum=*(int *)arg;
  int i;
  for(i=0; i<iterations; i++)
  {
    acquireLock();
    SortedList_insert(&list,elementArr+threadNum*iterations+i);
    releaseLock();
  }
  acquireLock();
  if(SortedList_length(&list)==-1)
    corruptedList();
  releaseLock();
  for(i=0; i<iterations; i++)
  {
    acquireLock();
    SortedListElement_t *element = SortedList_lookup(&list, elementArr[threadNum*iterations+i].key);
    if(element==NULL || SortedList_delete(element)==1)
      corruptedList();
    releaseLock();
  }
  free(arg);
  return NULL;
}

void printCSV(int yield, char type, int threads, int iterations, struct timespec *startTime, struct timespec *endTime)
{
  struct timespec elapsedTime = {(*endTime).tv_sec-(*startTime).tv_sec,(*endTime).tv_nsec-(*startTime).tv_nsec};
  long long elapsedTimeNsec = elapsedTime.tv_sec*1000000000+elapsedTime.tv_nsec;
  long long operations = iterations*threads*3;
  char y[5];
  int i=0;
  if(yield & INSERT_YIELD)
  {
    y[i]='i';
    i++;
  }
  if(yield & INSERT_YIELD)
  {
    y[i]='d';
    i++;
  }
  if(yield & INSERT_YIELD)
  {
    y[i]='l';
    i++;
  }
  if(i==0)
    strcpy(y, "none");
  else
    y[i]='\0';
  char t[5];
  if(type=='\0')
    strcpy(t,"none");
  else
    sprintf(t, "%c", type);
  printf("list-%s-%s,%d,%d,%lld,%lld,%lld\n",y,t,threads,iterations,operations,elapsedTimeNsec,elapsedTimeNsec/operations);
}

int main(int argc, char  *argv[])
{
  static struct option long_options[] =
  {
    {"threads", required_argument, 0, 't'},
    {"iterations", required_argument, 0, 'i'},
    {"sync", required_argument, 0, 's'},
    {"yield", required_argument, 0, 'y'},
    {0, 0, 0, 0}
  };

  char c;
  int threads=1;
  iterations=1;
  int i;
  while((c=getopt_long(argc, argv, "", long_options, 0)) != -1)
  {
    switch(c){
      case 't':
        threads=atoi(optarg);
        break;
      case 'i':
        iterations=atoi(optarg);
        break;
      case 's':
        testType=optarg[0];
        break;
      case 'y':
        i=0;
        while(optarg[i]!='\0')
        {
          switch(optarg[i]){
            case 'i':
              opt_yield|=INSERT_YIELD;
              break;
            case 'd':
              opt_yield|=DELETE_YIELD;
              break;
            case 'l':
              opt_yield|=LOOKUP_YIELD;
              break;
            default:
              printUsage(argv[0]);
              break;
          }
          i++;
        }
        break;
      default:
        printUsage(argv[0]);
        break;
    }
  }

  if(threads==0 || iterations==0 || (testType!='\0' && testType!='m' && testType!='s'))
    printUsage(argv[0]);

  signal(SIGSEGV, segfaultHandler);

  srand(time(NULL));

  pthread_mutex_init(&mutex, NULL);


  list=(SortedList_t){.next=&list,.prev= &list,.key=NULL};

  elementArr = malloc(threads*iterations*sizeof(SortedListElement_t));
  for(i=0; i<threads*iterations; i++)
    elementArr[i]=(SortedListElement_t){.next=NULL,.prev= NULL,.key="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"+rand()%52};

  struct timespec startTime;
  checkForError(clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &startTime), "getting start time");

  pthread_t thread[threads];
  for(i=0; i<threads; i++)
  {
    int *threadNum = malloc(sizeof(int));
    *threadNum=i;
    checkForError(pthread_create(&thread[i],NULL,threadMain,threadNum),"making thread");
  }
  for(i=0; i<threads; i++)
    pthread_join(thread[i],NULL);

  struct timespec endTime;
  checkForError(clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &endTime), "getting end time");

  if(SortedList_length(&list)!=0)
    corruptedList();

  printCSV(opt_yield, testType, threads, iterations, &startTime, &endTime);

  free(elementArr);
  pthread_mutex_destroy(&mutex);
}

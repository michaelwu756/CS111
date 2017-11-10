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
pthread_mutex_t *mutex;
int *spinLock=0;
SortedList_t *list;
SortedListElement_t *elementArr;
int lists;
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
  fprintf(stderr, "Usage: %s [--threads=N] [--iterations==N] [--yield=[idl]] [--sync=m|s] [--lists=N]\n", progName);
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

long long getElapsedTimeNsec(struct timespec *startTime, struct timespec *endTime)
{
  struct timespec elapsedTime = {(*endTime).tv_sec-(*startTime).tv_sec,(*endTime).tv_nsec-(*startTime).tv_nsec};
  return (long long)(elapsedTime.tv_sec*1000000000+elapsedTime.tv_nsec);
}

void acquireLock(int subListNum)
{
  if(testType=='m')
    pthread_mutex_lock(mutex+subListNum);
  if(testType=='s')
    while(__sync_lock_test_and_set(spinLock+subListNum, 1)!=0)
      ;
}

void releaseLock(int subListNum)
{
  if(testType=='m')
    pthread_mutex_unlock(mutex+subListNum);
  if(testType=='s')
    __sync_lock_release(spinLock+subListNum);
}

int simpleHash(const char *str)
{
  unsigned long hash = 5381;
  int c;

  while ((c = *str++))
    hash = ((hash << 5) + hash) + c;

  return hash%lists;
}

void *threadMain(void *arg)
{
  int threadNum=*(int *)arg;
  int i;
  long long *waitingForLockTime=malloc(sizeof(long long));
  *waitingForLockTime=0;
  struct timespec startTime;
  struct timespec endTime;
  for(i=0; i<iterations; i++)
  {
    int hash=simpleHash(elementArr[threadNum*iterations+i].key);
    checkForError(clock_gettime(CLOCK_MONOTONIC, &startTime), "getting lock acquisition start time");
    acquireLock(hash);
    checkForError(clock_gettime(CLOCK_MONOTONIC, &endTime), "getting lock acquisition end time");
    *waitingForLockTime+=getElapsedTimeNsec(&startTime, &endTime);
    SortedList_insert(list+hash,elementArr+threadNum*iterations+i);
    releaseLock(hash);
  }
  checkForError(clock_gettime(CLOCK_MONOTONIC, &startTime), "getting lock acquisition start time");
  for(i=0; i<lists; i++)
    acquireLock(i);
  checkForError(clock_gettime(CLOCK_MONOTONIC, &endTime), "getting lock acquisition end time");
  *waitingForLockTime+=getElapsedTimeNsec(&startTime, &endTime);
  int length=0;
  for(i=0; i<lists; i++)
  {
    int subListLength=SortedList_length(list+i);
    if(subListLength==-1)
      corruptedList();
    length+=subListLength;
  }
  for(i=0;i<lists;i++)
    releaseLock(i);
  for(i=0; i<iterations; i++)
  {
    int hash=simpleHash(elementArr[threadNum*iterations+i].key);
    checkForError(clock_gettime(CLOCK_MONOTONIC, &startTime), "getting lock acquisition start time");
    acquireLock(hash);
    checkForError(clock_gettime(CLOCK_MONOTONIC, &endTime), "getting lock acquisition end time");
    *waitingForLockTime+=getElapsedTimeNsec(&startTime, &endTime);
    SortedListElement_t *element = SortedList_lookup(list+hash, elementArr[threadNum*iterations+i].key);
    if(element==NULL || SortedList_delete(element)==1)
      corruptedList();
    releaseLock(hash);
  }
  free(arg);
  if(testType=='\0')
    *waitingForLockTime=0;
  return waitingForLockTime;
}

const char *generateKey(int length)
{
  char * key=malloc((length+1)*sizeof(char));
  int i;
  for(i=0;i<length;i++)
  {
    key[i]=rand()%95+32;
  }
  key[length]='\0';
  return (const char *)key;
}

void printCSV(int yield, char type, int threads, int iterations, int lists, struct timespec *startTime, struct timespec *endTime, long long waitForLockTime)
{
  long long elapsedTimeNsec = getElapsedTimeNsec(startTime, endTime);
  long long operations = iterations*threads*3;
  char y[5];
  int i=0;
  if(yield & INSERT_YIELD)
  {
    y[i]='i';
    i++;
  }
  if(yield & DELETE_YIELD)
  {
    y[i]='d';
    i++;
  }
  if(yield & LOOKUP_YIELD)
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
  printf("list-%s-%s,%d,%d,%d,%lld,%lld,%lld,%lld\n",y,t,threads,iterations,lists,operations,elapsedTimeNsec,elapsedTimeNsec/operations, waitForLockTime/operations);
}

int main(int argc, char  *argv[])
{
  static struct option long_options[] =
  {
    {"threads", required_argument, 0, 't'},
    {"iterations", required_argument, 0, 'i'},
    {"sync", required_argument, 0, 's'},
    {"yield", required_argument, 0, 'y'},
    {"lists", required_argument, 0, 'l'},
    {0, 0, 0, 0}
  };

  signed char c;
  int threads=1;
  iterations=1;
  lists=1;
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
      case 'l':
        lists=atoi(optarg);
        break;
      default:
        printUsage(argv[0]);
        break;
    }
  }

  if(threads==0 || iterations==0 || lists==0 || (testType!='\0' && testType!='m' && testType!='s'))
    printUsage(argv[0]);

  signal(SIGSEGV, segfaultHandler);

  srand(time(NULL));

  mutex=malloc(lists*sizeof(pthread_mutex_t));
  for(i=0;i<lists;i++)
    pthread_mutex_init(mutex+i, NULL);

  spinLock=malloc(lists*sizeof(int));
  for(i=0;i<lists;i++)
    spinLock[i]=0;

  list=malloc(lists*sizeof(SortedList_t));
  for(i=0;i<lists;i++)
    list[i]=(SortedList_t){.next=list+i,.prev=list+i,.key=NULL};

  elementArr = malloc(threads*iterations*sizeof(SortedListElement_t));
  for(i=0; i<threads*iterations; i++)
    elementArr[i]=(SortedListElement_t){.next=NULL,.prev= NULL,.key=generateKey(10)};

  struct timespec startTime;
  checkForError(clock_gettime(CLOCK_MONOTONIC, &startTime), "getting start time");

  pthread_t thread[threads];
  for(i=0; i<threads; i++)
  {
    int *threadNum = malloc(sizeof(int));
    *threadNum=i;
    checkForError(pthread_create(&thread[i],NULL,threadMain,threadNum),"making thread");
  }

  long long waitingForLockTime=0;
  for(i=0; i<threads; i++)
  {
    void *localWaitingForLockTime;
    pthread_join(thread[i],&localWaitingForLockTime);
    waitingForLockTime+=*(long long *)localWaitingForLockTime;
    free(localWaitingForLockTime);
  }

  struct timespec endTime;
  checkForError(clock_gettime(CLOCK_MONOTONIC, &endTime), "getting end time");

  for(i=0;i<lists;i++)
    if(SortedList_length(list+i)!=0)
      corruptedList();

  printCSV(opt_yield, testType, threads, iterations, lists,  &startTime, &endTime, waitingForLockTime);

  for(i=0; i<threads*iterations; i++)
    free((void *)(elementArr[i].key));
  free(elementArr);
  for(i=0;i<lists;i++)
    pthread_mutex_destroy(mutex+i);
  free(mutex);
  free(spinLock);
  free(list);
  return 0;
}

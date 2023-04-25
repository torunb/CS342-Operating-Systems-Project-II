#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>

pthread_mutex_t *queueMutex;
pthread_mutex_t finishedProcessesMutex;

struct Node** readyProcesses;
int readyQueueNum;
struct Node* finishedProcesses; 

typedef struct {
    int pid;
    double burstLength;
    double arrivalTime;
    double remainingTime;
    double finishTime;
    double turnaroundTime;
    int processorId;
} BurstItem;

struct Node {
    BurstItem pcb;
    struct Node *next;
};

struct arg {
    /* the processor id */
    int processorId;
    /* the process ready queue */
    struct Node* readyQueue;
};

/* the function to be executed by threads (processors) */
static void *processBurts(void *arg_ptr){
    int processorId = ((struct arg *) arg_ptr)->processorId;
    struct Node* readyQueue = ((struct arg *) arg_ptr)->readyQueue;

}

int main(int argc, char* argv[])
{
    /* the thread (processor) ids */
    pthread_t* tids;
    /* thread (processor) function arguments */
    struct arg* t_args;
    /* the number of processors */
    int N = 2;
    /* the time quantum (ms) */
    int Q = 20;
    int T=200, T1=10, T2=1000, L=100, L1=10, L2=500;
    int outmode = 1;
    /* the scheduling approach S or M */
    char* sap = "M";
    /* the queue selection method */
    char* qs = "RM";
    /* the scheduling algorithm */
    char* alg = "RR";
    char* infile = "in.txt";
    char* outfile = "out.txt";

    /* pid count */
    int pidCount = 0;

    /* file pointer */
    FILE* filePtr;

    for(int i = 1; i < argc; i++) {
        char* cur = argv[i];
        printf("cur=> %s\n", cur);

        if(strcmp(cur, "-n") == 0) {
            N = atoi(argv[++i]);
        }
        else if(strcmp(cur, "-a") == 0) {
            sap = argv[++i];
            qs = argv[++i];
        }
        else if(strcmp(cur, "-s") == 0) {
            alg = argv[++i];
            Q = atoi(argv[++i]);
        }
        else if(strcmp(cur, "-i") == 0) {
            infile = argv[++i];
        }
        else if(strcmp(cur, "-m") == 0) {
            outmode = atoi(argv[++i]);
        }
        else if(strcmp(cur, "-o") == 0) {
            outfile = argv[++i];
        }
        else if(strcmp(cur, "-r") == 0) {
            T=atoi(argv[++i]); T1=atoi(argv[++i]); T2=atoi(argv[++i]);
            L=atoi(argv[++i]); L1=atoi(argv[++i]); L2=atoi(argv[++i]);
        }
        else {
            printf("[-] Ignoring the unknown flag/argument: %s", cur);
        }
    }

    /* checking specified input conditions:
        * If sap is set to be M, we should have N to be greater than 1
        * If sap is set to be S, qs must be set to NA
        * If alg is set to FCFS or SJF, Q has to be set to 0
        NOTE: The checking procedure will be done after the inputs are taken because
        flags can be specified in random order!
    */

    if(strcmp(sap, "M") == 0 && N <= 1){
        N = 2; //set N to its default value if the input N is wrong
    }

    if(strcmp(sap, "S") == 0){
        strcpy(qs, "NA"); //not applicable 
    }

    if(strcmp(alg, "FCFS") == 0 || strcmp(alg,"SJF") == 0){
        Q = 0;
    }

    /* After everything is set, the process can begin...*/

    /* MUTEX LOCK INITIALIZATION */
    /* initialize the queue mutex array */
    if(strcmp(sap, "M") == 0){
        queueMutex = (pthread_mutex_t*) malloc(N * sizeof(pthread_mutex_t));
        /* initialize every mutex lock of queue(s) inside for loop */
        for (int i = 0; i < N; i++){
            pthread_mutex_init(&queueMutex[i], NULL);
        }        
    }
    else if(strcmp(sap, "S") == 0){
        queueMutex = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
        pthread_mutex_init(&queueMutex[0], NULL);
    }

    /* initialize the finished processes mutex lock*/
    pthread_mutex_init(&finishedProcessesMutex, NULL);

    /* QUEUE(S) CREATION */
    if(strcmp(sap, "M") == 0){
        readyProcesses = (struct Node**) malloc(N * sizeof(struct Node*));
        readyQueueNum = N;
        for(int i = 0; i < N; i++){
            readyProcesses[i] = NULL;
        }
    }
    else if(strcmp(sap, "S") == 0){
        readyProcesses = (struct Node**) malloc(sizeof(struct Node*));
        readyQueueNum = 1;
        readyProcesses[0] = NULL;
    }

    /* THREAD CREATION PART */
    /* dynamically allocate the number of thread ids */
    tids = (pthread_t*) malloc(N * sizeof(pthread_t));

    /* dynamically allocate the number of thread arguments */
    t_args = (struct arg*) malloc (N * sizeof(struct arg)); 

    /*  creating threads (processors) */
    int ret;

    for(int tIndex = 0; tIndex < N; tIndex++){
        t_args[tIndex].processorId = tIndex;
        if(strcmp(sap, "M") == 0){
            t_args[tIndex].readyQueue = readyProcesses[tIndex];
        }
        else if(strcmp(sap, "S") == 0){
            t_args[tIndex].readyQueue = readyProcesses[0];
        }
        ret = pthread_create(&(tids[tIndex]), NULL, processBurts,
                             (void *) &(t_args[tIndex]));
        
        if(ret != 0){
            exit(1);
        }
    }

    /* MAIN THREAD PROCESS READ FROM FILE SEGMENT */
    filePtr = fopen(infile, "r");

    if(filePtr == NULL)
    {
        exit(1);
    }

    /*information string*/
    char inputType[64];
    /*amount of time (ms)*/
    int timeInput;

    while(fscanf(filePtr, "%s %d", inputType, &timeInput) != EOF){
        
    }

    fclose(filePtr);

    /* joining threads after their termination */
    for (int tIndex = 0; tIndex < N; tIndex++) {
	    ret = pthread_join(tids[tIndex], NULL);
		if (ret != 0) {
			exit(1);
		}
	}

}

/* 
------------------dummy creation code---------------------

for(int i = 0; i < N; i++){
    struct Node* dummyBurst = (struct Node*)malloc(sizeof(struct Node));
    dummyBurst->pcb.processorId = -1; //dummy var
    dummyBurst->next = NULL;
    readyProcesses[i] = dummyBurst;
}

*/
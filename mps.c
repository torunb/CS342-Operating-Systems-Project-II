#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>

/* beginning timeval to measure the beginning time of the program */
struct timeval tbegin;

/* the time quantum (ms) */
int Q = 20;

pthread_mutex_t *queueMutex;
pthread_mutex_t finishedProcessesMutex;
char* alg;

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
    double waitingTime;
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
static void *processBurst(void *arg_ptr){
    int processorId = ((struct arg *) arg_ptr)->processorId;
    struct Node* readyQueue = ((struct arg *) arg_ptr)->readyQueue;
    struct Node** head = &readyQueue;
    int index = readyQueue->pcb.processorId;
    struct timeval finishTime;
    printf("%d\n", readyQueue->pcb.pid); // to see the id of the thread IT WILL BE DELETED DONT'T PANIC
    while(*head == NULL || readyQueue->pcb.pid != -1){
        pthread_mutex_lock(&queueMutex[index]);
        if(*head == NULL){
            pthread_mutex_unlock(&queueMutex[index]);
            usleep(1000);
        }
        else{
            int isFinished = 0;
            struct Node* current;

            if(strcmp(alg, "SJF") == 0 || strcmp(alg, "FCFS") == 0)
            {
                current = readyQueue;
                deleteHeadNode(&readyQueue);
                pthread_mutex_unlock(&queueMutex[index]);
                usleep(current->pcb.burstLength * 1000); // sleep its burst length time
                gettimeofday(&finishTime, 0);
                current->pcb.finishTime = 1000 * (finishTime.tv_sec - tbegin.tv_sec) + 0.001 * (finishTime.tv_usec - tbegin.tv_usec);
                current->pcb.turnaroundTime = current->pcb.finishTime - current->pcb.arrivalTime;
                current->pcb.waitingTime = current->pcb.turnaroundTime - current->pcb.burstLength;
                current->pcb.remainingTime = 0; 
                isFinished = 1;
            }

            if(strcmp(alg, "RR") == 0){
                current = readyQueue;
                if(current->pcb.remainingTime < Q){
                    usleep(current->pcb.remainingTime * 1000);
                    gettimeofday(&finishTime, 0);
                    current->pcb.finishTime = 1000 * (finishTime.tv_sec - tbegin.tv_sec) + 0.001 * (finishTime.tv_usec - tbegin.tv_usec);
                    current->pcb.turnaroundTime = current->pcb.finishTime - current->pcb.arrivalTime;
                    current->pcb.waitingTime = current->pcb.turnaroundTime - current->pcb.burstLength;
                    current->pcb.remainingTime = 0; 
                    isFinished = 1;
                }
                else {
                    usleep(Q);
                    current->pcb.remainingTime = current->pcb.finishTime - Q;
                }
                deleteHeadNode(&readyQueue);
                addNodeToEnd(&readyQueue,current->pcb.pid, current->pcb.processorId, current->pcb.arrivalTime);
            }

            if(isFinished)
            {
                // struct BurstItem bItem = current->pcb;
                // struct timeval finishTime;
                // gettimeofday(&finishTime, NULL);
                // bItem->finishTime= timeval_diff_ms(&start, &finishTime);
                // bItem->turnaroundTime = bItem->finishTime - bItem->arrivalTime;
                // bItem->waitingTime = bItem->turnaroundTime - bItem->burstLength;
                // bItem->remainingTime = 0;

                current->next = NULL;

                // insert edicez currentı
            }
        }  
    }
    pthread_exit(NULL); 
}

/*function to find the smallest integer position in an array*/
static int findSmallestIntPos(int* intArr, int numOfElements){
    int val = intArr[0];
    int position = 0;
    for(int i = 0; i < numOfElements; i++){
        if(val > intArr[i]){
            val = intArr[i];
            position = i;
        }
    }
    return position;
}

/*function to add a new node to queue linked list*/
static void addNodeToEnd(struct Node** head, int pid, int processorId, double arrivalTime){
    struct Node* nodeNew = (struct Node*) malloc(sizeof(struct Node));
    nodeNew->pcb.pid = pid;
    nodeNew->pcb.processorId = processorId;
    nodeNew->pcb.arrivalTime = arrivalTime;
    nodeNew->next = NULL;

    struct Node* last = *head;

    if(*head == NULL){
        *head = nodeNew;
        return;
    }

    while (last->next != NULL){
        last = last->next;
    }

    last->next = nodeNew;
    return;
}

/*function to add a dummyNode to queue linked list*/
static void addNodeToEndDummy(struct Node** head, int processorId){
    struct Node* nodeNew = (struct Node*) malloc(sizeof(struct Node));
    nodeNew->pcb.pid = -1;
    nodeNew->pcb.processorId = processorId;
    nodeNew->next = NULL;

    struct Node* last = *head;

    if(*head == NULL){
        *head = nodeNew;
        return;
    }

    while (last->next != NULL){
        last = last->next;
    }

    last->next = nodeNew;
    return;
}

/* function that adds according to SJF */
static void addNodeAccordingToSJF(struct Node** head, int pid, int processorId, double arrivalTime){
    struct Node* nodeNew = (struct Node*) malloc(sizeof(struct Node));
    nodeNew->pcb.pid = pid;
    nodeNew->pcb.processorId = processorId;
    nodeNew->pcb.arrivalTime = arrivalTime;
    nodeNew->next = NULL;

    struct Node* last = *head;

    if(*head == NULL){
        *head = nodeNew;
        return;
    }

    if(last->pcb.burstLength > nodeNew->pcb.burstLength){
        *head = nodeNew;
        nodeNew->next = last;
        return;
    }

    while (last->next != NULL && 
           last->next->pcb.burstLength <= nodeNew->pcb.burstLength)
    {
        last = last->next;
    }

    struct Node* temp = last->next;
    last->next = nodeNew;
    nodeNew->next = temp;
    return;
}

/* function to delete the head node in linked list */
static void deleteHeadNode(struct Node** head){
    struct Node* current = *head;

    if(*head = NULL){
        return;
    }

    *head = current->next;

    free(current);
    return;
}

int main(int argc, char* argv[])
{
    /* get the start time of the execution */
    gettimeofday(&tbegin, 0);

    /* the thread (processor) ids */
    pthread_t* tids;
    /* thread (processor) function arguments */
    struct arg* t_args;
    /* the number of processors */
    int N = 2;
    int T=200, T1=10, T2=1000, L=100, L1=10, L2=500;
    int outmode = 1;
    /* the scheduling approach S or M */
    char* sap = "M";
    /* the queue selection method */
    char* qs = "RM";
    /* the scheduling algorithm */
    alg = "RR";
    char* infile = "in.txt";
    char* outfile = "out.txt";

    /* pid count */
    int pidCount = 0;

    /* file pointer */
    FILE* filePtr;

    /* the number of load in queues for LM */
    int* loadNum;

    /* the turn number for queues for RM */
    int queueTurn = 0;

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
        if(strcmp(qs,"LM") == 0){
            loadNum = (int*) malloc(N * sizeof(int));
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
        ret = pthread_create(&(tids[tIndex]), NULL, processBurst,
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

    /*the time of the arrival of the process*/
    struct timeval tarrival;

    /*information string*/
    char inputType[64];
    /*amount of time (ms)*/
    int timeInput;

    while(fscanf(filePtr, "%s %d", inputType, &timeInput) != EOF){
        if(strcmp(inputType, "PL") == 0){
            if(strcmp(qs, "RM") == 0){
                pthread_mutex_lock(&queueMutex[queueTurn % N]);
                gettimeofday(&tarrival, 0);
                double arrivalTime = 1000 * (tarrival.tv_sec - tbegin.tv_sec) + 0.001 * (tarrival.tv_usec - tbegin.tv_usec);
                if(strcmp(alg, "FCFS") == 0 || strcmp(alg, "RR") == 0){
                    addNodeToEnd(&readyProcesses[queueTurn % N],pidCount,queueTurn % N, arrivalTime);
                }

                if(strcmp(alg,"SJF") == 0){
                    addNodeAccordingToSJF(&readyProcesses[queueTurn % N],pidCount,queueTurn % N, arrivalTime);
                }
                queueTurn++;
                pidCount++;
                pthread_mutex_unlock(&queueMutex[queueTurn % N]);
            }
            if(strcmp(qs,"LM") == 0){
                int smallestIntPos = findSmallestIntPos(loadNum, N);
                pthread_mutex_lock(&queueMutex[smallestIntPos]);
                gettimeofday(&tarrival, 0);
                double arrivalTime = 1000 * (tarrival.tv_sec - tbegin.tv_sec) + 0.001 * (tarrival.tv_usec - tbegin.tv_usec);
                if(strcmp(alg, "FCFS") == 0 || strcmp(alg, "RR") == 0){
                    addNodeToEnd(&readyProcesses[smallestIntPos],pidCount,smallestIntPos, arrivalTime);
                }

                if(strcmp(alg,"SJF") == 0){
                    addNodeAccordingToSJF(&readyProcesses[smallestIntPos],pidCount,smallestIntPos, arrivalTime);
                }
                pidCount++;
                pthread_mutex_unlock(&queueMutex[smallestIntPos]);
                loadNum[smallestIntPos] = loadNum[smallestIntPos] + 1;
            }
        }

        if(strcmp(inputType, "IAT") == 0){
            //usleep suspends execution for x microseconds
            usleep(timeInput * 1000);
        }
    }

    for(int i = 0; i < N; i++){
        pthread_mutex_lock(&queueMutex[i]);
        addNodeToEndDummy(&readyProcesses[i],i);
        pthread_mutex_unlock(&queueMutex[i]);
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
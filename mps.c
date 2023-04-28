#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#include <time.h>
#include <math.h>

/* beginning timeval to measure the beginning time of the program */
struct timeval tbegin;

/* the time quantum (ms) */
int Q = 20;

pthread_mutex_t *queueMutex;
pthread_mutex_t finishedProcessesMutex;

/* the number of processors */
int N;
/* the queue selection method */
char* qs;
/* the scheduling approach S or M */
char* sap;
char* alg;
char* infile = NULL;
char* outfile = NULL;
FILE* outFilePtr;

int outmode; // OUTMODE 

struct Node** readyProcesses;
int readyQueueNum;
struct Node* finishedProcesses; 

typedef struct {
    int pid;
    int burstLength;
    int arrivalTime;
    int remainingTime;
    int finishTime;
    int turnaroundTime;
    int waitingTime;
    int processorId;
} BurstItem;

struct Node {
    BurstItem pcb;
    struct Node *next;
};

struct arg {
    /* the processor's id */
    int processorId;
    /* the process ready queue */
    struct Node** readyQueue;
};

/*function to add a new node to queue linked list*/
static void addNodeToEnd(struct Node** head, int pid, int processorId, int arrivalTime, 
                         int burstLength, int remainingTime, int finishTime, int turnaroundTime,
                         int waitingTime){
    struct Node* nodeNew = (struct Node*) malloc(sizeof(struct Node));
    nodeNew->pcb.pid = pid;
    nodeNew->pcb.processorId = processorId;
    nodeNew->pcb.arrivalTime = arrivalTime;
    nodeNew->pcb.burstLength = burstLength;
    nodeNew->pcb.remainingTime = remainingTime;
    nodeNew->pcb.finishTime = finishTime;
    nodeNew->pcb.turnaroundTime = turnaroundTime;
    nodeNew->pcb.waitingTime = waitingTime;
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
static void addNodeAccordingToSJF(struct Node** head, int pid, int processorId, int arrivalTime, 
                                  int burstLength, int remainingTime, int finishTime, int turnaroundTime,
                                  int waitingTime){
    struct Node* nodeNew = (struct Node*) malloc(sizeof(struct Node));
    nodeNew->pcb.pid = pid;
    nodeNew->pcb.processorId = processorId;
    nodeNew->pcb.arrivalTime = arrivalTime;
    nodeNew->pcb.burstLength = burstLength;
    nodeNew->pcb.remainingTime = remainingTime;
    nodeNew->pcb.finishTime = finishTime;
    nodeNew->pcb.turnaroundTime = turnaroundTime;
    nodeNew->pcb.waitingTime = waitingTime;
    nodeNew->next = NULL;

    struct Node* last = *head;

    if(*head == NULL){
        *head = nodeNew;
        return;
    }

    if((*head)->pcb.burstLength > nodeNew->pcb.burstLength){
        nodeNew->next = *head;
        *head = nodeNew;
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

    if(*head == NULL){
        return;
    }

    *head = current->next;

    free(current);
    return;
}

/* function to sort the linked list in ascending order*/
static void sortQueueAsc(struct Node* head){
    struct Node* current = head;
    struct Node* index = NULL;

    int tempPid;

    if(head == NULL){
        return;
    }else{
        while(current != NULL){
            index = current->next;

            while(index != NULL){
                if(current->pcb.pid > index->pcb.pid){
                    tempPid = current->pcb.pid;
                    current->pcb.pid = index->pcb.pid;
                    index->pcb.pid = tempPid;
                }
                index = index->next;
            }
            current = current->next;
        }
    }
}

void static printInformation(struct Node** head, int currentTime){
    if(*head == NULL){
        printf("Error! Empty list\n");
    }
    
    else{
        struct Node** now = head;
        while(*now != NULL){
            
            if(!outfile && (*now)->pcb.pid != -1){
                printf("time = %d, cpu = %d, pid = %d, burstlen = %d, remainingtime = %d\n", currentTime, (*now)->pcb.processorId + 1, (*now)->pcb.pid + 1, (*now)->pcb.burstLength, (*now)->pcb.remainingTime);
            }
            else if(outfile && (*now)->pcb.pid != -1){
                fprintf(outFilePtr, "time = %d, cpu = %d, pid = %d, burstlen = %d, remainingtime = %d\n", currentTime, (*now)->pcb.processorId + 1, (*now)->pcb.pid + 1, (*now)->pcb.burstLength, (*now)->pcb.remainingTime);
            }
            now = &((*now)->next);
        }
    }
}

void static printOutMode3(struct Node** head, int stayFor, char* alg){
    if(*head == NULL){
        printf("Error! Empty list\n");
    }

    else{
        struct Node** now = head;
        while(*now != NULL){
            if(strcmp(alg, "FCFS") == 0 || strcmp(alg, "SJF") == 0){
                if(!outfile && (*now)->pcb.pid != -1){
                    printf("pid = %d, cpu = %d, it will stay for = %d\n", (*now)->pcb.pid + 1, (*now)->pcb.processorId + 1, (*now)->pcb.remainingTime);
                }
                else if(outfile && (*now)->pcb.pid != -1){
                    fprintf(outFilePtr, "pid = %d, cpu = %d, it will stay for = %d\n", (*now)->pcb.pid + 1, (*now)->pcb.processorId + 1, (*now)->pcb.remainingTime);
                }
            }
            else if(strcmp(alg, "RR") == 0){
                if(!outfile && (*now)->pcb.pid != -1){
                    printf("pid = %d, remaining time = %d, cpu = %d, it will stay for = %d\n", (*now)->pcb.pid + 1, (*now)->pcb.remainingTime, (*now)->pcb.processorId + 1, stayFor);
                }
                else if(outfile && (*now)->pcb.pid != -1){
                    fprintf(outFilePtr, "pid = %d, remaining time = %d, cpu = %d, it will stay for = %d\n", (*now)->pcb.pid + 1, (*now)->pcb.remainingTime, (*now)->pcb.processorId + 1, stayFor);
                }
            }
            now = &((*now)->next);
        }
    }
}

static int getQueueLength(struct Node* head){
    int count = 0;
    struct Node* current = head;
    while (current != NULL) {
        count++;
        current = current->next;
    }
    return count;
}

/*function to find the smallest integer position in an array*/
static int findSmallestIntPos(struct Node** readyProcesses, int numOfElements){
    for(int i = 0; i < N; i++){
        pthread_mutex_lock(&queueMutex[i]);
    }
    int arr[N];
    for(int i = 0; i < N; i++){
        arr[i] = getQueueLength(readyProcesses[i]);
    }
    int val = arr[0];
    int position = 0;
    for(int i = 0; i < numOfElements; i++){
        if(val > arr[i]){
            val = arr[i];
            position = i;
        }
    }
    for(int i = 0; i < N; i++){
        pthread_mutex_unlock(&queueMutex[i]);
    }
    return position;
}

/* the function to be executed by threads (processors) */
static void *processBurst(void *arg_ptr){
    int processorId = ((struct arg *) arg_ptr)->processorId;
    struct Node** readyQueue = ((struct arg *) arg_ptr)->readyQueue;
    int isDummyDetected = 0;
    struct timeval now;
    struct timeval finishTime;
    int queueId;

    if(strcmp(qs, "NA") == 0){
        queueId = 0;
    }
    else {
        queueId = processorId;
    }

    pthread_mutex_lock(&queueMutex[queueId]);
    int length = getQueueLength(*readyQueue);
    pthread_mutex_unlock(&queueMutex[queueId]);
    while(*readyQueue == NULL || !isDummyDetected || length > 1){
        pthread_mutex_lock(&queueMutex[queueId]);
        if(*readyQueue == NULL){
            pthread_mutex_unlock(&queueMutex[queueId]);
            usleep(1000);
        }
        else if((*readyQueue)->pcb.pid == -1){
            isDummyDetected = 1;
            addNodeToEndDummy(readyQueue, processorId);
            deleteHeadNode(readyQueue);
            pthread_mutex_unlock(&queueMutex[queueId]);
        }
        else{
            (*readyQueue)->pcb.processorId = processorId;
            int isFinished = 0;
            struct Node** current;
            int finPid, finBurst, finArr, finRem, finFinTime, finTurn, finWaiting, finProcessorId;

            if(strcmp(alg, "SJF") == 0 || strcmp(alg, "FCFS") == 0)
            {
                current = readyQueue;

                if(outmode == 2){
                    gettimeofday(&now, NULL);
                    int currentTime = 1000 * (now.tv_sec - tbegin.tv_sec) + 0.001 * (now.tv_usec - tbegin.tv_usec);
                    printInformation(current, currentTime);
                }

                else if(outmode == 3){
                    printOutMode3(current, (*current)->pcb.remainingTime, alg);
                }
                usleep((*current)->pcb.burstLength * 1000); 
                gettimeofday(&finishTime, 0);
                (*current)->pcb.finishTime = 1000 * (finishTime.tv_sec - tbegin.tv_sec) + 0.001 * (finishTime.tv_usec - tbegin.tv_usec);
                (*current)->pcb.turnaroundTime = (*current)->pcb.finishTime - (*current)->pcb.arrivalTime;
                (*current)->pcb.waitingTime = (*current)->pcb.turnaroundTime - (*current)->pcb.burstLength;
                (*current)->pcb.remainingTime = 0; 
                isFinished = 1;
                finPid = (*current)->pcb.pid;
                finBurst = (*current)->pcb.burstLength;
                finArr = (*current)->pcb.arrivalTime;
                finRem = (*current)->pcb.remainingTime;
                finFinTime = (*current)->pcb.finishTime;
                finTurn = (*current)->pcb.turnaroundTime;
                finWaiting = (*current)->pcb.waitingTime;
                finProcessorId = (*current)->pcb.processorId;
                deleteHeadNode(readyQueue);
                pthread_mutex_unlock(&queueMutex[queueId]);
            }

            if(strcmp(alg, "RR") == 0){
                current = readyQueue;
                if((*current)->pcb.remainingTime <= Q){
                    if(outmode == 2){
                        gettimeofday(&now, NULL);
                        int currentTime = 1000 * (now.tv_sec - tbegin.tv_sec) + 0.001 * (now.tv_usec - tbegin.tv_usec);
                        printInformation(current, currentTime);
                    }
                    else if(outmode == 3){
                        if((*current)->pcb.remainingTime < Q){
                            printOutMode3(current, (*current)->pcb.remainingTime, alg);
                        }
                        else{
                            printOutMode3(current, Q, alg);
                        }                       
                    }
                    usleep((*current)->pcb.remainingTime * 1000);
                    gettimeofday(&finishTime, 0);
                    (*current)->pcb.finishTime = 1000 * (finishTime.tv_sec - tbegin.tv_sec) + 0.001 * (finishTime.tv_usec - tbegin.tv_usec);
                    (*current)->pcb.turnaroundTime = (*current)->pcb.finishTime - (*current)->pcb.arrivalTime;
                    (*current)->pcb.waitingTime = (*current)->pcb.turnaroundTime - (*current)->pcb.burstLength;
                    (*current)->pcb.remainingTime = 0; 
                    isFinished = 1;
                    finPid = (*current)->pcb.pid;
                    finBurst = (*current)->pcb.burstLength;
                    finArr = (*current)->pcb.arrivalTime;
                    finRem = (*current)->pcb.remainingTime;
                    finFinTime = (*current)->pcb.finishTime;
                    finTurn = (*current)->pcb.turnaroundTime;
                    finWaiting = (*current)->pcb.waitingTime;
                    finProcessorId = (*current)->pcb.processorId;
                    deleteHeadNode(readyQueue);
                }
                else {
                    if(outmode == 2){
                        gettimeofday(&now, NULL);
                        int currentTime = 1000 * (now.tv_sec - tbegin.tv_sec) + 0.001 * (now.tv_usec - tbegin.tv_usec);
                        printInformation(current, currentTime);
                    }
                    else if(outmode == 3){
                        if((*current)->pcb.remainingTime < Q){
                            printOutMode3(current, (*current)->pcb.remainingTime, alg);
                        }
                        else{
                            printOutMode3(current, Q, alg);
                        }    
                    }
                    usleep(Q * 1000);
                    (*current)->pcb.remainingTime = (*current)->pcb.remainingTime - Q;
                    addNodeToEnd(readyQueue, (*current)->pcb.pid, (*current)->pcb.processorId, (*current)->pcb.arrivalTime, 
                                (*current)->pcb.burstLength, (*current)->pcb.remainingTime, 0, 0 , 0);
                    deleteHeadNode(readyQueue);
                }
                pthread_mutex_unlock(&queueMutex[queueId]);
            }

            if(isFinished)
            {
                pthread_mutex_lock(&finishedProcessesMutex);
                addNodeToEnd(&finishedProcesses,finPid, finProcessorId, finArr, 
                             finBurst, finRem, finFinTime, 
                             finTurn, finWaiting);
                pthread_mutex_unlock(&finishedProcessesMutex);
            }
        }
        pthread_mutex_lock(&queueMutex[queueId]); 
        length = getQueueLength(*readyQueue);
        pthread_mutex_unlock(&queueMutex[queueId]); 
    }
    pthread_exit(NULL); 
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
    N = 2;
    int T=200, T1=10, T2=1000, L=100, L1=10, L2=500, pc=10;
    outmode = 1;
    /* the scheduling approach S or M */
    sap = "M";
    /* the queue selection method */
    qs = "RM";
    /* the scheduling algorithm */
    alg = "RR";

    /* pid count */
    int pidCount = 0;

    /* file pointer */
    FILE* filePtr;

    /* the turn number for queues for RM */
    int queueTurn = 0;

    /* check if -i specified */
    int isISpecified = 0;

    /* check if -r specified */
    int isRSpecified = 0;

    time_t t;

    for(int i = 1; i < argc; i++) {
        char* cur = argv[i];

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
            isISpecified = 1;
        }
        else if(strcmp(cur, "-m") == 0) {
            outmode = atoi(argv[++i]);
        }
        else if(strcmp(cur, "-o") == 0) {
            outfile = argv[++i];
            outFilePtr = fopen(outfile, "w");
        }
        else if(strcmp(cur, "-r") == 0) {
            T=atoi(argv[++i]); T1=atoi(argv[++i]); T2=atoi(argv[++i]);
            L=atoi(argv[++i]); L1=atoi(argv[++i]); L2=atoi(argv[++i]);
            pc=atoi(argv[++i]);
            isRSpecified = 1;
        }
        else {
            printf("Ignore unknown argument: %s", cur);
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
        pthread_mutex_init(queueMutex, NULL);
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
            t_args[tIndex].readyQueue = &readyProcesses[tIndex];
        }
        else if(strcmp(sap, "S") == 0){
            t_args[tIndex].readyQueue = readyProcesses;
        }
        ret = pthread_create(&(tids[tIndex]), NULL, processBurst,
                             (void *) &(t_args[tIndex]));
        
        if(ret != 0){
            exit(1);
        }
    }
    
    /*the time of the arrival of the process*/
    struct timeval tarrival;

    if(isISpecified && !isRSpecified){

        /* MAIN THREAD PROCESS READ FROM FILE SEGMENT */
        filePtr = fopen(infile, "r");

        if(filePtr == NULL)
        {
            printf("Error: there is no input file with the given name \n");
            exit(1);
        }

        /*information string*/
        char inputType[64];
        /*amount of time (ms)*/
        int timeInput;

        while(fscanf(filePtr, "%s %d", inputType, &timeInput) != EOF){
            if(strcmp(inputType, "PL") == 0){
                if(strcmp(qs, "RM") == 0){
                    int pos = queueTurn % N;
                    pthread_mutex_lock(&queueMutex[pos]);
                    gettimeofday(&tarrival, 0);
                    int arrivalTime = 1000 * (tarrival.tv_sec - tbegin.tv_sec) + 0.001 * (tarrival.tv_usec - tbegin.tv_usec);
                    if(strcmp(alg, "FCFS") == 0 || strcmp(alg, "RR") == 0){
                        addNodeToEnd(&readyProcesses[pos],pidCount,pos, arrivalTime, timeInput, timeInput, 0, 0, 0);
                    }

                    if(strcmp(alg,"SJF") == 0){
                        addNodeAccordingToSJF(&readyProcesses[pos],pidCount,pos, arrivalTime, timeInput, timeInput, 0, 0, 0);
                    }
                    queueTurn++;
                    pidCount++;
                    pthread_mutex_unlock(&queueMutex[pos]);
                }
                if(strcmp(qs,"LM") == 0){
                    int smallestIntPos = findSmallestIntPos(readyProcesses, N);
                    pthread_mutex_lock(&queueMutex[smallestIntPos]);
                    gettimeofday(&tarrival, 0);
                    int arrivalTime = 1000 * (tarrival.tv_sec - tbegin.tv_sec) + 0.001 * (tarrival.tv_usec - tbegin.tv_usec);
                    if(strcmp(alg, "FCFS") == 0 || strcmp(alg, "RR") == 0){
                        addNodeToEnd(&readyProcesses[smallestIntPos],pidCount,smallestIntPos, arrivalTime, timeInput, timeInput, 0, 0, 0);
                    }

                    if(strcmp(alg,"SJF") == 0){
                        addNodeAccordingToSJF(&readyProcesses[smallestIntPos],pidCount,smallestIntPos, arrivalTime, timeInput, timeInput, 0, 0, 0);
                    }
                    pidCount++;
                    pthread_mutex_unlock(&queueMutex[smallestIntPos]);
                }
                if(strcmp(qs, "NA") == 0){
                    pthread_mutex_lock(&queueMutex[0]);
                    gettimeofday(&tarrival, 0);
                    int arrivalTime = 1000 * (tarrival.tv_sec - tbegin.tv_sec) + 0.001 * (tarrival.tv_usec - tbegin.tv_usec);
                    if(strcmp(alg, "FCFS") == 0 || strcmp(alg, "RR") == 0){
                        addNodeToEnd(&readyProcesses[0],pidCount,-1, arrivalTime, timeInput, timeInput, 0, 0, 0);
                    }

                    if(strcmp(alg,"SJF") == 0){
                        addNodeAccordingToSJF(&readyProcesses[0],pidCount, -1, arrivalTime, timeInput, timeInput, 0, 0, 0);
                    }
                    pidCount++;
                    pthread_mutex_unlock(&queueMutex[0]);;
                }
            }

            if(strcmp(inputType, "IAT") == 0){
                //usleep suspends execution for x microseconds
                usleep(timeInput * 1000);
            }
        }

        fclose(filePtr);
    }
    else {
        srand((unsigned) time(&t));

        for(int i = 0; i < pc; i++){
            double rateParameter = 1.0/L;
            double u, x;
            int num;
            do {
                u = (double)rand() / (double)RAND_MAX;
                x = (-log(1-u))/rateParameter;

                num = (int)round(x);
            } while(num < L1 || num > L2);

            //burst process
            if(strcmp(qs, "RM") == 0){
                int pos = queueTurn % N;
                pthread_mutex_lock(&queueMutex[pos]);
                gettimeofday(&tarrival, 0);
                int arrivalTime = 1000 * (tarrival.tv_sec - tbegin.tv_sec) + 0.001 * (tarrival.tv_usec - tbegin.tv_usec);
                if(strcmp(alg, "FCFS") == 0 || strcmp(alg, "RR") == 0){
                    addNodeToEnd(&readyProcesses[pos],pidCount,pos, arrivalTime, num, num, 0, 0, 0);
                }

                if(strcmp(alg,"SJF") == 0){
                    addNodeAccordingToSJF(&readyProcesses[pos],pidCount,pos, arrivalTime, num, num, 0, 0, 0);
                }
                queueTurn++;
                pidCount++;
                pthread_mutex_unlock(&queueMutex[pos]);
            }
            if(strcmp(qs,"LM") == 0){
                int smallestIntPos = findSmallestIntPos(readyProcesses, N);
                pthread_mutex_lock(&queueMutex[smallestIntPos]);
                gettimeofday(&tarrival, 0);
                int arrivalTime = 1000 * (tarrival.tv_sec - tbegin.tv_sec) + 0.001 * (tarrival.tv_usec - tbegin.tv_usec);
                if(strcmp(alg, "FCFS") == 0 || strcmp(alg, "RR") == 0){
                    addNodeToEnd(&readyProcesses[smallestIntPos], pidCount, smallestIntPos, arrivalTime, num, num, 0, 0, 0);
                }

                if(strcmp(alg,"SJF") == 0){
                    addNodeAccordingToSJF(&readyProcesses[smallestIntPos],pidCount,smallestIntPos, arrivalTime, num, num, 0, 0, 0);
                }
                pidCount++;
                pthread_mutex_unlock(&queueMutex[smallestIntPos]);
            }
            if(strcmp(qs, "NA") == 0){
                pthread_mutex_lock(&queueMutex[0]);
                gettimeofday(&tarrival, 0);
                int arrivalTime = 1000 * (tarrival.tv_sec - tbegin.tv_sec) + 0.001 * (tarrival.tv_usec - tbegin.tv_usec);
                if(strcmp(alg, "FCFS") == 0 || strcmp(alg, "RR") == 0){
                    addNodeToEnd(&readyProcesses[0],pidCount,-1, arrivalTime, num, num, 0, 0, 0);
                }

                if(strcmp(alg,"SJF") == 0){
                    addNodeAccordingToSJF(&readyProcesses[0],pidCount, -1, arrivalTime, num, num, 0, 0, 0);
                }
                pidCount++;
                pthread_mutex_unlock(&queueMutex[0]);
            }

            //iat process
            rateParameter = 1.0/T;
            do {
                u = (double)rand() / (double)RAND_MAX;
                x = (-log(1-u))/rateParameter;

                num = (int)round(x);
            } while(num < T1 || num > T2);

            usleep(1000 * num);
        }
    }

    if(strcmp(sap, "M") == 0){
        for(int i = 0; i < N; i++){
            pthread_mutex_lock(&queueMutex[i]);
            addNodeToEndDummy(&readyProcesses[i],i);
            pthread_mutex_unlock(&queueMutex[i]);
        }
    }

    if(strcmp(sap, "S") == 0){
        pthread_mutex_lock(&queueMutex[0]);
        addNodeToEndDummy(&readyProcesses[0],0);
        pthread_mutex_unlock(&queueMutex[0]);
    }

    /* joining threads after their termination */
    for (int tIndex = 0; tIndex < N; tIndex++) {
	    ret = pthread_join(tids[tIndex], NULL);
		if (ret != 0) {
			exit(1);
		}
	}

    struct Node* curr = finishedProcesses;
    int avgTurnaround = 0;
    int countForAvg = 0;
    
    sortQueueAsc(curr);
    if(!outfile){
        printf("\n");
        printf("%-10s %-10s %-10s %-10s %-10s %-12s %-10s\n", "pid", "cpu", "burstlen", "arv", "finish", "waitingtime", "turnaround");
        while(curr != NULL){
            printf("%-10d %-10d %-10d %-10d %-10d %-12d %-10d\n", curr->pcb.pid + 1, curr->pcb.processorId + 1, curr->pcb.burstLength, curr->pcb.arrivalTime, curr->pcb.finishTime, curr->pcb.waitingTime, curr->pcb.turnaroundTime);
            avgTurnaround = avgTurnaround + curr->pcb.turnaroundTime;
            countForAvg++;
            curr = curr->next;
        }
        avgTurnaround = avgTurnaround / countForAvg;
        printf("\n");
        printf("Average turnaround time: %d\n", avgTurnaround);
    }

    else if(outfile){
        fprintf(outFilePtr, "\n");
        fprintf(outFilePtr, "%-10s %-10s %-10s %-10s %-10s %-12s %-10s\n", "pid", "cpu", "burstlen", "arv", "finish", "waitingtime", "turnaround");
        while(curr != NULL){
            fprintf(outFilePtr, "%-10d %-10d %-10d %-10d %-10d %-12d %-10d\n", curr->pcb.pid + 1, curr->pcb.processorId + 1, curr->pcb.burstLength, curr->pcb.arrivalTime, curr->pcb.finishTime, curr->pcb.waitingTime, curr->pcb.turnaroundTime);
            avgTurnaround = avgTurnaround + curr->pcb.turnaroundTime;
            countForAvg++;
            curr = curr->next;
        }
        avgTurnaround = avgTurnaround / countForAvg;
        fprintf(outFilePtr, "\n");
        fprintf(outFilePtr, "Average turnaround time: %d\n", avgTurnaround);
    }
    
    free(tids);
    free(t_args);

    if(outfile)
        fclose(outFilePtr);

    if(strcmp(sap, "S") == 0){
        struct Node** current = &readyProcesses[0];
        struct Node* next;

        while((*current) != NULL){
            next = (*current)->next;
            free((*current));
            (*current) = next;
        }

        pthread_mutex_destroy(&queueMutex[0]);
        free(queueMutex);
    }

    if(strcmp(sap, "M") == 0){
        for(int i = 0; i < N; i++){
            struct Node** current = &readyProcesses[i];
            struct Node* next;

            while((*current) != NULL){
                next = (*current)->next;
                free((*current));
                (*current) = next;
            }

            pthread_mutex_destroy(&queueMutex[i]);
        }
        free(queueMutex);
    }

    free(readyProcesses);

    struct Node* current = finishedProcesses;
    struct Node* next;

    while(current != NULL){
        next = current->next;
        free(current);
        current = next;
    }

    exit(0);
    return(0);
}
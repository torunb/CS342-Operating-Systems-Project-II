#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
    int N = 2;
    int Q = 20;
    int T=200, T1=10, T2=1000, L=100, L1=10, L2=500;
    int outmode = 1;
    char* sap = "M";
    char* qs = "RM";
    char* alg = "RR";
    char* infile = "in.txt";
    char* outfile = "out.txt";


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
        printf("If SAP is set to be M, we should have N to be greater than 1");
        return 1; //termination due to error
    }

    if(strcmp(sap, "S") == 0){
        strcpy(qs, "NA"); //not applicable 
    }

    if(strcmp(alg, "FCFS") == 0 || strcmp(alg,"SJF") == 0){
        Q = 0;
    }

    
}
# CS342-Operating-Systems-Project-I
CS342 Operating Systems - Spring 2023 - Project #1 - Processes, IPC, and Threads

Application that is simulator for multiporcessor scheduling. There are two approaches, one is single-queue and other one is multi-queue. Three scheduling algorithms can be simulated: FCFS, RR, SJF. The -n N option is used to specify the number of processors. The -a SAP QS option is to specify the scheduling approach. QS indicates the queue selection method for multi-queue approach. The -s ALG Q option specifies the scheduling algorithm that will be simulated. The -i INFILE option, if specified, gives the name of an input file from which burst information. The -o OUTFILE option specifies the name of an output file into which all output of the simulator should go. -r T T1 T2 L L1 L2 option, if specified, indicates that burst information will be generated.

# How To Execute
To compile programs type **_make_**. Then your decision type,

**mps/mps_cv mps [-n N] [-a SAP QS] [-s ALG Q] [-i INFILE] [-m OUTMODE] [-o OUTFILE] [-r T T1 T2 L L1 L2]**

# Team Members
  * [Utku Boran Torun](https://github.com/torunb)
  * [Uğur Can Altun](https://github.com/ugurcanaltun)

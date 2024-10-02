// Abraham de León Gutiérrez
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <fcntl.h>
// REQUIRED LIBRARIES

// CALCULATE THE HEIGHT f(x) = sqrt(1-y2)
long double f(long double  x){
    return sqrt(1.0 - x*x);
}

// FUNTION TO CALCULATE PI
long double calculate_pi(long long int subintervals, long long int portion, long long int start){
    long double base = 1.0 / subintervals; // GET THE BASE OF EACH RECTANGLE
    long double halfbase=0.0, heigth=0.0, area=0.0; // INITIALIZATION ALL VARIABLES
    long long int fin=start+portion; // GET THE RANGE FOR THE PROCESS

    for (long long int i=start; i<fin; i++){ // CALCULATE THE AREA OF EACH RECTANGLE
        halfbase=(i*base)+(base/2.0); // GET THE HALF OF THE BASE (Xi + Base) / 2
        heigth = f(halfbase); // GET THE HEIGHT 
        area += (heigth * base); // AREA OF THE RECTANGLE AREA = BASE * HEIGHT
    }
    return area;
}

int main(int argc, char **argv){
    struct timeval start, end; // INITIALIZATION OF TIME VARIABLES
    double elapsed_time;
    long long int subintervals, procs; // TOTAL OF SUBINTERVALS

    if (argc != 3) { // CHECK WHETHER THE USER HAS ENTERED THE NUMBER OF INTERVALS AND THE NUMBER OD PROCESSES
        fprintf(stderr, "[i] usage: ./piserial <# of subintervals> <# of processes>\r\n");
		exit(1);
    }

    subintervals = atoll(argv[1]); // GET THE NUMBER OF INTERVALS
    procs = atoll(argv[2]); // GET THE NUMBER OF PROCESSES

    pid_t pids[procs]; // ARRAY TO STORE THE PIDS

    key_t clave;
    int shmid;

    // USE A FIXED KEY (1234)
    clave = 1234;

    // CREATE SHARED MEMORY SEGMENT OF SIZE LONG DOUBLE WITH R/W
    shmid = shmget(clave, sizeof(long double), 0666 | IPC_CREAT);

    if (shmid == -1) {
        perror("[!] Error creating shared memory");
        exit(1);
    }

    // ATTACH THE SHARED MEMORY TO PI
    long double *pi = (long double *)shmat(shmid, NULL, 0);

    if (pi == (long double *)-1) {
        perror("[!] Error attaching shared memory");
        exit(1);
    }

    // INITIALIZE PI VALUE TO 0
    *pi = 0.0;

    // UNLINK THE NAMED SEMAPHORE IF IT EXISTS (CLEANUP)
    sem_unlink("/sem_memoria"); 

    // CREATE SEMAPHORE WITH THE VALUE OF 1 FOR MUTUAL EXCLUSION
    sem_t *semaforo = sem_open("/sem_memoria", O_CREAT, 0644, 1);

    if (semaforo == SEM_FAILED) {
        perror("[!] Error creating the semaphore");
        exit(1);
    }

    gettimeofday(&start, NULL); // GET CURRENT TIME

    printf("\n[+] Calculating pi with %lld subintervals and %lld procs...\n",subintervals,procs);
    printf("\n--------------------------------------------------\n");

    // INITIALIZE THE VALUE OF THE PORTION FOR EACH PROCESS
    long long int intervals_portion=0;

    // IF THE NUMBER OF PROCESSES IS LESS THAN THE NUMBER OF SUBINTERVALS CALCULATE THE INTERVALS, OTHERWISE WORK BASED ON THE RESIDUE
    if(subintervals >= procs ){
        intervals_portion = subintervals/procs; // CALCUTE THE INTERVALS FOR EACH PROCESS
    }

    long long int residuo = subintervals % procs; // GET THE REMAINING INTERVALS
    long long int portion; // VARIABLE TO STORE CURRENT PORTION
    long long int current_start[procs+1]; /// ARRAY TO STORE THE STARTING POINT FOR EACH PROCESS
    long long int now; // START VALUE FOR THE PROCESS
    long double tmp; // TEMP VARIABLE TO STORE THE NUMBER RETURNED BY THE VARIABLE
    
    current_start[0]=0; // INITIALIZE THE STARTING POINT AS 0

    for(long long int i=0; i < procs; i++){ /// ITERATE OVER THE PROCESSES
        if(i >= subintervals){ // IF THE CURRENT PROCESS NUMBER IS GREATER THAN THE NUMBER OF SUBINTERVALS FINISH THE "FOR" CICLE
            break;
        }

        // SET THE AVG PORTION FOR THE PROCESS
        portion=intervals_portion; // IF THERE ARE REMAINING INTERVALS ADD 1 TO THE PORTION 
        if(residuo > 0){
            portion+=1;
            residuo-=1; // SUBTRACT 1 FROM THE RESIDUE SINCE THIS SUBINTERVAL NOW HAS A PROCESS
        }

        now=current_start[i]; // CURRENT STARTING POINT
        current_start[i+1]=portion+current_start[i]; // SET THE STARTING POINT FOR THE NEXT PROCESS
        
        pids[i]=fork(); // INITIALIZE THE PROCESS
        
        if (pids[i] == 0){ // CHECK IF THE NEW CHILD IS TRYING TO ENTER HERE
            tmp=calculate_pi(subintervals, portion, now); // CALL THE FUNCTION AND SAVE THE RETURNED VALUE

            // WAIT (Lock) ON THE SEMAPHORE TO ENSURE MUTUAL EXCLUSION
            sem_wait(semaforo);

            *pi += tmp; // ADD THE RESULT TO THE SHARED VARIABLE

            // RELEASE (Unlock) THE SEMAPHORE TO ALLOW OTHER PROCESSES TO ACCESS THE SHARED VALUE
            sem_post(semaforo); 

            // DETACH THE SHARED MEMORY FROM THE PROCESS
            shmdt(pi);

            // EXIT THE CHILD PROCESS
            exit(0);
        }
    }

    // WAIT FOR ALL THE PROCESSES TO COMPLETE THEIR EXECUTION
    for(long long int i=0; i < procs; i++){
        wait(NULL);
    }

    gettimeofday(&end, NULL); // GET CURRENT TIME

    printf("[!] The result is %.15LF :)\n",(*pi*4.0)); // AS THE FORMULA SAYS, MULTIPLY THE AREA BY 4

    // GET THE TIME IT TOOK FOR THE EXECUTION TO COMPLETE
    elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
    
    printf("\n    [i] It lasted:\n\n");
    printf("-----------------");
    printf("|     %6f secs    |",elapsed_time);
    printf("-----------------\n\n\r");

    // CLEANUP
    shmdt(pi);
    shmctl(shmid, IPC_RMID, NULL);
    sem_unlink("/sem_memoria");

    return 0;
}
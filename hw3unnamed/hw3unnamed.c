#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <math.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <sys/mman.h>


void pushersOperations(int pusher_index);
void closeShared();
int chef0operations();
int chef1operations();
int chef2operations();
int chef3operations();
int chef4operations();
int chef5operations();
void wholesalerPrint(char ingr);

pid_t chefs[6];

#define Shared_mem "/Shared"

typedef struct
{
    int signalArrived;

    /*int isMilk;
    int isFlour;
    int isWalnuts;
    int isSugar;*/

    sem_t newIngredient;
    sem_t milk_flour;
    sem_t milk_sugar;
    sem_t milk_walnuts;
    sem_t sugar_walnuts;
    sem_t sugar_flour;
    sem_t flour_walnuts;
    sem_t agentSem;


  /*  sem_t milkSem;
    sem_t flourSem;
    sem_t walnutsSem;
    sem_t sugarSem;
    sem_t mutex;*/
    char ingredients[2];
}structShared;


structShared *sem_shared;
int fd;

void createSharedMemory(){
    fd = shm_open(Shared_mem, O_CREAT | O_RDWR, 0666);
    if (fd < 0)
    {
        perror("shem_open:");
        exit(EXIT_FAILURE);
    }

    if(ftruncate(fd, sizeof(structShared)) == -1 && ((errno != EINTR)))
    {
        perror("ftruncate:");
        exit(EXIT_FAILURE);
    }

    sem_shared = (structShared *)mmap(NULL, sizeof(structShared), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(sem_shared == MAP_FAILED)
    {
        perror("mmap:");
        exit(EXIT_FAILURE);
    }

  /*  sem_shared->c0 = 0;
    sem_shared->c1 = 0;
    sem_shared->c2 = 0;
    sem_shared->c3 = 0;
    sem_shared->c4 = 0;
    sem_shared->c5 = 0;

    sem_shared->isMilk = 0;
    sem_shared->isFlour = 0;
    sem_shared->isWalnuts = 0;
    sem_shared->isSugar = 0;*/

    sem_shared->signalArrived = 0;

    if((sem_init(&(sem_shared->milk_flour), 1, 0) == -1 ) && ((errno != EINTR)))
    {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }

    if((sem_init(&(sem_shared->milk_sugar), 1, 0) == -1 ) && ((errno != EINTR)))
    {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }

    if((sem_init(&(sem_shared->milk_walnuts), 1, 0) == -1 ) && ((errno != EINTR)))
    {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }

    if((sem_init(&(sem_shared->sugar_walnuts), 1, 0) == -1 ) && ((errno != EINTR)))
    {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }

    if((sem_init(&(sem_shared->sugar_flour), 1, 0) == -1 ) && ((errno != EINTR)))
    {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }

    if((sem_init(&(sem_shared->flour_walnuts), 1, 0) == -1 ) && ((errno != EINTR)))
    {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }

    if((sem_init(&(sem_shared->agentSem), 1, 1) == -1 ) && ((errno != EINTR)))
    {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }


    if((sem_init(&(sem_shared->newIngredient), 1, 0) == -1 ) && ((errno != EINTR)))
    {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }

    sem_shared->ingredients[0] = '\0';
    sem_shared->ingredients[1] = '\0';

   /* if((sem_init(&(sem_shared->milkSem), 1, 0) == -1 ) && ((errno != EINTR)))
    {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }

    if((sem_init(&(sem_shared->flourSem), 1, 0) == -1 ) && ((errno != EINTR)))
    {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }

    if((sem_init(&(sem_shared->walnutsSem), 1, 0) == -1 ) && ((errno != EINTR)))
    {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }

    if((sem_init(&(sem_shared->sugarSem), 1, 0) == -1 ) && ((errno != EINTR)))
    {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }

    if((sem_init(&(sem_shared->mutex), 1, 1) == -1 ) && ((errno != EINTR)))
    {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
*/

}

void closeShared(){
    if(munmap(sem_shared, sizeof(structShared)) == -1)
    {
        perror("munmap: ");
        exit(EXIT_FAILURE);
    }

    if(close(fd) == -1)
    {
        perror("fd close: ");
        exit(EXIT_FAILURE);
    }

    if(shm_unlink(Shared_mem) == -1)
    {
        perror("fd close: ");
        exit(EXIT_FAILURE);
    }
}


void handler(int sig){
    if(sig == SIGUSR1)
    {
        (sem_shared->signalArrived)++;
    }
}


int main(int argc,char *argv[])
{
    int opt;
    char inputFilePath[120];
    while((opt = getopt(argc, argv, ":i:")) != -1)  
    {  
        switch(opt)  
        {  
            case 'i':
                strcpy(inputFilePath,optarg);
                break;
            case ':':  
                errno=EINVAL;
                fprintf(stderr, "Wrong format.\n" );
                fprintf(stderr, "Usage: %s -i inputPath\n", argv[0]);
                exit(EXIT_FAILURE);     
                break;  
            case '?':  
                errno=EINVAL;
                fprintf(stderr, "Wrong format.\n" );
                fprintf(stderr, "Usage: %s -i inputPath\n", argv[0]);
                exit(EXIT_FAILURE); 
                break; 
            case -1:
                break;
            default:
                abort (); 
        }
    }

    if(optind!=3){
        errno=EINVAL;
        fprintf(stderr, "Wrong format.\n" );
        fprintf(stderr, "Usage: %s -i inputPath\n", argv[0]);
        exit(EXIT_FAILURE); 
    }

    createSharedMemory();

    pid_t pushers[1];
    int j;
    int m = 1;

    for (j = 0; j < m; ++j) {
        if ((pushers[j] = fork()) < 0) {
            perror("fork");
            abort();
        } else if (pushers[j] == 0) { //child
            pushers[j] = getpid();
            struct sigaction newactPushers;
            newactPushers.sa_handler = &handler;
            newactPushers.sa_flags = 0;
            if((sigemptyset(&newactPushers.sa_mask) == -1) || (sigaction(SIGUSR1, &newactPushers, NULL) == -1)){
                perror("Failled to install SIGUSR1 signal handler");
                exit(EXIT_FAILURE);
            }

            pushersOperations(j);

        }
    }


    int i;
    int n = 6;

    for (i = 0; i < n; ++i) {
        if ((chefs[i] = fork()) < 0) {
            perror("fork");
            abort();
        } else if (chefs[i] == 0) { //child
            chefs[i] = getpid();
            struct sigaction newactChef;
            newactChef.sa_handler = &handler;
            newactChef.sa_flags = 0;
            if((sigemptyset(&newactChef.sa_mask) == -1) || (sigaction(SIGUSR1, &newactChef, NULL) == -1)){
                perror("Failled to install SIGUSR1 signal handler");
                exit(EXIT_FAILURE);
            }
            if(i == 0){
                printf("chef0 (pid %d) is waiting for walnuts and sugar.    ingredients in array:[%c][%c]\n", getpid(), sem_shared->ingredients[0],sem_shared->ingredients[1]);
                return chef0operations();
            }
            else if(i == 1){
                printf("chef1 (pid %d) is waiting for flour and walnuts.    ingredients in array:[%c][%c]\n", getpid(), sem_shared->ingredients[0],sem_shared->ingredients[1]);
                return chef1operations();
            }
            else if(i == 2){
                printf("chef2 (pid %d) is waiting for sugar and flour.  ingredients in array:[%c][%c]\n", getpid(), sem_shared->ingredients[0],sem_shared->ingredients[1]);
                return chef2operations();
            }
            else if(i == 3){
                printf("chef3 (pid %d) is waiting for milk and flour.   ingredients in array: [%c][%c]\n", getpid(), sem_shared->ingredients[0],sem_shared->ingredients[1]);
                return chef3operations();
            }
            else if(i == 4){
                printf("chef4 (pid %d) waiting for milk and walnuts.    ingredients in array:[%c][%c]\n", getpid(), sem_shared->ingredients[0],sem_shared->ingredients[1]);
                return chef4operations();
            }
            else if(i == 5){
                printf("chef5 (pid %d) is waiting for sugar and milk.   ingredients in array:[%c][%c]\n", getpid(), sem_shared->ingredients[0],sem_shared->ingredients[1]);
                return chef5operations();
                
            }
        }
    }
    sleep(1);
    i=0;
    FILE *fp;
    if ((fp = fopen (inputFilePath, "r")) == NULL) {
         printf("fopen error!");
         exit(EXIT_FAILURE);
    }

    char c;
    while (1)
    {
        if(sem_wait(&(sem_shared->agentSem)) == -1)
        {
            fprintf(stderr, "sem_wait\n");
            exit(EXIT_FAILURE);
        }
        c = fgetc(fp);
        if(c == EOF)
            break;
        if(c == '\n'){
            c = fgetc(fp);
            if(c == EOF)
                break;
        }
        if( (c != '\n')){ 
            sem_shared->ingredients[0] = c;
            sem_shared->ingredients[1] = fgetc(fp);

            printf("the wholesaler (pid %d) delivers ", getpid());
            wholesalerPrint(sem_shared->ingredients[0]);
            printf(" and ");
            wholesalerPrint(sem_shared->ingredients[1]);
            printf("\n");
            printf("the wholesaler (pid %d) is waiting for the dessert.\n", getpid());
           
            if(sem_post(&(sem_shared->newIngredient)) == -1)
            {
                fprintf(stderr, "sem_post\n");
                exit(EXIT_FAILURE);
            }
            
            /*else if(sem_shared->ingredients[0] == 'W'){
                if(sem_post(&(sem_shared->walnutsSem)) == -1)
                {
                    fprintf(stderr, "sem_post\n");
                    exit(EXIT_FAILURE);
                }
            }
            else if(sem_shared->ingredients[0] == 'F'){
                if(sem_post(&(sem_shared->flourSem)) == -1)
                {
                    fprintf(stderr, "sem_post\n");
                    exit(EXIT_FAILURE);
                }
            }
            else if(sem_shared->ingredients[0] == 'M'){
                if(sem_post(&(sem_shared->milkSem)) == -1)
                {
                    fprintf(stderr, "sem_post\n");
                    exit(EXIT_FAILURE);
                }
            }

            if(sem_shared->ingredients[1] == 'S'){
                if(sem_post(&(sem_shared->sugarSem)) == -1)
                {
                    fprintf(stderr, "sem_post\n");
                    exit(EXIT_FAILURE);
                }
            }
            else if(sem_shared->ingredients[1] == 'W'){
                if(sem_post(&(sem_shared->walnutsSem)) == -1)
                {
                    fprintf(stderr, "sem_post\n");
                    exit(EXIT_FAILURE);
                }
            }
            else if(sem_shared->ingredients[1] == 'F'){
                if(sem_post(&(sem_shared->flourSem)) == -1)
                {
                    fprintf(stderr, "sem_post\n");
                    exit(EXIT_FAILURE);
                }
            }
            else if(sem_shared->ingredients[1] == 'M'){
                if(sem_post(&(sem_shared->milkSem)) == -1)
                {
                    fprintf(stderr, "sem_post\n");
                    exit(EXIT_FAILURE);
                }
            }*/
        }
    }
    fclose (fp);
    sleep(1);
   
    int k1;
    for(k1=0; k1<6; ++k1){
        kill(chefs[k1], SIGUSR1);
    }
    for(k1=0; k1<1; ++k1){
        kill(pushers[k1], SIGUSR1);
    }

    int  k, status,totalDessert=0;
    size_t childPid;

    for (k = 0; k < 6; ++k)
    {
        childPid = waitpid(chefs[k], &status, 0);
         if (childPid == -1) {
            perror("wait error");
            exit(EXIT_FAILURE); 
        }
        totalDessert += WEXITSTATUS(status);
    }
    
    childPid = waitpid(pushers[0], &status, 0);
     if (childPid == -1) {
        perror("wait error");
        exit(EXIT_FAILURE); 
    }

    printf("the wholesaler (pid %d) is done (total desserts: %d).\n", getpid(), totalDessert);
    closeShared();
   
    return 0;
}

void wholesalerPrint(char ingr){
    if(ingr == 'F')
        printf("flour");
    else if(ingr == 'W')
        printf("walnuts");
    else if(ingr == 'S')
        printf("sugar");
    else if(ingr == 'M')
        printf("milk");
}

void removeFromArray(int chefNum){
    if(sem_shared->ingredients[0] == 'S'){
        sem_shared->ingredients[0] = '\0';
        printf("chef%d (pid %d) taken the sugar.    ingredients in array:[%c][%c]\n", chefNum, getpid(), sem_shared->ingredients[0],sem_shared->ingredients[1]);
    }
    else if(sem_shared->ingredients[0] == 'W'){
        sem_shared->ingredients[0] = '\0';
        printf("chef%d (pid %d) taken the walnuts.  ingredients in array:[%c][%c]\n", chefNum, getpid(), sem_shared->ingredients[0],sem_shared->ingredients[1]);
    }
    else if(sem_shared->ingredients[0] == 'M'){
        sem_shared->ingredients[0] = '\0';
        printf("chef%d (pid %d) taken the milk. ingredients in array:[%c][%c]\n", chefNum, getpid(), sem_shared->ingredients[0],sem_shared->ingredients[1]);
    }
    else if(sem_shared->ingredients[0] == 'F'){
        sem_shared->ingredients[0] = '\0';
        printf("chef%d (pid %d) taken the flour.    ingredients in array:[%c][%c]\n", chefNum, getpid(), sem_shared->ingredients[0],sem_shared->ingredients[1]);
    }

    if(sem_shared->ingredients[1] == 'S'){
        sem_shared->ingredients[1] = '\0';
        printf("chef%d (pid %d) taken the sugar.    ingredients in array:[%c][%c]\n", chefNum, getpid(), sem_shared->ingredients[0],sem_shared->ingredients[1]);
    }
    else if(sem_shared->ingredients[1] == 'W'){
        sem_shared->ingredients[1] = '\0';
        printf("chef%d (pid %d) taken the walnuts.  ingredients in array:[%c][%c]\n", chefNum, getpid(), sem_shared->ingredients[0],sem_shared->ingredients[1]);
    }
    else if(sem_shared->ingredients[1] == 'M'){
        sem_shared->ingredients[1] = '\0';
        printf("chef%d (pid %d) taken the milk. ingredients in array:[%c][%c]\n", chefNum, getpid(), sem_shared->ingredients[0],sem_shared->ingredients[1]);
    }
    else if(sem_shared->ingredients[1] == 'F'){
        sem_shared->ingredients[1] = '\0';
        printf("chef%d (pid %d) taken the flour.    ingredients in array:[%c][%c]\n", chefNum, getpid(), sem_shared->ingredients[0],sem_shared->ingredients[1]);
    }
}

int chef0operations(){
    int dessertNum=0;
    for(;;)
    {  
        if(sem_wait(&(sem_shared->milk_flour)) == -1)
        {
            if(sem_shared->signalArrived != 0){
                printf("chef0 (pid %d) is exiting.  ingredients in array:[%c][%c]\n",getpid(), sem_shared->ingredients[0],sem_shared->ingredients[1]);
                return dessertNum;
            }
            fprintf(stderr, "sem_wait\n");
            closeShared();
            exit(EXIT_FAILURE);
        } 
        dessertNum++;
        removeFromArray(0);
       // printf("chef0 (pid %d) has taken the walnuts\nchef0 (pid %d) has taken the sugar\n", getpid(), getpid());
        printf("chef0 (pid %d) is preparing the dessert.    ingredients in array:[%c][%c]\n", getpid(), sem_shared->ingredients[0],sem_shared->ingredients[1]);
        printf("chef0 (pid %d) has delivered the dessert.   ingredients in array:[%c][%c]\n", getpid(), sem_shared->ingredients[0],sem_shared->ingredients[1]);
        printf("the wholesaler (pid %d) has obtained the dessert and left.\n", getppid());

        if(sem_post(&(sem_shared->agentSem)) == -1)
        {
            fprintf(stderr, "sem_post\n");
            closeShared();
            exit(EXIT_FAILURE);
        }
    }
}

int chef1operations(){
    int dessertNum=0;

    for(;;)
    {
        if(sem_wait(&(sem_shared->milk_sugar)) == -1)
        {
            if(sem_shared->signalArrived != 0){
                printf("chef1 (pid %d) is exiting.  ingredients in array:[%c][%c]\n",getpid(), sem_shared->ingredients[0],sem_shared->ingredients[1]);
                return dessertNum;
            }
            fprintf(stderr, "sem_wait\n");
            closeShared();
            exit(EXIT_FAILURE);
        } 
        dessertNum++;
        removeFromArray(1);
       // printf("chef1 (pid %d) taken the flour\nchef1 (pid %d) taken the walnuts\n", getpid(), getpid());
        printf("chef1 (pid %d) is preparing the dessert.    ingredients in array:[%c][%c]\n", getpid(), sem_shared->ingredients[0],sem_shared->ingredients[1]);
        printf("chef1 (pid %d) has delivered the dessert.   ingredients in array:[%c][%c]\n", getpid(), sem_shared->ingredients[0],sem_shared->ingredients[1]);
        printf("the wholesaler (pid %d) has obtained the dessert and left.\n", getppid());

        if(sem_post(&(sem_shared->agentSem)) == -1)
        {
            fprintf(stderr, "sem_post\n");
            closeShared();
            exit(EXIT_FAILURE);
        }
    }
}

int chef2operations(){
    int dessertNum=0;
    for(;;)
    {
        if(sem_wait(&(sem_shared->milk_walnuts)) == -1)
        {
            if(sem_shared->signalArrived != 0){
                printf("chef2 (pid %d) is exiting.  ingredients in array:[%c][%c]\n",getpid(), sem_shared->ingredients[0],sem_shared->ingredients[1]);
                return dessertNum;
            }
            fprintf(stderr, "sem_wait\n");
            closeShared();
            exit(EXIT_FAILURE);
        } 
        dessertNum++;
        removeFromArray(2);
       // printf("chef2 (pid %d) taken the sugar\nchef2 (pid %d) taken the flour\n", getpid(), getpid());
        printf("chef2 (pid %d) is preparing the dessert.    ingredients in array:[%c][%c]\n", getpid(), sem_shared->ingredients[0],sem_shared->ingredients[1]);
        printf("chef2 (pid %d) has delivered the dessert.   ingredients in array:[%c][%c]\n", getpid(), sem_shared->ingredients[0],sem_shared->ingredients[1]);
        printf("the wholesaler (pid %d) has obtained the dessert and left.\n", getppid());

        if(sem_post(&(sem_shared->agentSem)) == -1)
        {
            fprintf(stderr, "sem_post\n");
            closeShared();
            exit(EXIT_FAILURE);
        }
    }

}

int chef3operations(){

    int dessertNum=0;
    for(;;)
    {
        if(sem_wait(&(sem_shared->sugar_walnuts)) == -1)
        {
            if(sem_shared->signalArrived != 0){
                printf("chef3 (pid %d) is exiting.  ingredients in array:[%c][%c]\n",getpid(), sem_shared->ingredients[0],sem_shared->ingredients[1]);
                return dessertNum;
            }
            fprintf(stderr, "sem_wait\n");
            closeShared();
            exit(EXIT_FAILURE);
        }
        dessertNum++;
        removeFromArray(3);
       // printf("chef3 (pid %d) taken the milk\nchef3 (pid %d) taken the flour\n", getpid(), getpid());
        printf("chef3 (pid %d) is preparing the dessert.    ingredients in array:[%c][%c]\n", getpid(), sem_shared->ingredients[0],sem_shared->ingredients[1]);
        printf("chef3 (pid %d) has delivered the dessert.   ingredients in array:[%c][%c]\n", getpid(), sem_shared->ingredients[0],sem_shared->ingredients[1]);
        printf("the wholesaler (pid %d) has obtained the dessert and left.\n", getppid());

        if(sem_post(&(sem_shared->agentSem)) == -1)
        {
            fprintf(stderr, "sem_post\n");
            closeShared();
            exit(EXIT_FAILURE);
        }
    }
}

int chef4operations(){
    int dessertNum=0;
    for(;;)
    {
        if(sem_wait(&(sem_shared->sugar_flour)) == -1)
        {
            if(sem_shared->signalArrived != 0){
                printf("chef4 (pid %d) is exiting.  ingredients in array:[%c][%c]\n",getpid(), sem_shared->ingredients[0],sem_shared->ingredients[1]);
                return dessertNum;
            }
            fprintf(stderr, "sem_wait\n");
            closeShared();
            exit(EXIT_FAILURE);
        } 
        dessertNum++;
        removeFromArray(4);
        //printf("chef4 (pid %d) is taken the milk\nchef4 (pid %d) taken the walnuts\n", getpid(),getpid());
        printf("chef4 (pid %d) is preparing the dessert.    ingredients in array:[%c][%c]\n", getpid(), sem_shared->ingredients[0],sem_shared->ingredients[1]);
        printf("chef4 (pid %d) has delivered the dessert.   ingredients in array:[%c][%c]\n", getpid(), sem_shared->ingredients[0],sem_shared->ingredients[1]);
        printf("the wholesaler (pid %d) has obtained the dessert and left.\n", getppid());

        if(sem_post(&(sem_shared->agentSem)) == -1)
        {
            fprintf(stderr, "sem_post\n");
            closeShared();
            exit(EXIT_FAILURE);
        }
    }
}

int chef5operations(){
    int dessertNum=0;
    for(;;)
    {
        if(sem_wait(&(sem_shared->flour_walnuts)) == -1)
        {
            if(sem_shared->signalArrived != 0){
                printf("chef5 (pid %d) is exiting.  ingredients in array:[%c][%c]\n",getpid(), sem_shared->ingredients[0],sem_shared->ingredients[1]);
                return dessertNum;
            }
            fprintf(stderr, "sem_wait\n");
            closeShared();
            exit(EXIT_FAILURE);
        } 
        dessertNum++;
        removeFromArray(5);
        //printf("chef5 (pid %d) taken the sugar\nchef5 (pid %d) taken the milk\n", getpid(),getpid());
        printf("chef5 (pid %d) is preparing the dessert.    ingredients in array:[%c][%c]\n", getpid(), sem_shared->ingredients[0],sem_shared->ingredients[1]);
        printf("chef5 (pid %d) has delivered the dessert.   ingredients in array:[%c][%c]\n", getpid(), sem_shared->ingredients[0],sem_shared->ingredients[1]);
        printf("the wholesaler (pid %d) has obtained the dessert and left.\n", getppid());

        if(sem_post(&(sem_shared->agentSem)) == -1)
        {
            fprintf(stderr, "sem_post\n");
            closeShared();
            exit(EXIT_FAILURE);
        }
    }
}


void pushersOperations(int pusher_index){
    for(;;){
        if(sem_wait(&(sem_shared->newIngredient)) == -1)
        {
            if(sem_shared->signalArrived != 0)
                exit(EXIT_SUCCESS);
            fprintf(stderr, "sem_wait\n");
            exit(EXIT_FAILURE);
        }
        
        if( ((sem_shared->ingredients[0] == 'M') & (sem_shared->ingredients[1] == 'F') ) |  ((sem_shared->ingredients[0] == 'F') & (sem_shared->ingredients[1] == 'M') ))
        {
            if(sem_post(&(sem_shared->sugar_walnuts)) == -1)
            {
                fprintf(stderr, "sem_post\n");
                exit(EXIT_FAILURE);
            }
        }
        else if( ((sem_shared->ingredients[0] == 'M') & (sem_shared->ingredients[1] == 'W') ) |  ((sem_shared->ingredients[0] == 'W') & (sem_shared->ingredients[1] == 'M') ))
        {
            if(sem_post(&(sem_shared->sugar_flour)) == -1)
            {
                fprintf(stderr, "sem_post\n");
                exit(EXIT_FAILURE);
            }
        }
        else if( ((sem_shared->ingredients[0] == 'M') & (sem_shared->ingredients[1] == 'S') ) |  ((sem_shared->ingredients[0] == 'S') & (sem_shared->ingredients[1] == 'M') ))
        {
            if(sem_post(&(sem_shared->flour_walnuts)) == -1)
            {
                fprintf(stderr, "sem_post\n");
                exit(EXIT_FAILURE);
            }
        }
        else if( ((sem_shared->ingredients[0] == 'S') & (sem_shared->ingredients[1] == 'W') ) |  ((sem_shared->ingredients[0] == 'W') & (sem_shared->ingredients[1] == 'S') ))
        {
            if(sem_post(&(sem_shared->milk_flour)) == -1)
            {
                fprintf(stderr, "sem_post\n");
                exit(EXIT_FAILURE);
            }
        }
        else if( ((sem_shared->ingredients[0] == 'S') & (sem_shared->ingredients[1] == 'F') ) |  ((sem_shared->ingredients[0] == 'F') & (sem_shared->ingredients[1] == 'S') ))
        {
            if(sem_post(&(sem_shared->milk_walnuts)) == -1)
            {
                fprintf(stderr, "sem_post\n");
                exit(EXIT_FAILURE);
            }
        }
        else if( ((sem_shared->ingredients[0] == 'W') & (sem_shared->ingredients[1] == 'F') ) |  ((sem_shared->ingredients[0] == 'F') & (sem_shared->ingredients[1] == 'W') ))
        {
            if(sem_post(&(sem_shared->milk_sugar)) == -1)
            {
                fprintf(stderr, "sem_post\n");
                exit(EXIT_FAILURE);
            }
        }

    }
/*
    //Pusher A - Milk
    if(pusher_index == 0){
        for(;;){
            
            if(sem_wait(&(sem_shared->milkSem)) == -1)
            {
                if(sem_shared->signalArrived != 0)
                    exit(EXIT_SUCCESS);
                fprintf(stderr, "sem_wait\n");
                exit(EXIT_FAILURE);
            }
            if(sem_wait(&(sem_shared->mutex)) == -1)
            {
                fprintf(stderr, "sem_wait\n");
                exit(EXIT_FAILURE);
            }
            if(sem_shared->isFlour){
                sem_shared->isFlour = 0;
                if(sem_post(&(sem_shared->sugar_walnuts)) == -1)
                {
                    fprintf(stderr, "sem_post\n");
                    exit(EXIT_FAILURE);
                }
            }
            if(sem_shared->isWalnuts){
                sem_shared->isWalnuts = 0;
                if(sem_post(&(sem_shared->sugar_flour)) == -1)
                {
                    fprintf(stderr, "sem_post\n");
                    exit(EXIT_FAILURE);
                }
            }
            if(sem_shared->isSugar){
                sem_shared->isSugar = 0;
                if(sem_post(&(sem_shared->flour_walnuts)) == -1)
                {
                    fprintf(stderr, "sem_post\n");
                    exit(EXIT_FAILURE);
                }
            }
            else{
                sem_shared->isMilk = 1;
            }

            if(sem_post(&(sem_shared->mutex)) == -1)
            {
                fprintf(stderr, "sem_post\n");
                exit(EXIT_FAILURE);
            }
        }
    }

    //Pusher B - Flour
    else if(pusher_index == 1){
        for(;;){
            
            if(sem_wait(&(sem_shared->flourSem)) == -1)
            {
                if(sem_shared->signalArrived != 0)
                    exit(EXIT_SUCCESS);
                fprintf(stderr, "sem_wait\n");
                exit(EXIT_FAILURE);
            }
            if(sem_wait(&(sem_shared->mutex)) == -1)
            {
                fprintf(stderr, "sem_wait\n");
                exit(EXIT_FAILURE);
            }

            if(sem_shared->isMilk){
                sem_shared->isMilk = 0;
                if(sem_post(&(sem_shared->sugar_walnuts)) == -1)
                {
                    fprintf(stderr, "sem_post\n");
                    exit(EXIT_FAILURE);
                }
            }

            if(sem_shared->isWalnuts){
                sem_shared->isWalnuts = 0;
                if(sem_post(&(sem_shared->milk_sugar)) == -1)
                {
                    fprintf(stderr, "sem_post\n");
                    exit(EXIT_FAILURE);
                }
            }

            if(sem_shared->isSugar){
                sem_shared->isSugar = 0;
                if(sem_post(&(sem_shared->milk_walnuts)) == -1)
                {
                    fprintf(stderr, "sem_post\n");
                    exit(EXIT_FAILURE);
                }
            }

            else{
                sem_shared->isFlour = 1;
            }
            if(sem_post(&(sem_shared->mutex)) == -1)
            {
                fprintf(stderr, "sem_post\n");
                exit(EXIT_FAILURE);
            }
        }
    }

    //Pusher C - Walnuts
    else if(pusher_index == 2){
        for(;;){
            
            if(sem_wait(&(sem_shared->walnutsSem)) == -1)
            {
                if(sem_shared->signalArrived != 0)
                    exit(EXIT_SUCCESS);
                fprintf(stderr, "sem_wait\n");
                exit(EXIT_FAILURE);
            }

            if(sem_wait(&(sem_shared->mutex)) == -1)
            {
                fprintf(stderr, "sem_wait\n");
                exit(EXIT_FAILURE);
            }

            if(sem_shared->isMilk){
                sem_shared->isMilk = 0;
                if(sem_post(&(sem_shared->sugar_flour)) == -1)
                {
                    fprintf(stderr, "sem_post\n");
                    exit(EXIT_FAILURE);
                }
            }

            if(sem_shared->isSugar){
                sem_shared->isSugar = 0;
                if(sem_post(&(sem_shared->milk_flour)) == -1)
                {
                    fprintf(stderr, "sem_post\n");
                    exit(EXIT_FAILURE);
                }
            }

            if(sem_shared->isFlour){
                sem_shared->isFlour = 0;
                if(sem_post(&(sem_shared->milk_sugar)) == -1)
                {
                    fprintf(stderr, "sem_post\n");
                    exit(EXIT_FAILURE);
                }
            }

            else{
                sem_shared->isWalnuts = 1;
            }

            if(sem_post(&(sem_shared->mutex)) == -1)
            {
                fprintf(stderr, "sem_post\n");
                exit(EXIT_FAILURE);
            }
        }

    }

    //Pusher D - Sugar
    else if(pusher_index == 3){
        for(;;){
            
            if(sem_wait(&(sem_shared->sugarSem)) == -1)
            {
                if(sem_shared->signalArrived != 0)
                    exit(EXIT_SUCCESS);
                fprintf(stderr, "sem_wait\n");
                exit(EXIT_FAILURE);
            }

            if(sem_wait(&(sem_shared->mutex)) == -1)
            {
                fprintf(stderr, "sem_wait\n");
                exit(EXIT_FAILURE);
            }

            if(sem_shared->isMilk){
                sem_shared->isMilk = 0;
                if(sem_post(&(sem_shared->flour_walnuts)) == -1)
                {
                    fprintf(stderr, "sem_post\n");
                    exit(EXIT_FAILURE);
                }
            }

            if(sem_shared->isFlour){
                sem_shared->isFlour = 0;
                if(sem_post(&(sem_shared->milk_walnuts)) == -1)
                {
                    fprintf(stderr, "sem_post\n");
                    exit(EXIT_FAILURE);
                }
            }

            if(sem_shared->isWalnuts){
                sem_shared->isWalnuts = 0;
                if(sem_post(&(sem_shared->milk_flour)) == -1)
                {
                    fprintf(stderr, "sem_post\n");
                    exit(EXIT_FAILURE);
                }
            }

            else{
                sem_shared->isSugar = 1;
            }

            if(sem_post(&(sem_shared->mutex)) == -1)
            {
                fprintf(stderr, "sem_post\n");
                exit(EXIT_FAILURE);
            }

        }
    }
*/
}
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <math.h>
#include <sys/stat.h>
#include <semaphore.h>


void createSemaphores();
void closeSemaphoes();
void closeShared();
void pushersOperations();
int chef0operations();
int chef1operations();
int chef2operations();
int chef3operations();
int chef4operations();
int chef5operations();
void wholesalerPrint(char ingr);
void removeFromArray(int chefNum);

#define chef0 "/milk_flour1"
#define chef1 "/milk_sugar1"
#define chef2 "/milk_walnuts1"
#define chef3 "/sugar_walnuts1"
#define chef4 "/sugar_flour1"
#define chef5 "/flour_walnuts1"
#define agentName "/agentSem1"


sem_t* newIngredient;
sem_t* milk_flour;
sem_t* milk_sugar;
sem_t* milk_walnuts;
sem_t* sugar_walnuts;
sem_t* sugar_flour;
sem_t* flour_walnuts;
sem_t* agentSem;


#define Shared_mem "/SharedNamed2"

typedef struct
{
    int signalArrived;
    char ingredients[2];
}structSharedNamed;


structSharedNamed *sem_shared;
int fd;
void createSharedMemory(){
    fd = shm_open(Shared_mem, O_CREAT | O_RDWR, 0666);
    if (fd < 0)
    {
        perror("shem_open:");
        exit(EXIT_FAILURE);
    }

    if(ftruncate(fd, sizeof(structSharedNamed)) == -1 && ((errno != EINTR)))
    {
        perror("ftruncate:");
        exit(EXIT_FAILURE);
    }

    sem_shared = (structSharedNamed *)mmap(NULL, sizeof(structSharedNamed), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(sem_shared == MAP_FAILED)
    {
        perror("mmap:");
        exit(EXIT_FAILURE);
    }

    sem_shared->signalArrived = 0;
}

char ingredientsName[70];

void closeSemaphoes(){
    sem_close(newIngredient);
    sem_close(milk_flour);
    sem_close(milk_sugar);
    sem_close(milk_walnuts);
    sem_close(sugar_walnuts);
    sem_close(sugar_flour);
    sem_close(flour_walnuts);
    sem_close(agentSem);

    sem_unlink(ingredientsName);
    sem_unlink(chef0);
    sem_unlink(chef1);
    sem_unlink(chef2);
    sem_unlink(chef3);
    sem_unlink(chef4);
    sem_unlink(chef5);
    sem_unlink(agentName);
}

void handler(int sig){
    if(sig == SIGUSR1)
    {
        (sem_shared->signalArrived)++;
    }
}

pid_t chefs[6];

int main(int argc,char *argv[])
{
    int opt;
    char inputFilePath[120];

    while((opt = getopt(argc, argv, ":i:n:")) != -1)  
    {  
        switch(opt)  
        {  
            case 'i':
                strcpy(inputFilePath,optarg);
                break;
            case 'n':
                strcpy(ingredientsName,optarg);
                break;
            case ':':  
                errno=EINVAL;
                fprintf(stderr, "Wrong format.\n" );
                fprintf(stderr, "Usage: %s -i inputPath -n named\n", argv[0]);
                exit(EXIT_FAILURE);     
                break;  
            case '?':  
                errno=EINVAL;
                fprintf(stderr, "Wrong format.\n" );
                fprintf(stderr, "Usage: %s -i inputPath -n named\n", argv[0]);
                exit(EXIT_FAILURE); 
                break; 
            case -1:
                break;
            default:
                abort (); 
        }
    }

    if(optind!=5){
        errno=EINVAL;
        fprintf(stderr, "Wrong format.\n" );
        fprintf(stderr, "Usage: %s -i inputPath -n named\n", argv[0]);
        exit(EXIT_FAILURE); 
    }
    createSemaphores();
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
                closeShared();
                closeSemaphoes();
                exit(EXIT_FAILURE);
            }

            pushersOperations();

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
                closeShared();
                closeSemaphoes();
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
        if(sem_wait((agentSem)) == -1)
        {
            fprintf(stderr, "sem_wait\n");
            closeShared();
            closeSemaphoes();
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
           
            if(sem_post((newIngredient)) == -1)
            {
                fprintf(stderr, "sem_post\n");
                closeShared();
                closeSemaphoes();
                exit(EXIT_FAILURE);
            }
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
   
    int  k, status, childPid,totalDessert=0;

    for (k = 0; k < 6; ++k)
    {
        childPid = waitpid(chefs[k], &status, 0);
         if (childPid == -1) {
            perror("waitpid error");
            closeShared();
            closeSemaphoes();
            exit(EXIT_FAILURE); 
        }
        totalDessert += WEXITSTATUS(status);
    }
    
    childPid = waitpid(pushers[0], &status, 0);
     if (childPid == -1) {
        perror("wait error");
        closeShared();
        closeSemaphoes();
        exit(EXIT_FAILURE); 
    }

    printf("the wholesaler (pid %d) is done (total desserts: %d).\n", getpid(), totalDessert);
    closeShared();
    closeSemaphoes();
    return 0;


}

void closeShared(){
    if(munmap(sem_shared, sizeof(structSharedNamed)) == -1)
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

void createSemaphores(){
    newIngredient = sem_open(ingredientsName,O_CREAT, 0644, 0); // create / open semaphore 
    if (newIngredient == SEM_FAILED){
        perror("sem open");
        exit(EXIT_FAILURE);
    }


    milk_flour = sem_open(chef0,O_CREAT, 0644, 0); // create / open semaphore 
    if (milk_flour == SEM_FAILED){
        perror("sem open");
        exit(EXIT_FAILURE);
    }

    milk_sugar = sem_open(chef1,O_CREAT, 0644, 0); // create / open semaphore 
    if (milk_sugar == SEM_FAILED){
        perror("sem open");
        exit(EXIT_FAILURE);
    }

    milk_walnuts = sem_open(chef2,O_CREAT, 0644, 0); // create / open semaphore 
    if (milk_walnuts == SEM_FAILED){
        perror("sem open");
        exit(EXIT_FAILURE);
    }

    sugar_walnuts = sem_open(chef3,O_CREAT, 0644, 0); // create / open semaphore 
    if (sugar_walnuts == SEM_FAILED){
        perror("sem open");
        exit(EXIT_FAILURE);
    }

    sugar_flour = sem_open(chef4,O_CREAT, 0644, 0); // create / open semaphore 
    if (sugar_flour == SEM_FAILED){
        perror("sem open");
        exit(EXIT_FAILURE);
    }

    flour_walnuts = sem_open(chef5,O_CREAT, 0644, 0); // create / open semaphore 
    if (flour_walnuts == SEM_FAILED){
        perror("sem open");
        exit(EXIT_FAILURE);
    }

    agentSem = sem_open(agentName,O_CREAT, 0644, 1); // create / open semaphore 
    if (agentSem == SEM_FAILED){
        perror("sem open");
        exit(EXIT_FAILURE);
    }
}


int chef0operations(){
    int dessertNum=0;
    for(;;)
    {  
        if(sem_wait((milk_flour)) == -1)
        {
            if(sem_shared->signalArrived != 0){
                printf("chef0 (pid %d) is exiting.  ingredients in array:[%c][%c]\n",getpid(), sem_shared->ingredients[0],sem_shared->ingredients[1]);
                closeSemaphoes();
                return dessertNum;
            }
            closeShared();
            closeSemaphoes();
            fprintf(stderr, "sem_wait\n");
            exit(EXIT_FAILURE);
        } 
        dessertNum++;
        removeFromArray(0);
       // printf("chef0 (pid %d) has taken the walnuts\nchef0 (pid %d) has taken the sugar\n", getpid(), getpid());
        printf("chef0 (pid %d) is preparing the dessert.    ingredients in array:[%c][%c]\n", getpid(), sem_shared->ingredients[0],sem_shared->ingredients[1]);
        printf("chef0 (pid %d) has delivered the dessert.   ingredients in array:[%c][%c]\n", getpid(), sem_shared->ingredients[0],sem_shared->ingredients[1]);
        printf("the wholesaler (pid %d) has obtained the dessert and left.\n", getppid());

        if(sem_post((agentSem)) == -1)
        {
            fprintf(stderr, "sem_post\n");
            closeShared();
            closeSemaphoes();
            exit(EXIT_FAILURE);
        }
    }
}

int chef1operations(){
    int dessertNum=0;
    for(;;)
    {
        if(sem_wait((milk_sugar)) == -1)
        {
            if(sem_shared->signalArrived != 0){
                printf("chef1 (pid %d) is exiting.  ingredients in array:[%c][%c]\n",getpid(), sem_shared->ingredients[0],sem_shared->ingredients[1]);
                closeSemaphoes();
                return dessertNum;
            }
            fprintf(stderr, "sem_wait\n");
            closeShared();
            closeSemaphoes();
            exit(EXIT_FAILURE);
        } 
        dessertNum++;
        removeFromArray(1);
       // printf("chef1 (pid %d) taken the flour\nchef1 (pid %d) taken the walnuts\n", getpid(), getpid());
        printf("chef1 (pid %d) is preparing the dessert.    ingredients in array:[%c][%c]\n", getpid(), sem_shared->ingredients[0],sem_shared->ingredients[1]);
        printf("chef1 (pid %d) has delivered the dessert.   ingredients in array:[%c][%c]\n", getpid(), sem_shared->ingredients[0],sem_shared->ingredients[1]);
        printf("the wholesaler (pid %d) has obtained the dessert and left.\n", getppid());

        if(sem_post((agentSem)) == -1)
        {
            fprintf(stderr, "sem_post\n");
            closeShared();
            closeSemaphoes();
            exit(EXIT_FAILURE);
        }
    }
}

int chef2operations(){
    int dessertNum=0;

    for(;;)
    {
        if(sem_wait((milk_walnuts)) == -1)
        {
            if(sem_shared->signalArrived != 0){
                printf("chef2 (pid %d) is exiting.  ingredients in array:[%c][%c]\n",getpid(), sem_shared->ingredients[0],sem_shared->ingredients[1]);
                closeSemaphoes();
                return dessertNum;
            }
            fprintf(stderr, "sem_wait\n");
            closeShared();
            closeSemaphoes();
            exit(EXIT_FAILURE);
        } 
        dessertNum++;
        removeFromArray(2);
       // printf("chef2 (pid %d) taken the sugar\nchef2 (pid %d) taken the flour\n", getpid(), getpid());
        printf("chef2 (pid %d) is preparing the dessert.    ingredients in array:[%c][%c]\n", getpid(), sem_shared->ingredients[0],sem_shared->ingredients[1]);
        printf("chef2 (pid %d) has delivered the dessert.   ingredients in array:[%c][%c]\n", getpid(), sem_shared->ingredients[0],sem_shared->ingredients[1]);
        printf("the wholesaler (pid %d) has obtained the dessert and left.\n", getppid());

        if(sem_post((agentSem)) == -1)
        {
            fprintf(stderr, "sem_post\n");
            closeShared();
            closeSemaphoes();
            exit(EXIT_FAILURE);
        }
    }

}

int chef3operations(){
    int dessertNum=0;
    for(;;)
    {
        printf("chef3 (pid %d) is waiting for milk and flour.   ingredients in array: [%c][%c]\n", getpid(), sem_shared->ingredients[0],sem_shared->ingredients[1]);
        if(sem_wait((sugar_walnuts)) == -1)
        {
            if(sem_shared->signalArrived != 0){
                printf("chef3 (pid %d) is exiting.  ingredients in array:[%c][%c]\n",getpid(), sem_shared->ingredients[0],sem_shared->ingredients[1]);
                closeSemaphoes();
                return dessertNum;
            }
            fprintf(stderr, "sem_wait\n");
            closeShared();
            closeSemaphoes();
            exit(EXIT_FAILURE);
        }
        dessertNum++;
        removeFromArray(3);
       // printf("chef3 (pid %d) taken the milk\nchef3 (pid %d) taken the flour\n", getpid(), getpid());
        printf("chef3 (pid %d) is preparing the dessert.    ingredients in array:[%c][%c]\n", getpid(), sem_shared->ingredients[0],sem_shared->ingredients[1]);
        printf("chef3 (pid %d) has delivered the dessert.   ingredients in array:[%c][%c]\n", getpid(), sem_shared->ingredients[0],sem_shared->ingredients[1]);
        printf("the wholesaler (pid %d) has obtained the dessert and left.\n", getppid());

        if(sem_post((agentSem)) == -1)
        {
            fprintf(stderr, "sem_post\n");
            closeShared();
            closeSemaphoes();
            exit(EXIT_FAILURE);
        }
    }
}

int chef4operations(){
    int dessertNum=0;
    for(;;)
    {
        if(sem_wait((sugar_flour)) == -1)
        {
            if(sem_shared->signalArrived != 0){
                printf("chef4 (pid %d) is exiting.  ingredients in array:[%c][%c]\n",getpid(), sem_shared->ingredients[0],sem_shared->ingredients[1]);
                closeSemaphoes();
                return dessertNum;
            }
            fprintf(stderr, "sem_wait\n");
            closeShared();
            closeSemaphoes();
            exit(EXIT_FAILURE);
        } 
        dessertNum++;
        removeFromArray(4);
        //printf("chef4 (pid %d) is taken the milk\nchef4 (pid %d) taken the walnuts\n", getpid(),getpid());
        printf("chef4 (pid %d) is preparing the dessert.    ingredients in array:[%c][%c]\n", getpid(), sem_shared->ingredients[0],sem_shared->ingredients[1]);
        printf("chef4 (pid %d) has delivered the dessert.   ingredients in array:[%c][%c]\n", getpid(), sem_shared->ingredients[0],sem_shared->ingredients[1]);
        printf("the wholesaler (pid %d) has obtained the dessert and left.\n", getppid());

        if(sem_post((agentSem)) == -1)
        {
            fprintf(stderr, "sem_post\n");
            closeShared();
            closeSemaphoes();
            exit(EXIT_FAILURE);
        }
    }
}

int chef5operations(){
    int dessertNum=0;
    for(;;)
    {
        if(sem_wait((flour_walnuts)) == -1)
        {
            if(sem_shared->signalArrived != 0){
                printf("chef5 (pid %d) is exiting.  ingredients in array:[%c][%c]\n",getpid(), sem_shared->ingredients[0],sem_shared->ingredients[1]);
                closeSemaphoes();
                return dessertNum;
            }
            fprintf(stderr, "sem_wait\n");
            closeShared();
            closeSemaphoes();
            exit(EXIT_FAILURE);
        } 
        dessertNum++;
        removeFromArray(5);
        //printf("chef5 (pid %d) taken the sugar\nchef5 (pid %d) taken the milk\n", getpid(),getpid());
        printf("chef5 (pid %d) is preparing the dessert.    ingredients in array:[%c][%c]\n", getpid(), sem_shared->ingredients[0],sem_shared->ingredients[1]);
        printf("chef5 (pid %d) has delivered the dessert.   ingredients in array:[%c][%c]\n", getpid(), sem_shared->ingredients[0],sem_shared->ingredients[1]);
        printf("the wholesaler (pid %d) has obtained the dessert and left.\n", getppid());

        if(sem_post((agentSem)) == -1)
        {
            fprintf(stderr, "sem_post\n");
            closeShared();
            closeSemaphoes();
            exit(EXIT_FAILURE);
        }
    }
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

void pushersOperations(){
    for(;;){
        if(sem_wait((newIngredient)) == -1)
        {
            if(sem_shared->signalArrived != 0){
                closeSemaphoes();
                exit(EXIT_SUCCESS);
            }
            fprintf(stderr, "sem_wait\n");
            closeShared();
            closeSemaphoes();
            exit(EXIT_FAILURE);
        }
        
        if( ((sem_shared->ingredients[0] == 'M') & (sem_shared->ingredients[1] == 'F') ) |  ((sem_shared->ingredients[0] == 'F') & (sem_shared->ingredients[1] == 'M') ))
        {
            if(sem_post((sugar_walnuts)) == -1)
            {
                fprintf(stderr, "sem_post\n");
                closeShared();
                closeSemaphoes();
                exit(EXIT_FAILURE);
            }
        }
        else if( ((sem_shared->ingredients[0] == 'M') & (sem_shared->ingredients[1] == 'W') ) |  ((sem_shared->ingredients[0] == 'W') & (sem_shared->ingredients[1] == 'M') ))
        {
            if(sem_post((sugar_flour)) == -1)
            {
                fprintf(stderr, "sem_post\n");
                closeShared();
                closeSemaphoes();
                exit(EXIT_FAILURE);
            }
        }
        else if( ((sem_shared->ingredients[0] == 'M') & (sem_shared->ingredients[1] == 'S') ) |  ((sem_shared->ingredients[0] == 'S') & (sem_shared->ingredients[1] == 'M') ))
        {
            if(sem_post((flour_walnuts)) == -1)
            {
                fprintf(stderr, "sem_post\n");
                closeShared();
                closeSemaphoes();
                exit(EXIT_FAILURE);
            }
        }
        else if( ((sem_shared->ingredients[0] == 'S') & (sem_shared->ingredients[1] == 'W') ) |  ((sem_shared->ingredients[0] == 'W') & (sem_shared->ingredients[1] == 'S') ))
        {
            if(sem_post((milk_flour)) == -1)
            {
                fprintf(stderr, "sem_post\n");
                closeShared();
                closeSemaphoes();
                exit(EXIT_FAILURE);
            }
        }
        else if( ((sem_shared->ingredients[0] == 'S') & (sem_shared->ingredients[1] == 'F') ) |  ((sem_shared->ingredients[0] == 'F') & (sem_shared->ingredients[1] == 'S') ))
        {
            if(sem_post((milk_walnuts)) == -1)
            {
                fprintf(stderr, "sem_post\n");
                closeShared();
                closeSemaphoes();
                exit(EXIT_FAILURE);
            }
        }
        else if( ((sem_shared->ingredients[0] == 'W') & (sem_shared->ingredients[1] == 'F') ) |  ((sem_shared->ingredients[0] == 'F') & (sem_shared->ingredients[1] == 'W') ))
        {
            if(sem_post((milk_sugar)) == -1)
            {
                fprintf(stderr, "sem_post\n");
                closeShared();
                closeSemaphoes();
                exit(EXIT_FAILURE);
            }
        }

    }
}
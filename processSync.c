#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/sem.h>

#define SIZE 16

/*
 * Lab Assignment 6
 * Controlled Process Synchronization
 *
 * @authors Kasey Stowell, Kehlsey Lewis
 * @version Fall 2018
 *
 */

int main (int argc, char*argv[])
{
   int status;
   long int i, loop, temp, *shmPtr;
   int shmId;
   pid_t pid;

//variables for the semaphore
   int semId;
   struct sembuf buf;


// get value of loop variable (from command-line argument) 
   loop = atoi(argv[1]);

   if ((shmId = shmget (IPC_PRIVATE, SIZE, IPC_CREAT|S_IRUSR|S_IWUSR)) < 0) {
      perror ("i can't get no..\n");
      exit (1);
   }
   if ((shmPtr = shmat (shmId, 0, 0)) == (void*) -1) {
      perror ("can't attach\n");
      exit (1);
   }

   shmPtr[0] = 0;
   shmPtr[1] = 1;

// create semaphore
   if((semId = semget (IPC_PRIVATE, 1, 00600)) < 1) {
      perror("Error initializing semaphore");
      exit(1);
   }

// initialize semaphore
   semctl (semId, 0, SETVAL, 1);

   buf.sem_num = 0; //semaphore ID
   buf.sem_op = 0; //semaphore operation
   buf.sem_flg = 0; //semaphore flag


//child process
   if (!(pid = fork())) {

         //locks the semaphore
      buf.sem_op = -1;
      semop(semId, &buf, 1);

      for (i=0; i<loop; i++) {

         // swap the contents of shmPtr[0] and shmPtr[1]
         temp = shmPtr[0];
         shmPtr[0]= shmPtr[1];
         shmPtr[1] = temp;

         printf ("child: %li values: %li\t%li\n",i, shmPtr[0], shmPtr[1]);

      }

         //unlocks the semaphore once finished
      buf.sem_op = 1;
      semop(semId, &buf, 1);

      if (shmdt (shmPtr) < 0) {
         perror ("just can't let go\n");
         exit (1);
      }
      exit(0);
   }

   //parent
   else{
      //locks the semaphore
      buf.sem_op = -1;
      semop(semId, &buf, 1);

      for (i=0; i<loop; i++) {

               // swap the contents of shmPtr[1] and shmPtr[0]
         temp = shmPtr[1];
         shmPtr[1] = shmPtr[0];
         shmPtr[0] = temp;

         //for testing? maybe keep to show the prof
         printf ("parent: %li values: %li\t%li\n",i, shmPtr[0], shmPtr[1]);

      }
      
      //unlocks the semaphore once done
      buf.sem_op = 1;
      semop(semId, &buf, 1);

      wait (&status);
      printf ("values: %li\t%li\n", shmPtr[0], shmPtr[1]);
   }


   //delete the semaphore
   semctl(semId, 0, IPC_RMID);

   if (shmdt (shmPtr) < 0) {
      perror ("just can't let go\n");
      exit (1);
   }
   if (shmctl (shmId, IPC_RMID, 0) < 0) {
      perror ("can't deallocate\n");
      exit(1);
   }


   return 0;
}
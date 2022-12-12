#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
#include <assert.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <string.h>
#include <time.h>
#include "types.h"

struct sembuf ops[] = {{0,-1,0},{0,+1,0}};
int main (int argc, char *argv[]){
	commande_t commande;
  reponse_t  reponse;
  int fil_mess,shmid,*carte;
  int sop,sid_att;
  key_t kfile,ksem_att;
  pid_t pid;
  kfile = ftok(FICHIER_FIL,1); // file mess
  assert(kfile != -1);

  key_t cle = ftok(FICHIER_SEG,1); //file seg
  assert(cle != -1);

  ksem_att =ftok(FICHIER_SEM_ATTENTE,1);
  assert(ksem_att!=-1);
    
  fil_mess = msgget(kfile,IPC_CREAT|0600);
  assert(fil_mess != -1);

  sid_att=semget(ksem_att,0,0);

  shmid=shmget(cle, sizeof(int),0);
  assert(shmid >= 0);

  carte = (int*)shmat(shmid,NULL,0);
  assert(carte != (void*)-1);

  int nb_spec = carte[0]-1/*strtol((const char *) argv[1], NULL,10)*/;
  srand(time(NULL));

  pid=getpid();

  commande.expediteur=pid;
  commande.numero=rand() % (nb_spec+1);
  commande.type=3;
    while(1){
		msgsnd(fil_mess,&commande,sizeof(commande_t)-sizeof(long),0);
    printf("Le client %d commande la spécialité n°%d\n\n",pid,commande.numero);

		msgrcv(fil_mess,&reponse,sizeof(reponse_t)-sizeof(long),pid,0);

		sop=semop(sid_att,ops,1);
		assert(sop==0);

		printf("Le client %ld recoit sa commande la n°%d la paie et s'en va\n\n",reponse.type,reponse.numero_commande);
		
		sop=semop(sid_att,ops+1,1);
		assert(sop==0);
    }
}
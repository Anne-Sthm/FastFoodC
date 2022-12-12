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

int file_mess;
int shmid;
int sid, sid_att/*file_att*/;
void arret(int s){
  /* Arret des IPC */
  assert(msgctl(file_mess,IPC_RMID,NULL) != 1);
  assert(shmctl(shmid,IPC_RMID,0) >=0);
  assert(semctl(sid,0,IPC_RMID)!=-1);
  assert(semctl(sid_att,0,IPC_RMID)!=-1);
  //assert(semctl(file_att,0,IPC_RMID)!=-1);
  kill(0,SIGINT);
  exit(1);

}
void usage(const char *s){
  fprintf(stderr,"Usage : %s nb_serveurs nb_cuisiniers nb_term nb_spec nb_1 nb_2 ... nb_k \n",s);
  exit(-1);
}
int set_signal_handler(int signo, void (*handler)(int)) {
  struct sigaction sa;
  sa.sa_handler = handler;    // call `handler` on signal
  sigemptyset(&sa.sa_mask);   // don't block other signals in handler
  sa.sa_flags = 0 ;            //  don't restart system calls
  return sigaction(signo, &sa, NULL);
}
int main(int argc, char const *argv[])
{
  char buf[256];
  //char buf_serv[256];
	key_t kseg;
  key_t kfile;
  key_t ksem;
  key_t ksem_att/*ksem_file*/;
  int nb_serveurs, nb_cuisiniers, nb_term, nb_spec;
  //int nb_clients= 10;
  int* m;
  int i;

	if(argc <=6)
    usage(argv[0]);

  int nb_ust=argc-5;
  nb_spec = (int)strtol(argv[4],NULL,0); 
  nb_term = (int)strtol(argv[3],NULL,0);
  nb_serveurs = (int)strtol(argv[1],NULL,0);
  nb_cuisiniers= (int)strtol(argv[2],NULL,0);
  //snprintf(buf_serv,sizeof(buf),"%d",nb_serveurs);
  if (nb_term>nb_spec)
  {	
    if(nb_term<=0){
      printf("Il faut au moin un terminal de paiement\n");
      exit(0);
    } 
  	printf("Le nombre de terminaux ne peux pas être plus grand que le nombre de serveurs\n");
  	return 0;
  }
  int carte[nb_spec][nb_ust];
  m =(int *)carte; // transformation en tableau unidimentionnel

  kseg = ftok(FICHIER_SEG,1); // cle du segment partagée
  assert(kseg != -1);

  kfile = ftok(FICHIER_FIL,1); // file mess
  assert(kfile != -1);

  ksem= ftok(FICHIER_SEM,1); // cle du semaphore des ustensilles
  assert(ksem!=-1);

  ksem_att = ftok(FICHIER_SEM_ATTENTE,1); // cle du semaphore des terminaux de paiement
  assert(ksem_att!=-1);

  /*ksem_file = ftok(FICHIER_FIL_ATT,1); // cle du semaphore de la file d'attente des serveurs
  assert(ksem_file!=-1);*/

  file_mess = msgget(kfile,IPC_CREAT|0600); // création de la file de message
  assert(file_mess != -1);
  		
  shmid=shmget(kseg, sizeof(int), IPC_CREAT|0666); // création du segment partagé
  assert(shmid >= 0);

  sid = semget(ksem,nb_ust,IPC_CREAT|0666); // création de sémaphore des ustensilles
  assert(sid >= 0);

  sid_att = semget(ksem_att,1,IPC_CREAT|0666); //création de sémaphore des terminaux de paiement
  assert(sid_att >= 0);

  /*file_att = semget(ksem_file,nb_serveurs,IPC_CREAT|0666); //création de sémaphore des terminaux de paiement
  assert(file_att >= 0);*/

  semctl(sid_att,0,SETVAL,nb_term); // initialisation de sémaphore des terminaux de paiement 

  m = (int*)shmat(shmid,NULL,0);
  assert(m != (void*)-1);

  srand( time( NULL ) );

  printf("Menu du restaurant\n");
  for(int i =0;i<nb_spec;i++){ //affichage de la carte
		printf("\tspecialite(%d) : ",i);
  	for (int j = 0; j < nb_ust; j++){
      semctl(sid,j,SETVAL,(int)strtol(argv[5+j],NULL,0));//initialisation du sémaphore des ustensilles
  		carte[i][j]=rand() % ((int)strtol(argv[5+j],NULL,0)+1);
		  printf(" ust(%d)=%d",j,carte[i][j]);
  	}
		printf("\n");	
  }
  printf("\n");
  m[0]=nb_spec;
  m[1]=nb_ust;
  for(int i=0;i<nb_spec;i++){ // initialisation de la mémoire partagée 
    for(int j=0;j<nb_ust;j++){
    	m[j+nb_ust*i+2]=carte[i][j];
      //printf("m[%d] = %d\n",j+nb_ust*i+2,m[j+nb_ust*i+2]);
    }
  }
  for (i=1;i<=nb_serveurs;i++){ //lancement des serveurs
    pid_t p = fork();
    assert( p != -1);

    if (p==0) {
      //semctl(file_att,i,SETVAL,0);
      snprintf(buf,sizeof(buf),"%d",i);
      execl("./serveur","./serveur",buf,NULL);
      assert(0);
    }
  }

  for (i=1;i<=nb_cuisiniers;i++){ //lancement des cuisiniers
    pid_t p = fork();
    assert( p != -1);

    if (p==0) {
      snprintf(buf,sizeof(buf),"%d",i);
      execl("./cuisinier","./cuisinier",buf,NULL);
      assert(0);
    }
  }

  while(1){ //création des clients
    pid_t p = fork();
    assert( p != -1);

    if (p==0) {
      execl("./clients","./clients",NULL);
      assert(0);
    }
    sleep(rand()%3);
    assert(set_signal_handler(SIGINT,arret) == 0); //arrêt des ipc
  } 
}
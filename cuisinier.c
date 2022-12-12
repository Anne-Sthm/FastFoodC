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

struct sembuf *positif;
struct sembuf *negatif;
int main(int argc, char const *argv[])
{
	commande_t commande;
	reponse_t  reponse;

	int fil_mess, shmid, sid, sop;
	int numOrdre;
	int nb_spec, nb_ust;
	int *carte;

	key_t kfile = ftok(FICHIER_FIL,1); // file mess
  	assert(kfile != -1);

  	key_t cle = ftok(FICHIER_SEG,1); //seg
  	assert(cle != -1);

  	key_t ksem= ftok(FICHIER_SEM,1); //sem
 	assert(ksem!=-1);

	fil_mess = msgget(kfile,0);
	assert(fil_mess!=-1);

	shmid=shmget(cle, sizeof(int),0);
  	assert(shmid >= 0);

  	carte = (int*)shmat(shmid,NULL,0);
  	assert(carte != (void*)-1);

  	sid = semget(ksem,0,0);
  	assert(sid >= 0);

	numOrdre = (int)strtol(argv[1],NULL,0);
	nb_spec = carte[0];
	nb_ust = carte[1];


	positif= malloc(nb_spec * nb_ust * sizeof(struct sembuf));
	negatif= malloc(nb_spec * nb_ust * sizeof(struct sembuf));

	for (int i = 0; i < nb_spec; i++){
		for(int j =0; j< nb_ust;j++){
			struct sembuf tempP = {j,0+carte[j+ i*nb_ust+2],0};
			positif[j+i*nb_ust] = tempP;
			//printf("valeur= %d\n",j+ i*nb_ust+2);
			struct sembuf tempN = {j,0-carte[j+ i*nb_ust+2],0};
			negatif[j+i*nb_ust] = tempN;
			//printf("{j = %d, carte = %d, 0}\n", j, 0-carte[j+ i*nb_ust+2]);

		}
	}
  	srand( time( NULL ) );
	while(1){
		msgrcv(fil_mess,&commande,sizeof(commande_t)-sizeof(long),1,0);
		printf("Le cuisinier %d a recu une commande du seveur %d\n\n",numOrdre,commande.numero_serveur);
		/*printf("sémaphore avant kouisine: ");
		for(int j =0; j <nb_ust;j++){
            printf("ust(%d)= %d ", j, semctl(sid,j,GETVAL,0));
		}
		printf("\n");*/
		for(int i = 0; i < nb_ust; i++){
			if(carte[i+commande.numero*nb_ust+2]!=0){
				sop = semop(sid,negatif+(i+commande.numero* nb_ust),1);
           		assert(sop==0);
           		/*printf("Cuisinier %d reserve: ", numOrdre);
				for(int j =0; j <nb_ust;j++){
            		printf("ust(%d)= %d ", j, semctl(sid,j,GETVAL,0));
				}
				printf("\n");*/
           	}
		}

		sleep(rand()%3);
		printf("Le cuisinier %d réserve des ustensiles de cuisine et prépare la commande du serveur %d\n\n",numOrdre,commande.numero_serveur);
		reponse.numero_commande = commande.numero;
		reponse.type= commande.serveur_pid;
		reponse.numero_cuisinier = numOrdre;

		msgsnd(fil_mess,&reponse,sizeof(reponse_t)-sizeof(long),0);
		printf("Le cuisinier %d a terminer de cuisiné et envoie la commande au serveur %d\n\n",numOrdre,commande.numero_serveur);
		for(int i = 0; i < nb_ust; i++){
			if(carte[i+commande.numero*nb_ust+2]!=0){
				sop = semop(sid,positif+(i+commande.numero*nb_ust),1);
				assert(sop==0);
			}
		}
		/*printf("Sémahpore après kouisine: ");
		for(int j =0; j <nb_ust;j++){
            printf("ust(%d)= %d ", j, semctl(sid,j,GETVAL,0));
		}
		printf("\n");*/
	}
	return 0;
}
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

//struct sembuf *positif;
//struct sembuf *negatif;

int main(int argc, char const *argv[])
{
    commande_t commande;
    reponse_t  reponse;

    key_t kseg = ftok(FICHIER_SEG,1);
    assert(kseg!=-1);
    int fil_mess/*file_att,sop*/;
    pid_t pid;

    //segment partagé

    int shmid, *carte;
    key_t kfile/*ksem_file*/;

    kfile = ftok(FICHIER_FIL,1); // file mess
    assert(kfile != -1);
    
    /*ksem_file = ftok(FICHIER_FIL_ATT,1); // cle du semaphore de la file d'attente des serveurs
    assert(ksem_file!=-1)*/;

    fil_mess = msgget(kfile,IPC_CREAT|0600);
    assert(fil_mess != -1);

    shmid=shmget(kseg, sizeof(int),0); // récupération du segment partagé
    assert(shmid >= 0);

    carte = (int*)shmat(shmid,NULL,0); // on fait pointer notre variable vers la valeur du segment partagé
    assert(carte != (void*)-1);

    /*file_att = semget(ksem_file,0,0); //récupération de la file d'attente
    assert(file_att >= 0);*/

    int nbOrdre = strtol((const char *) argv[1], NULL,10);
    //int nb_serv = strtol((const char *) argv[2], NULL,10);

    //positif= malloc(nb_serv * sizeof(struct sembuf));
    //negatif= malloc(nb_serv * sizeof(struct sembuf));

    /*for(int j =1; j<= nb_serv;j++){
            struct sembuf tempP = {j,1,0};
            positif[j] = tempP;
            struct sembuf tempN = {j,-1,0};
            negatif[j] = tempN;
    }*/
    pid=getpid();
    while(1){
        //serveur attend la commande d'un client
        msgrcv(fil_mess,&commande,sizeof(commande_t)-sizeof(long),3,0);
        /*for(int i=1;i<=nb_serv;i++){
            sop = semop(file_att,positif+(i+nbOrdre* nb_serv),1);
            assert(sop==0);
        }*/
        printf("Le serveur %d prend la commande du client %d\n\n",nbOrdre,commande.expediteur);
        commande.type=1;
        commande.numero_serveur=nbOrdre;
        commande.serveur_pid=pid;
        //serveur envoie la commande au cuisinier
        msgsnd(fil_mess,&commande,sizeof(commande_t)-sizeof(long),0);
        printf("Le serveur %d envoie la commande du client %d aux cuisiniers\n\n",nbOrdre,commande.expediteur);
        //serveur attend que le cuisinier renvoi un réponse 
        msgrcv(fil_mess,&reponse,sizeof(reponse_t)-sizeof(long),pid,0);
        printf("Le serveur %d recoit la commande du client %d de la part du cuisinier %d \n\n",nbOrdre,commande.expediteur,reponse.numero_cuisinier);
        //serveur envoie au client commmande prete
        reponse.type=commande.expediteur;
        msgsnd(fil_mess,&reponse,sizeof(reponse_t)-sizeof(long),0);
        /*for(int i=1;i<=nb_serv;i++){
            sop = semop(file_att,negatif+(i+nbOrdre* nb_serv),1);
            assert(sop==0);
        }*/
        printf("Le serveur %d fait passer le client %d au terminal de paiement\n\n",nbOrdre,commande.expediteur);
    }
}
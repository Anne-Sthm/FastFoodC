typedef struct 
{
	long type;
	pid_t expediteur, serveur_pid;
	int numero,numero_serveur;
} 
commande_t;

typedef struct 
{
	long type;
	int numero_commande,numero_cuisinier;
} 
reponse_t;

#define FICHIER_FIL "fil.serv"
#define FICHIER_SEG "seg.serv"
#define FICHIER_SEM "sem.serv"
#define FICHIER_SEM_ATTENTE "sem_attente.serv"
#define FICHIER_FIL_ATT "file_att.serv"



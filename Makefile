CC = gcc -Wall 

all : serveur clients cuisinier fast_food

clients : clients.c types.h
	$(CC) clients.c -o clients
cuisinier : cuisinier.c types.h
	$(CC) cuisinier.c -o cuisinier
serveur : serveur.c types.h
	$(CC) serveur.c -o serveur
	touch fil.serv sem.serv seg.serv sem_attente.serv file_att.serv
fast_food : fast_food.c types.h
	$(CC) fast_food.c -o fast_food
clean :
	rm -f serveur clients fil.serv sem.serv seg.serv sem_attente.serv file_att.serv cuisinier fast_food
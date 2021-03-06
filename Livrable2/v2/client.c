#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "./sendTCP.h"
#include <semaphore.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <signal.h>

int dSG;


void exitt(int n){
    char *fin="fin";
    sendTCP(dSG,fin,sizeof(fin),0);
    int clo = close(dSG);
    if(clo==-1){
		perror("Erreur fermeture socket: ");
		exit(1);
	}
    exit(0);
}
void* envoi(void *data){
    int *dS=data; //on recupère le descripteur de socket
    while(1){ 

        char saisie1[100];
        memset(saisie1,0,sizeof(saisie1));
        //printf("\nsaisir le message: ");
        fgets(saisie1,100,stdin);

        int result = sendTCP(*dS,saisie1,sizeof(saisie1),0); //la valeur de retour est traitée dans sendTCP
        saisie1[strlen(saisie1)-1] = '\0'; //permet d'appliquer strcmp après fgets
        if(strcmp(saisie1,"fin")==0){
            printf("Arret du client\n");
            exit(0);
        }
    }
    pthread_exit(NULL);

}

void* recevoir(void * data){
    int *dS=data;
    while(1){

        char rep[100];
  
        int rec = recv(*dS,&rep,sizeof(rep),0);
        if(rec==-1){
            perror("Error recv : ");
            exit(1);
        }
        if(strcmp(rep,"fin")==0){ //utile pour les arrets des serveur
            printf("Arret du serveur\n");
            exit(0);
        }
        fputs(rep,stdout);
    }
    pthread_exit(NULL);
}

void envoiPseudo(char* pseudo, int dS){


    int result = sendTCP(dS,pseudo,sizeof(pseudo),0); //la valeur de retour est traitée dans sendTCP

}



int main(int argc, char *argv[]){


    int dS= socket(AF_INET, SOCK_STREAM, 0);
    if(dS==-1){
        perror("Erreur ! Socket non créee");
        exit(1);
    }

    struct sockaddr_in adServ ;
    adServ.sin_family = AF_INET ; 
    adServ.sin_port = htons(atoi(argv[2]));
    int res = inet_pton(AF_INET,argv[1], &(adServ.sin_addr));
    if(res == -1){
        perror("Erreur ! Famille d'adresse non valide");
        exit(1);
    }
    else if(res == 0){
        perror("Erreur ! Adresse réseau non valide");
        exit(1);
    }

    socklen_t lgA = sizeof(struct sockaddr_in);
    res = connect(dS, (struct sockaddr *) &adServ, lgA); //connexion de la socket decrite par dS à l'adresse donnée (adresse de longeur lgA)
    if(res==-1){
        perror("connexion échouée");
        exit(1);
    }
    else{
        printf("Connexion effectuée !\n");
    }

    pthread_t envo;
    pthread_t recpt;
    dSG=dS;
    signal(SIGINT,exitt);

    printf("Saisir votre pseudo: ");
    char pseud[100];
    fgets(pseud,100,stdin);
    envoiPseudo(pseud,dS);

    pthread_create(&envo, NULL,envoi, (void*)&dS); 
    pthread_create(&recpt, NULL,recevoir, (void*)&dS);
    pthread_join(envo, NULL); 
    pthread_join(recpt, NULL); 
    int clo = close(dS);
    if(clo==-1){
		perror("error with closing client : ");
		exit(1);
	}
}
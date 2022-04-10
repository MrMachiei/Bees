#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#define MAX 1000

static struct sembuf buf;
int *miodek;
int *ile_wojownikow;
int *vp;
int *misie;
int dostep_do_miodku, magazyn, dostep_do_woj, dostep_do_vp, dostep_do_misie;

void podnies(int semid, int semnum){
	buf.sem_num = semnum;
	buf.sem_op = 1;
	buf.sem_flg = 0;
	if(semop(semid, &buf, 1) == -1){
		perror("Blad podnoszenia!");
		exit(1);
	}
}

void opusc(int semid, int semnum){
	buf.sem_num = semnum;
	buf.sem_op = -1;
	buf.sem_flg = 0;
	if(semop(semid, &buf, 1) == -1){
		perror("Blad opuszczenia!");
		exit(1);
	}
}

void zjedz(int ile){
	for(int i = 0; i < ile; i++){
		int czy_zjadla = 0;
		while(czy_zjadla == 0){
			opusc(magazyn, 0);
			opusc(dostep_do_miodku, 0);
			if(*miodek >= 1){
				*miodek -= 1;
				czy_zjadla = 1;
			}
			podnies(dostep_do_miodku, 0);
			podnies(magazyn,0);
		}
	}
}

void dodaj(int ile){
	for(int i = 0; i < ile; i++){
		opusc(magazyn, 0);
		opusc(dostep_do_miodku, 0);
		*miodek += 1;
		podnies(dostep_do_miodku, 0);
		podnies(magazyn,0);
	}
}

void robotnica(){
	opusc(dostep_do_miodku, 0);
	if(*miodek >= 7){
		*miodek -=7;
		podnies(dostep_do_miodku, 0);
	}
	else{
		podnies(dostep_do_miodku, 0);
		//KOMUNIKAT
		return;
	}
	sleep(3);
	int licznik = 0;
	while(1){
		sleep(2);
		licznik += 2;
		if(licznik == 14){
			zjedz(1);
			licznik = 0;
		}
		//printf("Najedzona!");
		sleep(1);
		licznik += 1;
		dodaj(2);
	}
}

void wojownik(){
	opusc(dostep_do_miodku, 0);
	if(*miodek >= 10){
		*miodek -=10;
		podnies(dostep_do_miodku, 0);
	}else{
		return;
	}
	sleep(5);
	opusc(dostep_do_woj,0);
	*ile_wojownikow += 1;
	podnies(dostep_do_woj, 0);
	while(1){
		sleep(14);
		zjedz(1);
	}
}

void krolowa(){
	opusc(dostep_do_miodku, 0);
	if(*miodek >= 500){
		*miodek -=500;
		podnies(dostep_do_miodku, 0);
	}
	else{
		podnies(dostep_do_miodku, 0);
		//KOMUNIKAT
		return;
	}
	sleep(9);
	srand(getpid());
	int ran = rand() % 4;
	if (ran == 0 || ran == 1){
		opusc(dostep_do_vp,0);
		*vp += 1;
		podnies(dostep_do_vp, 0);
	}else if(ran == 2){
		opusc(dostep_do_woj, 0);
		*ile_wojownikow += 1;
		podnies(dostep_do_woj, 0);
		while(1){
			sleep(14);
			zjedz(1);
		}
	}else{
		int licznik = 0;
		while(1){
			sleep(2);
			licznik += 2;
			if(licznik == 14){
				zjedz(1);
				licznik = 0;
			}
			sleep(1);
			dodaj(2);
		}
	}
}
void wypisz_menu(){
	printf("A - start gry\nB - produkcja robotnicy\nC - produkcja wojownika\nD - produkcja krolowej\nE - koniec gry\n");
}
void wypisz(){
	printf("\e[1;1H\e[2J");
	printf("Miod: %d\n", *miodek);
	printf("Misie: %d\n", *misie);
	printf("Wojwnicy: %d\n", *ile_wojownikow);
	printf("Punkty zwyciestwa: %d\n\n", *vp);
}
void mis(){
	srand(getpid());
	int glod = 10;
	int szansa_na_nast = 5;
	if(fork()==0){
		while(1){
			sleep(10);
			int ran = rand() % 100 + 1;
			if(ran <= glod){//atak
				glod = 10;
				opusc(dostep_do_woj, 0);
				if(*ile_wojownikow >= 5){
					podnies(dostep_do_woj,0);
					continue;
				}else if(*ile_wojownikow == 3){
					*ile_wojownikow -= 3;
					podnies(dostep_do_woj, 0);
					opusc(dostep_do_miodku, 0);
					if(*miodek >= 20) *miodek -= 20;
					else *miodek = 0;
					podnies(dostep_do_miodku,0);
				}else{
					podnies(dostep_do_woj, 0);
					opusc(dostep_do_miodku, 0);
					if(*miodek >= 50) *miodek -= 50;
					else *miodek = 0;
					podnies(dostep_do_miodku,0);
				}
			}else{
				glod += 10;
			}
		}
	}else{
		while(1){
			sleep(20);
			int ran = rand() % 100 + 1;
			if(ran <= szansa_na_nast){//atak
				szansa_na_nast = 5;
				opusc(dostep_do_misie,0);
				*misie += 1;
				podnies(dostep_do_misie,0);
				if(fork() == 0) mis();

			}else{
				szansa_na_nast += 5;
			}
		}
	}
}
void usun_woj(){

}

int main(int argc, char* argv[]){
	srand(time(NULL));
	//tworzenie semafora dostepu do miodku
	dostep_do_miodku = semget(1, 1, IPC_CREAT|0600);
	dostep_do_vp = semget(3, 1, IPC_CREAT|0600);
	dostep_do_woj = semget(4, 1, IPC_CREAT|0600);
	dostep_do_misie = semget(5, 1, IPC_CREAT|0600);
	if(dostep_do_miodku == -1){
		perror("Blad tworzenia semafora");
		exit(1);
	}
	if(semctl(dostep_do_miodku, 0,SETVAL, (int)1) == -1){
		perror("Blad ustawiania");
		exit(1);
	}
	if(dostep_do_misie== -1){
		perror("Blad tworzenia semafora");
		exit(1);
	}
	if(semctl(dostep_do_misie, 0,SETVAL, (int)1) == -1){
		perror("Blad ustawiania");
		exit(1);
	}
	if(dostep_do_vp == -1){
		perror("Blad tworzenia semafora");
		exit(1);
	}
	if(semctl(dostep_do_vp, 0,SETVAL, (int)1) == -1){
		perror("Blad ustawiania");
		exit(1);
	}
	if(dostep_do_woj == -1){
		perror("Blad tworzenia semafora");
		exit(1);
	}
	if(semctl(dostep_do_woj, 0,SETVAL, (int)1) == -1){
		perror("Blad ustawiania");
		exit(1);
	}
	//tworzenie limitu magazynu
	magazyn = semget(2, 1, IPC_CREAT|0600);
	if(magazyn == -1){
		perror("Blad tworzenia semafora");
		exit(1);
	}
	if(semctl(magazyn, 0,SETVAL, (int)100) == -1){
		perror("Blad ustawiania");
		exit(1);
	}
		
	//tworzenie miejsca na miod
	int miod_adres = shmget(114125, sizeof(int), IPC_CREAT|0600);
	if(miod_adres == -1){
		perror("Blad tworzenia pamieci");
		exit(1);
	}
	miodek = (int*)shmat(miod_adres, NULL,0);
	*miodek = 0;
	if(miodek == NULL){
		perror("Przylaczenie segmentu blad!");
		exit(1);
	}
	int mis_adres = shmget(114126, sizeof(int), IPC_CREAT|0600);
	if(mis_adres == -1){
		perror("Blad tworzenia pamieci");
		exit(1);
	}
	misie = (int*)shmat(mis_adres, NULL,0);
	*misie = 1;
	if(misie == NULL){
		perror("Przylaczenie segmentu blad!");
		exit(1);
	}
	int woj_adres = shmget(114127, sizeof(int), IPC_CREAT|0600);
	if(woj_adres == -1){
		perror("Blad tworzenia pamieci");
		exit(1);
	}
	ile_wojownikow = (int*)shmat(woj_adres, NULL,0);
	*ile_wojownikow = 0;
	if(ile_wojownikow == NULL){
		perror("Przylaczenie segmentu blad!");
		exit(1);
	}
	int vp_adres = shmget(114128, sizeof(int), IPC_CREAT|0600);
	if(vp_adres == -1){
		perror("Blad tworzenia pamieci");
		exit(1);
	}
	vp = (int*)shmat(vp_adres, NULL,0);
	*vp = 0;
	if(vp == NULL){
		perror("Przylaczenie segmentu blad!");
		exit(1);
	}
	//program
	*miodek = 7;
	if(fork()==0){
		while(1){
			wypisz();
			wypisz_menu();
			sleep(1);
		}
	}
	char opcja = 'z';
	int start = 0;
	while(1){
		opcja = getchar();
		if(opcja < 'a') opcja += 'a'-'A';
		if(opcja >= 'a' && opcja <='e'){
			switch(opcja){
				case 'a':{
					if(start == 1) break;
					start = 1;
					if(fork() == 0) mis();
					break;
				}
				case 'b':{
					if(start == 0) break;
					if(fork() == 0){
						robotnica();
						exit(0);
					}
					else break;
				}
				case 'c':{
					if(start == 0) break;
					if( fork()== 0){
						
						wojownik();
						exit(0);
					}
					else{
						break;
					}
				}
				case 'd':{
					if(start == 0) break;
					if(fork() == 0){
						krolowa();
						exit(0);
					}
					else break;
				}
				case 'e':{
					signal(SIGQUIT, SIG_IGN);
					kill(0, SIGQUIT);
					//sprzatanie
					shmctl(miod_adres, IPC_RMID, (struct shmid_ds*)0);
					shmctl(woj_adres, IPC_RMID, (struct shmid_ds*)0);
					shmctl(mis_adres, IPC_RMID, (struct shmid_ds*)0);
					shmctl(vp_adres, IPC_RMID, (struct shmid_ds*)0);
					if(semctl(dostep_do_miodku, 0,IPC_RMID, (union semun)0) == -1){
						perror("Blad ustawiania");
						exit(1);
					}
					if(semctl(magazyn, 0,IPC_RMID, (union semun)0) == -1){
						perror("Blad ustawiania");
						exit(1);
					}
					if(semctl(dostep_do_misie, 0,IPC_RMID, (union semun)0) == -1){
						perror("Blad ustawiania");
						exit(1);
					}
					if(semctl(dostep_do_woj, 0,IPC_RMID, (union semun)0) == -1){
						perror("Blad ustawiania");
						exit(1);
					}
					if(semctl(dostep_do_vp, 0,IPC_RMID, (union semun)0) == -1){
						perror("Blad ustawiania");
						exit(1);
					}
					exit(0);
					break;
				}
			}
		}
		opusc(dostep_do_vp,0);
		if(*vp == 3){
			
			signal(SIGQUIT, SIG_IGN);
			kill(0, SIGQUIT);
			printf("GRATULUJE WYGRANEJ!\n");
			break;
		}else{
			podnies(dostep_do_vp,0);
		}
	}

	//sprzatanie
	shmctl(miod_adres, IPC_RMID, (struct shmid_ds*)0);
	shmctl(woj_adres, IPC_RMID, (struct shmid_ds*)0);
	shmctl(mis_adres, IPC_RMID, (struct shmid_ds*)0);
	shmctl(vp_adres, IPC_RMID, (struct shmid_ds*)0);
	if(semctl(dostep_do_miodku, 0,IPC_RMID, (union semun)0) == -1){
		perror("Blad ustawiania");
		exit(1);
	}
	if(semctl(magazyn, 0,IPC_RMID, (union semun)0) == -1){
		perror("Blad ustawiania");
		exit(1);
	}
	if(semctl(dostep_do_misie, 0,IPC_RMID, (union semun)0) == -1){
		perror("Blad ustawiania");
		exit(1);
	}
	if(semctl(dostep_do_woj, 0,IPC_RMID, (union semun)0) == -1){
		perror("Blad ustawiania");
		exit(1);
	}
	if(semctl(dostep_do_vp, 0,IPC_RMID, (union semun)0) == -1){
		perror("Blad ustawiania");
		exit(1);
	}
	exit(0);
}
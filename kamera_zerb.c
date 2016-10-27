#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/statvfs.h>

#include "kamera_zerb.h"

int main()
{
	int sock, n, erabiltzaile, pasahitza, komando, error, x_ard, y_ard;
	struct sockaddr_in zerb_helb, bez_helb;
	socklen_t helb_tam;
	char buf[MAX_BUF], file_path[MAX_BUF];
	char posizioa[11], arg[17];
	char mugimendua[4];
	char argazki_izena[9];	
	char * sep;
	struct stat file_info;
	FILE *fp;

	

	x_ard = 90;
	y_ard = 90;
	
	// Sortu socketa.
	if((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("Errorea socketa sortzean");
		exit(1);
	}
	
	//zerbitzaria
	memset(&zerb_helb, 0, sizeof(zerb_helb));
	zerb_helb.sin_family = AF_INET;
	zerb_helb.sin_addr.s_addr = htonl(INADDR_ANY);
	zerb_helb.sin_port = htons(PORT);


	// Esleitu helbidea socketari.	
	if(bind(sock, (struct sockaddr *) &zerb_helb, sizeof(zerb_helb)) < 0)
	{
		perror("Errorea socketari helbide bat esleitzean");
		exit(1);
	}
	

	//prozesu umeak hiltzeko modu egokian. Aurrerago begiratu, konkurrentea egiterakoan
	//signal(SIGCHLD, SIG_IGN);
	
	// Zehaztu uneko egoera bezala hasierako egoera.
	egoera = ST_INIT;
	
	while(1)
	{
		helb_tam = sizeof(bez_helb);
		if((n=recvfrom(sock, buf, MAX_BUF, 0, (struct sockaddr *) &bez_helb, &helb_tam)) < 0)
		{
			perror("Errorea datuak jasotzean");
			exit(1);
		}

		//dena ondo joan bada jarraitu, bestela whiletik atera eta prozesua egoki amaitu
		if(n==0)
			continue; 

		//bezeroarekin konektatu
		if(connect(sock, (struct sockaddr *) &bez_helb, helb_tam) < 0)
		{
			perror("Errorea bezeroarekin konektatzean");
			exit(1);
		}

		do
		{
			// Aztertu jasotako komandoa ezaguna den ala ez.
			if((komando=bilatu_substring(buf,KOMANDOAK)) < 0)
			{
				ustegabekoa(sock);
				continue;
			}
		
			// Jasotako komandoaren arabera egin beharrekoa egin.
			switch(komando)
			{
				case COM_USER:
					if(egoera != ST_INIT)		// Egiaztatu esperotako egoeran jaso dela komandoa.
					{
						ustegabekoa(sock);
						continue;
					}
					buf[n-1] = 0;	// Lerro bukaera, edo "End Of Line" (EOL), ezabatzen du.
					// Baliozko erabiltzaile bat den egiaztatu.
					if((erabiltzaile = bilatu_string(buf+5, erab_zer)) < 0)
					{
							
						//if(sendto(sock, "ERROR$1", 7, 0, (struct sockaddr *) &bez_helb, helb_tam) < 0)
						printf("Ez dago erabiltzaile hori gure listan.\n");
						if(write(sock,"ERROR$1",7)<0)
						{
							perror("Errorea datuak bidaltzean\n");
							exit(1);
						}
					}
					else
					{
						
						//if(sendto(sock, "OK", 2, 0, (struct sockaddr *) &bez_helb, helb_tam) < 0)
						printf("Erabiltzaile hori gure listan dago.\n");
						if(write(sock,"OK",2)<0)
						{
							perror("Errorea datuak bidaltzean\n");
							exit(1);
						}
						egoera = ST_AUTH;
					}
					break;

				case COM_PASS:
					if(egoera != ST_AUTH)		// Egiaztatu esperotako egoeran jaso dela komandoa.
					{
						ustegabekoa(sock);
						continue;
					}
					buf[n-1] = 0;	// EOL ezabatu.
					// Pasahitza zuzena dela egiaztatu.
					//if(!strcmp(pass_zer[erabiltzaile], buf+5))
					pasahitza = bilatu_string(buf+5, pass_zer);
					if(pasahitza==erabiltzaile)
					{
						printf("Pasahitza zuzena da.\n");
						if(write(sock,"OK",2)<0)
						{
							perror("Errorea datuak bidaltzean\n");
							exit(1);
						}
						egoera = ST_MAIN;
					}
					else
					{
						printf("Pasahitza ez da zuzena.\n");
						if(write(sock,"ERROR$2",7)<0)
							{
							perror("Errorea datuak bidaltzean\n");
							exit(1);
						}
						egoera = ST_INIT;
						
					}
					break;

				case COM_POSITION:
					if(n > 9)	// Egiaztatu ez dela parametrorik jaso.
					{
						ustegabekoa(sock);
						continue;
					}
					if(egoera != ST_MAIN)	// Egiaztatu esperotako egoeran jaso dela komandoa eta ez dela parametrorik jaso.
					{
						ustegabekoa(sock);
						continue;
					}
					stringSortu(x_ard,y_ard,posizioa);
					printf("Posizioa honakoa da:.\n");
					printf("x ardatza: %d\n", x_ard);
					printf("y ardatza: %d\n", y_ard);
					if(write(sock, posizioa, 10)<0)
					{
						perror("Errorea datuak bidaltzean\n");
						exit(1);
					}
				
					break;
				case COM_RESET:
					if(n > 6)	// Egiaztatu ez dela parametrorik jaso.
					{
						ustegabekoa(sock);
						continue;
					}
					if(egoera != ST_MAIN)		// Egiaztatu esperotako egoeran jaso dela komandoa.
					{
						ustegabekoa(sock);
						continue;
					}
					buf[n-1] = 0; // EOL ezabatu.

					x_ard = 90;
					y_ard = 90;

					printf("Kamera hasierako posiziora bueltatu da. \n");
					printf("x ardatza = %d \n",x_ard);
					printf("y ardatza = %d \n",y_ard);
				
					if(write(sock,"OK$090?090", 10)<0)
					{
						perror("Errorea datuak bidaltzean\n");
						exit(1);
					}
				
					break;
				case COM_UP:
					if(egoera != ST_MAIN) // Egiaztatu esperotako egoeran jaso dela komandoa eta ez dela parametrorik jaso.
					{
						ustegabekoa(sock);
						continue;
					}
					buf[n-1] = 0; // EOL ezabatu.
					
					

					sprintf(mugimendua,"%s",buf+3);

					printf("mugimendua honakoa da: %s gradu goruntz\n", mugimendua);
					
					//int y = atoi(mugimendua);
					y_ard = y_ard + atoi(mugimendua);

					y_ard=ardatzaFrogatu(y_ard);
					
					stringSortu(x_ard,y_ard,posizioa);
					printf("Kameraren posizioa honakoa da: \n");
					printf("x ardatza = %d \n",x_ard);
					printf("y ardatza = %d \n",y_ard);
				
					if(write(sock,posizioa, 10)<0)
					{
						perror("Errorea datuak bidaltzean\n");
						exit(1);
					}
					break;
				case COM_DOWN:
					if(egoera != ST_MAIN) // Egiaztatu esperotako egoeran jaso dela komandoa eta ez dela parametrorik jaso.
					{
						ustegabekoa(sock);
						continue;
					}
					buf[n-1] = 0; // EOL ezabatu.

					
					
					sprintf(mugimendua,"%s",buf+5);
					printf("mugimendua honakoa da: %s gradu behera\n", mugimendua);
					
					//int y = atoi(mugimendua);
					y_ard = y_ard - atoi(mugimendua);
					y_ard=ardatzaFrogatu(y_ard);
					
					
					stringSortu(x_ard,y_ard,posizioa);
					printf("Kameraren posizioa honakoa da: \n");
					printf("x ardatza = %d \n",x_ard);
					printf("y ardatza = %d \n",y_ard);
				
					if(write(sock,posizioa, 10)<0)
					{
						perror("Errorea datuak bidaltzean\n");
						exit(1);
					}
					break;
				case COM_LEFT:
					if(egoera != ST_MAIN) // Egiaztatu esperotako egoeran jaso dela komandoa eta ez dela parametrorik jaso.
					{
						ustegabekoa(sock);
						continue;
					}
					buf[n-1] = 0; // EOL ezabatu.

					sprintf(mugimendua,"%s",buf+5);
					printf("mugimendua honakoa da: %s  gradu ezkerretara\n", mugimendua);
					
					//int x = atoi(mugimendua);
					x_ard = x_ard - atoi(mugimendua);
					x_ard=ardatzaFrogatu(x_ard);
					
					stringSortu(x_ard,y_ard,posizioa);
					printf("Kameraren posizioa honakoa da: \n");
					printf("x ardatza = %d \n",x_ard);
					printf("y ardatza = %d \n",y_ard);
				
					if(write(sock,posizioa, 10)<0)
					{
						perror("Errorea datuak bidaltzean\n");
						exit(1);
					}
					break;
				case COM_RIGHT:
					if(egoera != ST_MAIN) // Egiaztatu esperotako egoeran jaso dela komandoa eta ez dela parametrorik jaso.
					{
						ustegabekoa(sock);
						continue;
					}
					buf[n-1] = 0; // EOL ezabatu.

					
					sprintf(mugimendua,"%s",buf+6);
					printf("mugimendua honakoa da: %s  gradu eskubitara\n", mugimendua);
					
					//int x = atoi(mugimendua);
					x_ard = x_ard + atoi(mugimendua);
					x_ard=ardatzaFrogatu(x_ard);
					
					stringSortu(x_ard,y_ard,posizioa);
					printf("Kameraren posizioa honakoa da: \n");
					printf("x ardatza = %d \n",x_ard);
					printf("y ardatza = %d \n",y_ard);
				
					if(write(sock,posizioa, 10)<0)
					{
						perror("Errorea datuak bidaltzean\n");
						exit(1);
					}
					break;
				case COM_PHOTO:
					if(n > 6)	// Egiaztatu ez dela parametrorik jaso.
					{
						ustegabekoa(sock);
						continue;
					}
					if(egoera != ST_MAIN) // Egiaztatu esperotako egoeran jaso dela komandoa eta ez dela parametrorik jaso.
					{
						ustegabekoa(sock);
						continue;
					}
					buf[n-1] = 0;
					if(x_ard==90 || y_ard==90)
						strcpy(argazki_izena,"argazki0");
					if(x_ard<90 && y_ard<90)
						strcpy(argazki_izena,"argazki1");
					if(x_ard>90 && y_ard<90)
						strcpy(argazki_izena,"argazki2");
					if(x_ard<90 && y_ard>90)
						strcpy(argazki_izena,"argazki3");
					if(x_ard>90 && y_ard>90)
						strcpy(argazki_izena,"argazki4");
				
					// Fitxategiak dauden karpeta eta fitxategiaren izena kateatu.
					sprintf(file_path,"%s/%s",FILES_PATH,argazki_izena);
					// Lortu fitxategiari buruzko informazioa.
					if(stat(file_path, &file_info) < 0)	
					{
						ustegabekoa(sock);
						continue;
					}
					else
					{
						sprintf(arg,"OK$%s?%ld",argazki_izena,file_info.st_size);
						printf("bidali nahi dugun argumentu izena: %s \n", arg);
						if(write(sock,arg, 17)<0) // 3+9+1+4=17
						{
							perror("Errorea datuak bidaltzean\n");
							exit(1);
						}
						egoera=ST_PHOTO;
					}
					break;
				case COM_FOTOP:
					if(egoera != ST_PHOTO) // Egiaztatu esperotako egoeran jaso dela komandoa eta ez dela parametrorik jaso.
					{
						ustegabekoa(sock);
						continue;
					}
					//printf("Fotop barruan zaude\n");
					if((fp=fopen(file_path,"r")) == NULL)
					{
						ustegabekoa(sock);
					}
					else
					{
					//Hau zergatik??
					// Fitxategiaren lehenengo zatia irakurri eta errore bat gertatu bada bidali errore kodea.
					if((n=fread(buf,1,MAX_BUF,fp))<MAX_BUF && ferror(fp) != 0)
					{
						ustegabekoa(sock);
					}
					else
					{
						//write(sock,"OK",4);
						// Bidali fitxategiaren zatiak.
						do
						{
							write(sock,buf,MAX_BUF);
						} while((n=fread(buf,1,MAX_BUF,fp)) == MAX_BUF);
						
						if(ferror(fp) != 0)
						{
							close(sock);
							//hau zergatik??
							return;
						}
						else if(n>0)
							write(sock,buf,n);	// Bidali azkeneko zatia.
					}
					fclose(fp);
					}
					break;
				case COM_LOGOUT:
					if(n > 7)	// Egiaztatu ez dela parametrorik jaso.
					{
						ustegabekoa(sock);
						continue;
					}
					if((egoera==ST_INIT)||(egoera==ST_AUTH))
					{
						printf("Ez zaude logeatuta. Sartu erabiltzailea:\n");
						write(sock,"ERROR$4",7);
						egoera = ST_INIT;
					}else{
						printf("Log out-a egoki burutu da. Erabiltzailea sartu:\n");
						write(sock,"OK",2);
						egoera = ST_INIT;
					}
					break;
			}
		}while((n=read(sock, buf, MAX_BUF)) > 0);
			
		close(sock);
		if(n < 0)
		{
			perror("Errorea mezua jasotzean");
			exit(1);
		}
		exit(0);
		
	}
}



/*
* 'string' parametroko karaktere katea bilatzen du 'string_zerr' parametroan. 'string_zerr' bektoreko azkeneko elementua NULL izan behar da.
* 'string' katearen lehen agerpenaren indizea itzuliko du, edo balio negatibo bat ez bada agerpenik aurkitu.
*/

int bilatu_string(char *string, char **string_zerr)
{
	int i=0;
	while(string_zerr[i] != NULL)
	{
		if(!strcmp(string,string_zerr[i]))
			return i;
		i++;
	}
	return -1;
}

/*
* 'string' parametroa 'string_zerr' bektoreko karaktere kateetako batetik hasten den egiaztatzen du. 'string_zerr' bektoreko azkeneko elementua NULL izan behar da.
* 'string' parametro karaktere katearen hasierarekin bat egiten duen 'string_zerr' bektoreko lehenego karaktere katearen indizea itzuliko du. Ez bada bat-egiterik egon balio negatibo bat itzuliko du.
*/
int bilatu_substring(char *string, char **string_zerr)
{
	int i=0;
	while(string_zerr[i] != NULL)
	{
		if(!strncmp(string,string_zerr[i],strlen(string_zerr[i])))
			return i;
		i++;
	}
	return -1;
}

/*
* Ustekabeko zerbait gertatzen denean egin beharrekoa: dagokion errorea bidali bezeroari eta egoera eguneratu.
*/
void ustegabekoa(int s)
{
	write(s,"ERROR$8",7);
	if(egoera == ST_AUTH)
	{
		egoera = ST_INIT;
	}
	else {
		printf("Errore bat egon da\n");
		exit(1);
	}
}

void stringSortu(int x, int y, char* pos)
{
	sprintf(pos,"OK$%03d?%03d",x,y);
}

int ardatzaFrogatu(int y)
{
	if (y < 0)
		y = 0;
		
	if (y > 180)
		y = 180;	
	
	return y;
}





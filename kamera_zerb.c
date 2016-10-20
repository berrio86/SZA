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
	struct sockaddr_in zerb_helb, bez_helb, bez_helb2;
	socklen_t helb_tam;
	char buf[MAX_BUF];
	char * sep;

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
			/*case COM_POSITION:
				if(egoera != ST_MAIN)	// Egiaztatu esperotako egoeran jaso dela komandoa eta ez dela parametrorik jaso.
				{
					ustegabekoa(s);
					continue;
				}
				
				if(write(sock,"OK$", 10)<0)
				{
					perror("Errorea datuak bidaltzean\n");
					exit(1);
				}
				
				break;
			case COM_RESET:
				if(egoera != ST_MAIN)		// Egiaztatu esperotako egoeran jaso dela komandoa.
				{
					ustegabekoa(s);
					continue;
				}
				buf[n-2] = 0; // EOL ezabatu.
				sprintf(file_path,"%s/%s",FILES_PATH,buf+4);		// Fitxategiak dauden karpeta eta fitxategiaren izena kateatu.
				if(stat(file_path, &file_info) < 0)							// Lortu fitxategiari buruzko informazioa.
				{
					write(s,"ER5\r\n",5);
				}
				else
				{
					sprintf(buf,"OK%ld\r\n", file_info.st_size);
					write(s, buf, strlen(buf));
					egoera = ST_DOWN;
				}
				break;
			case COM_UP:
				if(n > 6 || egoera != ST_DOWN)		// Egiaztatu esperotako egoeran jaso dela komandoa eta ez dela parametrorik jaso.
				{
					ustegabekoa(s);
					continue;
				}
				egoera = ST_MAIN;
				// Fitxategia ireki.
				if((fp=fopen(file_path,"r")) == NULL)
				{
					write(s,"ER6\r\n",5);
				}
				else
				{
					// Fitxategiaren lehenengo zatia irakurri eta errore bat gertatu bada bidali errore kodea.
					if((n=fread(buf,1,MAX_BUF,fp))<MAX_BUF && ferror(fp) != 0)
					{
						write(s,"ER6\r\n",5);
					}
					else
					{
						write(s,"OK\r\n",4);
						// Bidali fitxategiaren zatiak.
						do
						{
							write(s,buf,n);
						} while((n=fread(buf,1,MAX_BUF,fp)) == MAX_BUF);
						if(ferror(fp) != 0)
						{
							close(s);
							return;
						}
						else if(n>0)
							write(s,buf,n);	// Bidali azkeneko zatia.
					}
					fclose(fp);
				}
				break;
			case COM_DOWN:
				if(egoera != ST_MAIN)		// Egiaztatu esperotako egoeran jaso dela komandoa.
				{
					ustegabekoa(s);
					continue;
				}
				// Erabiltzaile anonimoak ez dauka ekintza honetarako baimenik.
				if(erabiltzaile==0)
				{
					write(s,"ER7\r\n",5);
					continue;
				}
				
				buf[n-2] = 0;	// EOL kendu.
				// Mezuak dauzkan bi zatiak (fitxategi izena eta tamaina) erauzi.
				if((sep = strchr(buf,'?')) == NULL)
				{
					ustegabekoa(s);
					continue;
				}
				*sep = 0;
				sep++;
				strcpy(file_name,buf+4);	// Fitxategi izena lortu.
				file_size = atoi(sep);		// Fitxategi tamaina lortu.
				sprintf(file_path,"%s/%s",FILES_PATH,file_name);	// Fitxategiak dauden karpeta eta fitxategiaren izena kateatu.
				if(file_size > MAX_UPLOAD_SIZE)	// Fitxategi tamainak maximoa gainditzen ez duela egiaztatu.
				{
					write(s,"ER8\r\n",5);
					continue;
				}
				if(toki_librea() < file_size + SPACE_MARGIN)	// Mantendu beti toki libre minimo bat diskoan.
				{
					write(s,"ER9\r\n",5);
					continue;
				}
				write(s,"OK\r\n",4);
				egoera = ST_UP;
				break;
			case COM_LEFT:
				if(n > 6 || egoera != ST_UP)		// Egiaztatu esperotako egoeran jaso dela komandoa eta ez dela parametrorik jaso.
				{
					ustegabekoa(s);
					continue;
				}
				egoera = ST_MAIN;
				irakurrita = 0L;
				fp=fopen(file_path,"w");		// Sortu fitxategi berria diskoan.
				error = (fp == NULL);
				while(irakurrita < file_size)
				{
					// Jaso fitxategi zati bat.
					if((n=read(s,buf,MAX_BUF)) <= 0)
					{
						close(s);
						return;
					}
					// Ez bada errorerik izan gorde fitxategi zatia diskoan.
					// Errorerik gertatuz gero segi fitxategi zatiak jasotzen.
					// Fitxategi osoa jasotzeak alferrikako trafikoa sortzen du, baina aplikazio protokoloak ez du beste aukerarik ematen.
					if(!error)
					{
						if(fwrite(buf, 1, n, fp) < n)
						{
							fclose(fp);
							unlink(file_path);
							error = 1;
						}
					}
					irakurrita += n;
				}
				if(!error)
				{
					fclose(fp);
					write(s,"OK\r\n",4);
				}
				else
					write(s,"ER10\r\n",6);
				break;
			case COM_RIGHT:
				if(egoera != ST_MAIN)		// Egiaztatu esperotako egoeran jaso dela komandoa.
				{
					ustegabekoa(s);
					continue;
				}
				// Erabiltzaile anonimoak ez dauka ekintza honetarako baimenik.
				if(erabiltzaile==0)
				{
					write(s,"ER7\r\n",5);
					continue;
				}
				buf[n-2] = 0; // EOL ezabatu.
				sprintf(file_path,"%s/%s",FILES_PATH,buf+4);	// Fitxategiak dauden karpeta eta fitxategiaren izena kateatu.
				if(unlink(file_path) < 0)		// Ezabatu fitxategia.
					write(s,"ER11\r\n",6);
				else
					write(s,"OK\r\n",4);
				break;
			case COM_PHOTO:
				break;
			case COM_PHOTOP:
				break;*/
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
* Telneteko lerro jauzi estandar bat ("\r\n") aurkitu arte datuak irakurtzen ditu stream batetik.
* Erraztasunagatik lerro jauzia irakurketa bakoitzeko azken bi bytetan soilik bilatzen da.
* Dena ondo joanez gero irakurritako karaktere kopurua itzuliko du.
* Fluxua amaituz gero ezer irakurri gabe 0 itzuliko du.
* Zerbait irakurri ondoren fluxua amaituz gero ("\r\n" aurkitu gabe) -1 itzuliko du.
* 'tam' parametroan adierazitako karaktere kopurua irakurriz gero "\r\n" aurkitu gabe -2 itzuliko du.
* Beste edozein error gertatu ezkero -3 itzuliko du.
*/
int readline(int stream, char *buf, int tam)
{
	/*
		Kontuz! Inplementazio hau sinplea da, baina ez da batere eraginkorra.
	*/
	char c;
	int guztira=0;
	int cr = 0;

	while(guztira<tam)
	{
		int n = read(stream, &c, 1);
		if(n == 0)
		{
			if(guztira == 0)
				return 0;
			else
				return -1;
		}
		if(n<0)
			return -3;
		buf[guztira++]=c;
		if(cr && c=='\n')
			return guztira;
		else if(c=='\r')
			cr = 1;
		else
			cr = 0;
	}
	return -2;
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
		printf("Errore bat egon da");
		exit(1);
	}
}

/*char[10] stringSortu()
{
	
}*/




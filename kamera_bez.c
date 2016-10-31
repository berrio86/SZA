#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h>
#include <unistd.h>

#include "kamera_bez.h"

int main(int argc, char *argv[])
{
	char buf[MAX_BUF], param[MAX_BUF];
	char zerbitzaria[MAX_BUF] = SERVER;
	int portua = PORT;
	
	int sock, n, status, aukera, x_ard, y_ard;
	char *argazki;
	long fitx_tamaina;
	FILE *fp;
	struct sockaddr_in zerb_helb;
	struct hostent *hp;

	// Parametroak prozesatu.

	if(argc != 1)
	{
		fprintf(stderr, "\nErabilera: %s parametro bakarra izan behar da.\n", argv[0]);
		exit(1);
	}

	// Ongi etorria.

	fprintf(stdout, "\n\nOngi etorri segurtasun kameraren gestioaren aplikaziora.\n");
	
	// Socketa sortu.
	
	if((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("Errorea socketa sortzean.");
		exit(1);
	}

	// Zerbitzariko socketaren helbidea sortu.

	memset(&zerb_helb, 0, sizeof(zerb_helb));
	zerb_helb.sin_family = AF_INET;
	zerb_helb.sin_port = htons(portua);
	if((hp = gethostbyname(zerbitzaria)) == NULL)
	{
		herror("Errorea zerbitzariaren izena ebaztean.");
		exit(1);
	}
	memcpy(&zerb_helb.sin_addr, hp->h_addr, hp->h_length);
	
	// Konektatu zerbitzariarekin.

	if(connect(sock, (struct sockaddr *) &zerb_helb, sizeof(zerb_helb)) < 0)
	{
		perror("Errorea zerbitzariarekin konektatzean.");
		exit(1);
	}

	// Erabiltzaile eta pasahitza bidali.

	int i=0;
	do
	{
		printf("Sar ezazu erabiltzaile izena: ");
		fgets(param,MAX_BUF,stdin);
		sprintf(buf,"%s$%s",KOMANDOAK[COM_USER],param);
	
		if(write(sock, buf, strlen(buf)) < 0){
			perror("[USER] Errorea idazterakoan");
		}
		read(sock, buf, MAX_BUF);
		
		status = parse(buf);
		if(status != 0)
		{
			fprintf(stderr,"Errorea: ");
			fprintf(stderr,"%s",ER_MEZUAK[status]);
			continue;
		}

		//Erabiltzailea ondo dago.

		printf("Sar ezazu zure pasahitza: ");
		fgets(param,MAX_BUF,stdin);
		sprintf(buf,"%s$%s",KOMANDOAK[COM_PASS],param);
		
		if(write(sock, buf, strlen(buf)) < 0){
			perror("[PASS] Errorea idazterakoan");
		}
		read(sock, buf, MAX_BUF);

		status = parse(buf);
		if(status != 0)
		{
			fprintf(stderr,"Errorea: ");
			fprintf(stderr,"%s",ER_MEZUAK[status]);
			continue;
		}
		break;

		//Pasahitza zuzena.
	} while(1);

	//Logeatuta dago.
	do
		{
			aukera = menua();
			switch(aukera)
			{
				case OP_POS: //Uneko posizioa jakin.
					sprintf(buf,"%s",KOMANDOAK[COM_POSITION]);
					if(write(sock, buf, strlen(buf)) < 0){
						perror("[POS] Errorea idazterakoan");
					}
					n = read(sock, buf, MAX_BUF);
					buf[n] = 0;

					status = parse(buf);
					if(status != 0)
					{
						fprintf(stderr,"Errorea: ");
						fprintf(stderr,"%s",ER_MEZUAK[status]);
						continue;
					}
					x_ard = atoi(extract(buf, "$", "?"));
					y_ard = atoi(buf+7);

					printf("\nKamararen uneko posizioa hau da:");
					printf("\nX posizioa: %d. Y posizioa: %d.\n", x_ard, y_ard);

					break;

				case OP_RST: //Kamararen posizioa berrezarri.
				sprintf(buf, "%s", KOMANDOAK[COM_RESET]);
					if(write(sock, buf, strlen(buf)) < 0){
						perror("[RST] Errorea idazterakoan");
					}
					n = read(sock, buf, MAX_BUF);
					buf[n] = 0;

					status = parse(buf);
					if(status != 0)
					{
						fprintf(stderr,"Errorea kamerari RESET egiterakoan.\n");
						continue;
					}
					printf("Kamara ondo berrezarri egin da X: 90 eta Y: 90 puntuan.\n");

					break;

				case OP_UP: //Kamara mugitu gorantz.
					printf("\nSar ezazu mugimenduaren gradu kopurua: ");
					fgets(param,MAX_BUF,stdin);

					sprintf(buf,"%s$%03d", KOMANDOAK[COM_UP], atoi(param));

					if(write(sock, buf, strlen(buf)) < 0){
						perror("[UP] Errorea idazterakoan");
					}
					n = read(sock, buf, MAX_BUF);
					buf[n] = 0;

					x_ard = atoi(extract(buf, "$", "?"));
					y_ard = atoi(buf+7);

					printf("\nPosizioa ondo aldatu da. Posizio berria:");
					printf("\nX posizioa: %d. Y posizioa: %d.\n", x_ard, y_ard);

					break;

				case OP_DOWN: //Kamara mugitu beherantz.
					printf("\nSar ezazu mugimenduaren gradu kopurua: ");
					fgets(param,MAX_BUF,stdin);

					sprintf(buf,"%s$%03d", KOMANDOAK[COM_DOWN], atoi(param));

					if(write(sock, buf, strlen(buf)) < 0){
						perror("[DOWN] Errorea idazterakoan");
					}
					n = read(sock, buf, MAX_BUF);
					buf[n] = 0;

					x_ard = atoi(extract(buf, "$", "?"));
					y_ard = atoi(buf+7);

					printf("\nPosizioa ondo aldatu da. Posizio berria:");
					printf("\nX posizioa: %d. Y posizioa: %d.\n", x_ard, y_ard);

					break;

				case OP_LEFT: //Kamara mugitu ezkerrerantz.
					printf("\nSar ezazu mugimenduaren gradu kopurua: ");
					fgets(param,MAX_BUF,stdin);

					sprintf(buf,"%s$%03d", KOMANDOAK[COM_LEFT], atoi(param));

					if(write(sock, buf, strlen(buf)) < 0){
						perror("[LEFT] Errorea idazterakoan");
					}
					n = read(sock, buf, MAX_BUF);
					buf[n] = 0;

					x_ard = atoi(extract(buf, "$", "?"));
					y_ard = atoi(buf+7);

					printf("\nPosizioa ondo aldatu da. Posizio berria:");
					printf("\nX posizioa: %d. Y posizioa: %d.\n", x_ard, y_ard);

					break;

				case OP_RIGHT: //Kamara mugitu eskuinerantz.
					printf("\nSar ezazu mugimenduaren gradu kopurua: ");
					fgets(param,MAX_BUF,stdin);

					sprintf(buf,"%s$%03d", KOMANDOAK[COM_RIGHT], atoi(param));

					if(write(sock, buf, strlen(buf)) < 0){
						perror("[RIGHT] Errorea idazterakoan");
					}
					n = read(sock, buf, MAX_BUF);
					buf[n] = 0;

					x_ard = atoi(extract(buf, "$", "?"));
					y_ard = atoi(buf+7);

					printf("\nPosizioa ondo aldatu da. Posizio berria:");
					printf("\nX posizioa: %d. Y posizioa: %d.\n", x_ard, y_ard);

					break;

				case OP_PHOTO: //Kamarako argazkia jaso.
					sprintf(buf, "%s", KOMANDOAK[COM_PHOTO]);
					if(write(sock, buf, strlen(buf)) < 0){
						perror("[PHOTO] Errorea idazterakoan");
					}
					n = read(sock, buf, MAX_BUF);
					buf[n] = 0;

					status = parse(buf);
					if(status != 0)
					{
						fprintf(stderr,"Errorea: ");
						fprintf(stderr,"%s",ER_MEZUAK[status]);
						continue;
					}
					fitx_tamaina = atol(buf+12);
					argazki = extract(buf, "$", "?");

					//Kamararen argazki izena eta bidaliko den tamaina jaso dugu.

					sprintf(buf, "%s", KOMANDOAK[COM_FOTOP]);
					if(write(sock, buf, strlen(buf)) < 0){
						perror("[PHOTO] Errorea idazterakoan");
					}

					if(status != 0)
					{
						fprintf(stderr,"Errorea: ");
						fprintf(stderr,"%s",ER_MEZUAK[status]);
						continue;
					}

					if((fp = fopen(argazki,"w")) == NULL)		// Fitxategia sortu diskoan.
					{
						perror("Ezin da fitxategia disko lokalean gorde");
						exit(1);
					}

					n = read(sock, buf, MAX_BUF);
					buf[n] = 0;
					if(fwrite(buf, 1, n, fp) < 0)				//Fitxategian idatzi jasotakoa.
					{
						perror("Arazoa fitxategia disko lokalean gordetzean");
						exit(1);
					}
					fclose(fp);
					printf("Fitxategia ondo jaso da.\n");

					break;

				case OP_LOGOUT: //Kameratik atera.
					sprintf(buf, "%s", KOMANDOAK[COM_LOGOUT]);
					if(write(sock, buf, strlen(buf)) < 0){
						perror("[LOGOUT] Errorea idazterakoan");
					}
					n = read(sock, buf, MAX_BUF);
					buf[n] = 0;

					status = parse(buf);
					if(status != 0)
					{
						fprintf(stderr,"Errorea: ");
						fprintf(stderr,"%s",ER_MEZUAK[status]);
						continue;
					}
					printf("Eskerrik asko segurtasun kameraren gestioaren aplikazioa erabiltzeagatik.\n");
					break;

			}
		}while (aukera != 8); //LOGOUT egiterakoan, whiletik atera eta socketa itxiko da, programa amaituz.

	close(sock);
}


/* Zerbitzaritik jasotako erantzuna aztertzen du.
* Jasotakoa OK bada 0 balioa itzuliko du eta bestela errorearen kode zenbakia.
*/
int parse(char *status)
{
	if(!strncmp(status,"OK",2))
		return 0;
	else if(!strncmp(status,"ERROR",5))
		return(atoi(status+6));
	else
	{
		fprintf(stderr,"Ustekabeko erantzuna.\n");
		exit(1); 
	}
}

/* String batetik ezkerreko eta eskuineko delimitadore bi emanda
*  haien barruan dagoena bueltatzen du. Ezer ez dagoenean NULL bueltatuko du.
*/

char *extract(const char *const string, const char *const left, const char *const right)
{
    char  *head;
    char  *tail;
    size_t length;
    char  *result;

    if ((string == NULL) || (left == NULL) || (right == NULL))
        return NULL;
    length = strlen(left);
    head   = strstr(string, left);
    if (head == NULL)
        return NULL;
    head += length;
    tail  = strstr(head, right);
    if (tail == NULL)
        return tail;
    length = tail - head;
    result = malloc(1 + length);
    if (result == NULL)
        return NULL;
    result[length] = '\0';

    memcpy(result, head, length);
    return result;
}

/* Erabiltzaileari agertuko zaion menua da, kameraren aukera guztiak ikustaraziz.
*/
int menua()
{
	char katea[MAX_BUF];
	int aukera;
	
	printf("\n");
	printf("\t\t\t\t*****************************************\n");
	printf("\t\t\t\t****** Segurtasun Kameraren Panela ******\n");
	printf("\t\t\t\t*****************************************\n");
	printf("\t\t\t\t**                                     **\n");
	printf("\t\t\t\t**       1. Uneko posizioa jakin       **\n");
	printf("\t\t\t\t**       2. Posizioa berrezarri        **\n");
	printf("\t\t\t\t**       3. Gorantz mugitu             **\n");
	printf("\t\t\t\t**       4. Beherantz mugitu           **\n");
	printf("\t\t\t\t**       5. Ezkerrerantz mugitu        **\n");
	printf("\t\t\t\t**       6. Eskuinerantz mugitu        **\n");
	printf("\t\t\t\t**       7. Argazkia atera             **\n");
	printf("\t\t\t\t**       8. Saioa itxi                 **\n");
	printf("\t\t\t\t**                                     **\n");
	printf("\t\t\t\t*****************************************\n");
	printf("\t\t\t\t*****************************************\n");
	
	printf("\t\t\t\tEgin zure aukera: ");
	while(1)
	{
		fgets(katea,MAX_BUF,stdin);
		aukera = atoi(katea);
		if(aukera > 0 && aukera < 9)
			break;
		printf("\t\t\t\tAukera okerra, saiatu berriro: ");
	}
	printf("\n");
	return aukera;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_BUF 1024
#define PORT 50007

int main()
{
	int sock, n, buk;
	struct sockaddr_in zerb_helb, bez_helb;
	socklen_t helb_tam;
	char buf[MAX_BUF];
	struct ip_mreq multicast;

	if((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("Errorea socketa sortzean");
		exit(1);
	}

	/*
		IKASLEAK BETETZEKO:
		Erantzun igorleari jasotako kodea gehi ikaslearen datuekin eta jaso erantzuna.
		Erantzuna positiboa da amaitu, bestela saiatu berriro.
	*/
	
	memset(&zerb_helb, 0, sizeof(zerb_helb));
	zerb_helb.sin_family = AF_INET;
	zerb_helb.sin_addr.s_addr = htonl(INADDR_ANY);
	zerb_helb.sin_port = htons(PORT);
	
	if(bind(sock, (struct sockaddr *) &zerb_helb, sizeof(zerb_helb)) < 0)
	{
		perror("Errorea socketari helbide bat esleitzean");
		exit(1);
	}

	/*
		IKASLEAK BETETZEKO:
		Esleitu socketari dagokion multicast helbidea.
	*/
	multicast.imr_interface.s_addr=htonl(INADDR_ANY);
	inet_aton("224.0.0.11", &multicast.imr_multiaddr.s_addr);
	if(setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &multicast, sizeof(multicast))<0){
		perror("Errorea socketa multicast moduan esleitzean");
		exit(1);
	}
	// Kasu honetan begizta ez da infinitua izango.
	buk=0;
	while(buk==0)
	{
		helb_tam = sizeof(bez_helb);
		if((n=recvfrom(sock, buf, MAX_BUF, 0, (struct sockaddr *) &bez_helb, &helb_tam)) < 0)
		{
			perror("Errorea datuak jasotzean");
			exit(1);
		}
		buf[n]=0;
		sprintf(buf,"%sInaki Berriotxoa Gabiria",buf);

		if(sendto(sock, buf, strlen(buf), 0, (struct sockaddr *) &bez_helb, helb_tam) < 0)
		{
			perror("Errorea datuak bidaltzean");
			exit(1);
		}
		if((n=recvfrom(sock, buf, MAX_BUF, 0, (struct sockaddr *) &bez_helb, &helb_tam)) < 0)
		{
			perror("Errorea datuak jasotzean");
			exit(1);
		}
		buf[2]=0;
		if(strcmp(buf,"OK")==0){
			buk=1;
			printf("Dena ederki joan da.\n");
		}
		else{
			printf("Zeozer ez da ondo joan.\n");
		}
	}
	/*
		IKASLEAK BETETZEKO:
		Askatu multicast helbidea.
	*/
	setsockopt(sock, IPPROTO_IP, IP_DROP_MEMBERSHIP, &multicast, sizeof(multicast));
	close(sock);
}

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "udp.h"

using namespace std;
Network *udp;


int main()
{
	udp = new Network("192.168.0.100", 5555, "192.168.0.111", 8082); //Network(string ipDest, unsigned short int portDest, string ipMe, unsigned short int portMe);
	char udpMsg[100];
	int count=0;
	usleep(1000000);

	while(1)
	{
		sprintf(udpMsg, "%f,%f,%d,%f,%f,%f", 223.244, 223.244, count, 223.244, 223.244, 223.244); 
		udp->sendData(udpMsg); // sned message

		if(udp->copyData()) //returns 1 if data is recieved, 0 if data not recieved for 1 sec , 2 if mutex is locked (data not copied)
		printf("%s \n", udp->uData->c_str()); //acess recieved data

		cout<<count<<"\n";
		count++;
		usleep(10000);
	}

	delete udp;
	return 0;
}

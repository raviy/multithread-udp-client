#include <iostream>
#include <thread>
#include <mutex>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

using namespace std;

class Network
{
	public:
	Network(string ipDest, unsigned short int portDest, string ipMe, unsigned short int portMe);
	~Network();
	int sendData(string message);
	bool copyData();
	string *uData;

	private:
	char data[100];
	bool rcvFlag;
	int s;
	char *buf;
	timeval tv;
	mutex dataLock;
	thread *daqThread;
	string *tempData;
	void recieveData();
	struct sockaddr_in addr, addrMe, addrDest;
};

Network::Network(string ipDest, unsigned short int portDest, string ipMe, unsigned short int portMe)
{
	this->addrDest.sin_family = AF_INET;
	int ret = inet_aton(ipDest.c_str(), &this->addrDest.sin_addr);
	if (ret == 0) { perror("inet_aton"); exit(1); }
	this->addrDest.sin_port = htons(portDest);	

	this->addrMe.sin_family = AF_INET;
	ret = inet_aton(ipMe.c_str(), &this->addrMe.sin_addr);
	if (ret == 0) { perror("inet_aton"); exit(1); }
	this->addrMe.sin_port = htons(portMe);

	this->s = socket(AF_INET, SOCK_DGRAM, 0);
	if (this->s == -1) { perror("socket"); exit(1); }

	ret = bind(this->s, (struct sockaddr*) &this->addrMe, sizeof(this->addrMe));
	if (ret == -1) { perror("bind"); exit(1);}

	tv.tv_sec  = 1; // 1 second timer
	tv.tv_usec = 0;

	setsockopt(this->s, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<char*>(&this->tv), sizeof(timeval));

	this->daqThread = new thread(&Network::recieveData, this); //start new thread
	
	this->tempData = new string("No data available");
}

Network::~Network()
{
	close(this->s);
	this->daqThread->detach();
	this->rcvFlag = false;
}

int Network::sendData(string message)
{
	this->buf = strdup(message.c_str());
	int ret = sendto(this->s, this->buf, strlen(this->buf), 0, (struct sockaddr *)&this->addrDest, sizeof(this->addrDest));
	if (ret == -1) { perror("sendto"); return 1; }
}

void Network::recieveData()
{
	while(1)
	{
		unsigned int len = sizeof(this->addr);
		
		int blen = recvfrom(this->s, this->data, sizeof(this->data), 0, (struct sockaddr*) &this->addr, &len);
		if (blen == -1) perror("recvfrom");
	
		this->dataLock.try_lock();
		if (blen == 0 || blen == -1)
		{
			this->rcvFlag = false;
			//this->tempData = new string("Recieve Timeout");
			this->dataLock.unlock();
		}
		else
		{
			this->rcvFlag = true;
			this->tempData = new string(this->data);
			this->dataLock.unlock();
		}
		usleep(10000);
	}
}

bool Network::copyData()
{
	if(this->dataLock.try_lock())
	{
		uData = this->tempData;
		bool y = rcvFlag;
		this->dataLock.unlock();
		return y;
	}

	return 2;
}

#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h> 
#include <netinet/in.h>

bool decision(){
    int a=1;
    }

int main()
{

		signal(SIGINT, signalHandler);

		int SocketClient = socket(AF_INET, SOCK_STREAM, 0);
		if (SocketClient == -1)
		{
			std::cerr << "[CLIENT]Eroare la socket\n";
				return errno;
		}

		sockaddr_in server;
		server.sin_family = AF_INET;
		server.sin_addr.s_addr = inet_addr("127.0.0.1");
		server.sin_port = htons(2024);

		int test;
		if ((test=connect(SocketClient, (struct sockaddr*)&server, sizeof(server))) == -1)
		{ 
			std::cerr << "[CLIENT]Eroare la conectare\n";
			return errno;
		}

		bool exit = false;
		char buffer[512];

		while (!exit)
		{
            decision();
        }
}

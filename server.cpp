#include <iostream>
#include <string>
#include <thread>
#include <errno.h>
#include <mutex>
#include <fstream>
#include <bits/stdc++.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sqlite3.h>
#include <regex>
#include <csignal>
using namespace std;
bool server_quit = false;
struct sockaddr_in server; // structura folosita de server
struct sockaddr_in from;
struct login{
    char username[200];
    char password[200];
};
enum Protocol
{
	_Exit1,
	_LogIn,
	_LogOut,
	_Register,
    _CreateGame,
    _JoinGame
};
enum LOGIN
{
    _Connectat,
    _Eroare,
    _Sloturi
};
enum REGISTER
{
_Username,
_Allok
};

struct loginn{
    char username[20];
    char password[20];
};
void signalHandler(int signum)
{
	printf("[server]Server closed..\n");
	server_quit = true;
	exit(signum);
}; //TODO: vezi despre ce e vb (semnal)


bool logInPlayer(int fd){

    printf("\nI am in logIn\n");
	char username[200], password[200];
	int len;

    FILE *log;
    log=fopen("login12.txt","r");
    struct loginn l;
    int semnal;
    read(fd,&semnal,sizeof(semnal));
    read(fd,&username,semnal);
    read(fd,&semnal,sizeof(semnal));
    read(fd,&password,semnal);
    
    int raspuns=LOGIN::_Sloturi;
    
    while(fread(&l,sizeof(l),1,log))
{
    if(strcmp(username,l.username)==0&&strcmp(password,l.password)==0)
{
    raspuns=LOGIN::_Connectat;
    printf("Succesful login\n");
}
    else{
    raspuns=LOGIN::_Eraore;
    printf("Please Enter correct username and password");
}
}
 write(fd,raspuns,sizeof(raspuns));
 fclose(log);   
};

bool registerPlayer(int fd){
    printf("\nI am in register\n");
    int length;
    char username[200],password[200];
    FILE *log;
    log=fopen("login12.txt","r+w");
    struct loginn l;
    
    read(fd,&length,sizeof(length));
    read(fd,&l.username,length);
    read(fd,&length,sizeof(length));
    read(fd,&l.password,length);

int raspuns=LOGIN::_Connectat;;
printf("got answers\n");
    while(fread(&l,sizeof(l),1,log))
    {
        if(strcmp(username,l.username)==0)
        {       

            printf("erroare\n");
            raspuns = REGISTER::_Username; 
            write(fd,&raspuns,sizeof(raspuns));
            fclose(log);
            return false;
        }
    }
    printf("a mers\n");
    fwrite(&l,sizeof(l),1,log);
    raspuns = LOGIN::_Eroare;
    write(fd,&raspuns,sizeof(raspuns));
    fclose(log);
    return true;
     
};

bool createGame(int fd){
int a=1;
};

bool joinGame(int fd){
int a=1;
};

bool playGame(int fd1, int fd2){
int a=1;
};

bool Servire(int fd, sockaddr_in c_socket)
{
	char ipAdr[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &c_socket.sin_addr, ipAdr, INET_ADDRSTRLEN);
	printf("Clinetul cu IP-ul %s s-a conectat la file-descriptor-ul %d.\n", ipAdr, fd);
	int rd;
	bool quits = false;
	while (!quits && !server_quit)
	{
		fflush(stdin);
		int desc;
		rd = read(fd, &desc, sizeof(desc));
		if (!rd)
		{
			printf("Client disconnected\n");
			quits = true;
		}
		printf("%d\n", desc);
		if (desc == Protocol::_LogIn)
			logInPlayer(fd);
		else if (desc == Protocol::_Register)
				registerPlayer(fd);
			else if (desc == Protocol::_CreateGame)
					createGame(fd);
				else if (desc == Protocol::_JoinGame)
						joinGame(fd);
					else if (desc == Protocol::_Exit1)
							;
	
	}
	return true;
};


//..............................................................................................
int main()
{

	char msg[100];			 //mesajul primit de la client
	char msgrasp[100] = " "; //mesaj de raspuns pentru client

	signal(SIGINT, signalHandler);
	std::cout << "[server]Starting server.\n";

	// pregatirea structurilor de date
	bzero(&server, sizeof(server));
	bzero(&from, sizeof(from));

	//socket
	int server_socket = socket(AF_INET, SOCK_STREAM, 0); //descriptor socket
	if (server_socket == -1)
	{
		cerr << "[server]Eroare la crearea socket-ului!\n";
		return -2;
	}
	std::cout << "[server]Created the socket.\n";

	int enable = 1;
	setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &enable, sizeof(int));

	sockaddr_in servera;
	memset(&servera, 0, sizeof(servera));
	
	servera.sin_family = AF_INET;
	servera.sin_addr.s_addr = htonl(INADDR_ANY);
	servera.sin_port = htons(2024);

	if (bind(server_socket, (sockaddr *)(&servera), sizeof(servera)) == -1)
	{
		perror("[server]Eroare la bind!\n");
		return errno;
	}
	std::cout << "[server]Sucessful bind.\n";

	//asteptam clienti
	if (listen(server_socket, 6) == -1)
	{
		cerr << "[server]Eroare la listen!\n";
		return -2;
	}
	std::cout << "[server]Listening to clients...\n";
	//accept
	sockaddr_in client_socket;
	memset(&client_socket, 0, sizeof(client_socket));
	while (true)
	{
		int client;
		socklen_t lenght = sizeof(from);

		fflush(stdout);

		client = accept(server_socket, (sockaddr *)(&client_socket), &lenght);
		if (client < 0)
		{
			cerr << "[server]Eroare la acceptarea clientului!\n";
			return -2;
		}
        std::thread client_thread(Servire,client,client_socket);
        client_thread.join();
	}

	//close
		printf("[server]Stopping...\n");
		return (0);
}

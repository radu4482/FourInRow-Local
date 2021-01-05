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
std::mutex m;
enum Protocol
{
	_Exit1,
	_LogIn,
	_LogOut,
	_Register,
	_CreateGame,
	_JoinGame
};

struct utilizator
{
	int fd = -1;
	char username[20];

	bool setLink(int link)
	{
		fd = link;
	}

	bool setName(char *name)
	{
		strcpy(username, name);
		username[strlen(username) + 1] = '\0';
		return true;
	};

	int getLink()
	{
		return fd;
	}
	char *getName()
	{
		return username;
	}
};

bool setUser(int fd, utilizator &Utilizator)
{
	char username[20];
	int len;

	//citim numele ales de utilizator
	read(fd, &len, sizeof(len));
	read(fd, &username, len);
	username[len] = 0;
	//il inscriem cu numele si adresa sa
	Utilizator.setName(username);
	Utilizator.setLink(fd);

	printf("[User:%s]Done setting up the User\n", Utilizator.getName());
	return true;
};

struct game
{
	utilizator Player1;
	utilizator Player2;
	int playerTurn = 0;

	bool setPlayers(utilizator Utilizator1, utilizator Utilizator2)
	{
		Player1 = Utilizator1;
		Player2 = Utilizator2;
		return true;
	};

	bool nextTurn()
	{
		playerTurn = 1 - playerTurn;
		return true;
	};

	int getPlayerTurn()
	{
		return playerTurn;
	}

	utilizator getPlayer(int turn)
	{
		if (turn)
			return Player2;
		return Player1;
	}

	bool isYourTurn(int Player)
	{
		if (playerTurn == Player - 1)
			return true;
		return false;
	}
};

struct gameLobby
{
	game Game;
	utilizator Utilizator1;
	utilizator Utilizator2;
	int playerNumber = 1;
	char hostName[20];
	int Player1Turn=0;
	int Player2Turn=0;


	bool setPlayer1Turn(int turn){
		Player1Turn=turn;
		Player2Turn=1-turn;
	}

	bool empty = true;

	void setGame()
	{
		Game.setPlayers(Utilizator1, Utilizator2);
	};

	bool enoughPlayers()
	{
		if (playerNumber == 2)
			return true;
		return false;
	};

	bool emptyLobby()
	{
		return empty;
	}
	bool startLobby(utilizator &Utilizator)
	{
		Utilizator1 = Utilizator;
		strcpy(hostName, Utilizator1.getName());
		empty = false;
	};

	utilizator getUtilizator1()
	{
		return Utilizator1;
	}

	utilizator getUtilizator2()
	{
		return Utilizator2;
	}

	bool updatePlayer2(utilizator &Utilizator)
	{
		Utilizator2 = Utilizator;
		playerNumber = 2;
		empty = false;
		Game.setPlayers(getUtilizator1(), getUtilizator2());
		return true;
	};

	int getPlayersNumber()
	{
		return playerNumber;
	}
};

gameLobby Lobbys[6];
void gameTestConnection(utilizator Player, int fdEnemy, int PlayerIndex)
{
	int turn = 0;
	int aux;
	for (int i = 0; i < 2; i++)
	{
		if (turn == PlayerIndex)
		{
			read(Player.getLink(), &aux, sizeof(aux));
			write(fdEnemy, &aux, sizeof(aux));
		}
		else
		{
			read(Player.getLink(), &aux, sizeof(aux));
		}
		turn = 1 - turn;
	}
};

void gamePlayerTurn(utilizator Player, int fdEnemy, int PlayerIndex)
{
	printf("[In Game]");
	int len;
	char message[100];
	int a;
	int turn = 0;
	//gameTestConnection(Player, fdEnemy, PlayerIndex);
	while (1)
	{
		if (turn == PlayerIndex)
		{
			read(Player.getLink(), &len, sizeof(len));
			if(len==100)break;
			//read(Player.getLink(), &message, len);
			printf("[Game][Player:%s][Message]%d\n", Player.getName(), len);
			write(fdEnemy, &len, sizeof(len));
			//write(fdEnemy, &message, len);
		}
		else
		{
			read(Player.getLink(), &a, sizeof(a));
			if(a==100)break;
			printf("[Player: %s]A primit mesajul;\n", Player.getName());
		}
		turn = 1 - turn;
	}

}

bool addInList(gameLobby &Lobby)
{
	for (int i = 0; i < 6; i++)
		if (Lobbys[i].emptyLobby())
		{
			Lobbys[i] = Lobby;
			return true;
		}
	return false;
}

void signalHandler(int signum)
{
	printf("[Server]Server closed..\n");
	server_quit = true;
	exit(signum);
}; //TODO: vezi despre ce e vb (semnal)

void createGame(utilizator &Utilizator)
{
	printf("[User:%s]A ales sa creeze joc\n", Utilizator.getName());
	gameLobby lobby;
	int turn;
	read(Utilizator.getLink(),&turn,sizeof(turn));
	lobby.startLobby(Utilizator);
	addInList(lobby);
	int fdEnemy;

	read(Utilizator.getLink(), &fdEnemy, sizeof(fdEnemy));
	printf("[User:%s] Lobby full , Game will start \n", Utilizator.getName());

	lobby.setGame();
	gamePlayerTurn(Utilizator, fdEnemy, lobby.Player1Turn);
};

bool joinGame(utilizator &Utilizator)
{
	printf("[User:%s]A ales sa join game\n", Utilizator.getName());
	char name[20];
	int len;
	int aux = 0;
	int decision;
	for (int i = 0; i < 6; i++)
		if (!Lobbys[i].emptyLobby())
			aux++;
	write(Utilizator.getLink(), &aux, sizeof(aux));
	if (aux == 0)
	{
		printf("[User:%s][Join]No lobbys\n", Utilizator.getName());
		return false;
	}
	for (int i = 0; i < 6; i++)
		if (!Lobbys[i].emptyLobby())
		{
			strcpy(name, Lobbys[i].getUtilizator1().getName());
			len = strlen(name);
			write(Utilizator.getLink(), &i, sizeof(i));
			write(Utilizator.getLink(), &len, sizeof(len));
			write(Utilizator.getLink(), &name, len);
			printf("[Game nr.%d] By %s\n", i, name);
		}
	read(Utilizator.getLink(), &decision, sizeof(decision));

	printf("[User:%s] Decision is : %d\n", Utilizator.getName(), decision);

	Lobbys[decision].updatePlayer2(Utilizator);

	int a = Utilizator.getLink();
	write(Lobbys[decision].getUtilizator1().getLink(), &a, sizeof(a));
	a=Lobbys[decision].Player2Turn;
	write(Utilizator.getLink(),&a,sizeof(a));
	printf("[User:%s] [GameStart] \n", Utilizator.getName());

	gamePlayerTurn(Utilizator, Lobbys[decision].getUtilizator1().getLink(), Lobbys[decision].Player2Turn);
	return true;
};

bool Servire(int fd, sockaddr_in c_socket)
{
	char ipAdr[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &c_socket.sin_addr, ipAdr, INET_ADDRSTRLEN);
	printf("Clinetul cu IP-ul %s s-a conectat la file-descriptor-ul %d.\n", ipAdr, fd);
	int rd;
	bool quits = false;

	fflush(stdin);
	char desc[20];
	int len;

	utilizator Utilizator;
	setUser(fd, Utilizator);

	while (!quits && !server_quit)
	{
		read(fd, &len, sizeof(len));
		read(fd, &desc, len);

		if (strcmp(desc, "join") == 0)
		{
			joinGame(Utilizator);
		}
		else if (strcmp(desc, "createGame") == 0)
		{
			createGame(Utilizator);
		}
	}
	return true;
};

//..............................................................................................
int main()
{

	char msg[100];			 //mesajul primit de la client
	char msgrasp[100] = " "; //mesaj de raspuns pentru client

	signal(SIGINT, signalHandler);
	std::cout << "[server]Starting server.\n"
			  << flush;

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
	std::cout << "[server]Created the socket.\n"
			  << flush;

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
	std::cout << "[server]Sucessful bind.\n"
			  << flush;

	//asteptam clienti
	if (listen(server_socket, 6) == -1)
	{
		cerr << "[server]Eroare la listen!\n";
		return -2;
	}
	std::cout << "[server]Listening to clients...\n"
			  << flush;
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
		std::thread client_thread(Servire, client, client_socket);
		client_thread.detach();
	}

	//close
	printf("[server]Stopping...\n");
	return (0);
}

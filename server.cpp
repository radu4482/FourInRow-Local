#include <iostream>
#include <string>
#include <thread>
#include <errno.h>
#include <mutex>
#include <fstream>
#include <bits/stdc++.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sqlite3.h>
#include <regex>
#include <csignal>
#include <cstdlib>
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

void signalHandler(int signum)
{
	printf("[Server]Server closed..\n");
	server_quit = true;
	exit(signum);
};

struct utilizator
{
	int exit = 0;
	int id;
	int fd = -1;
	char username[20];

	bool setExit(int value)
	{
		exit = value;
	}
	int getExit()
	{
		return exit;
	}
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
	int getId()
	{
		return id;
	}
	bool setId(int idUnique)
	{
		id = idUnique;
	}
};

utilizator Utilizatori[6];

bool setUniqueId(utilizator &Utilizator)
{
	int id = rand() % 100 + 1;
	bool aux;
	do
	{
		aux = false;
		for (int i = 0; i < 6; i++)
			if (Utilizatori[i].getId() == id)
				aux = true;
	} while (aux);
	Utilizator.setId(id);
}
bool addUserInList(utilizator Utilizator)
{
	for (int i = 0; i < 6; i++)
		if (Utilizatori[i].getLink() == -1)
		{
			Utilizatori[i] = Utilizator;
			return true;
		}
	return false;
};

bool isThisUsername(char name[20])
{
	for (int i = 0; i < 6; i++)
		if (strcmp(name, Utilizatori[i].getName()) == 0)
		{
			return true;
		}
	return false;
};

bool setUser(int fd, utilizator &Utilizator)
{
	char username[20];
	int len;
	int usedname;
	//citim numele ales de utilizator
	do
	{
		read(fd, &len, sizeof(len));
		read(fd, &username, len);
		username[len] = 0;
		if (isThisUsername(username))
			usedname = 1;
		else
			usedname = 0;
		write(fd, &usedname, sizeof(usedname));

	} while (isThisUsername(username));
	//il inscriem cu numele si adresa sa
	Utilizator.setName(username);
	Utilizator.setLink(fd);
	setUniqueId(Utilizator);
	addUserInList(Utilizator);
	printf("[User:%s]Done setting up the User with Ip:%d\n", Utilizator.getName(), Utilizator.getLink());
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
	int id = rand() % 1000;
	game Game;
	utilizator Utilizator1;
	utilizator Utilizator2;
	int playerNumber = 1;
	char hostName[20];
	int Player1Turn = 0;
	int Player2Turn = 0;

	int getId()
	{
		return id;
	}

	bool setPlayer1Turn(int turn)
	{
		Player1Turn = turn;
		Player2Turn = 1 - turn;
		return true;
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

void cleanLobby(gameLobby Lobby)
{
	for (int i = 0; i < 6; i++)
		if (Lobbys[i].getId() == Lobby.getId())
		{
			gameLobby newLobby;
			Lobbys[i] = newLobby;
			break;
		}
}

bool addLobbyInList(gameLobby &Lobby)
{
	for (int i = 0; i < 6; i++)
		if (Lobbys[i].emptyLobby())
		{
			Lobbys[i] = Lobby;
			return true;
		}
	return false;
}

struct gamePlay
{
	int matrix[8][8];
	int matrix_size = 8;

	bool setupMatrix(int size)
	{
		for (int i = 0; i < size; i++)
			for (int j = 0; j < size; j++)
				matrix[i][j] = 0;
	};

	//Urmatoarele 3 functii sunt pentru verificarea si aplicarea deciziei jucatorului
	bool columnFull(int columnIndex)
	{

		if (matrix[0][columnIndex] != 0)
			return true;
		return false;
	};

	int firstEmptyRow(int columnIndex)
	{

		for (int i = matrix_size - 1; i >= 0; i--)
			if (matrix[i][columnIndex] == 0)
				return i;
	};

	bool addInColumn(int columnIndex, int value)
	{

		if (columnFull(columnIndex))
			return false;

		int row = firstEmptyRow(columnIndex);
		matrix[row][columnIndex] = value;
		return true;
	};

	//Urmatoarele functii sunt pentru verificarea jocului
	bool latitude(int row, int column)
	{
		int i = column - 1, j = column + 1;
		int color = matrix[row][column];
		int sum = 1;
		while (i >= 0 && matrix[row][i] == color)
		{
			sum++;
			i--;
		}
		while (j < matrix_size && matrix[row][j] == color)
		{
			sum++;
			j++;
		}
		if (sum >= 4)
		{
			return true;
		}
		return false;
	};

	bool longitude(int row, int column)
	{
		int i = row - 1, j = row + 1;
		int color = matrix[row][column];
		int sum = 1;
		while (i >= 0 && matrix[i][column] == color)
		{
			sum++;
			i--;
		}
		while (j < matrix_size && matrix[j][column] == color)
		{
			sum++;
			j++;
		}
		if (sum >= 4)
		{
			return true;
		}
		return false;
	};

	bool diagonal(int row, int column)
	{

		int color = matrix[row][column];

		int sum1 = 1;
		int i1 = -1, j1 = 1;
		while (row + i1 >= 0 && column + i1 >= 0 && matrix[row + i1][column + i1] == color)
		{
			i1--;
			sum1++;
		}
		while (row + j1 < matrix_size && column + j1 < matrix_size && matrix[row + j1][column + j1] == color)
		{
			j1++;
			sum1++;
		}
		if (sum1 >= 4)
		{
			return true;
		}

		int sum2 = 1;
		int i2 = -1, j2 = 1;
		while (row + i2 >= 0 && column - i2 < matrix_size && matrix[row + i2][column - i2] == color)
		{
			i2--;
			sum2++;
		}
		while (row + j2 < matrix_size && column - j2 >= 0 && matrix[row + j2][column - j2] == color)
		{
			j2++;
			sum2++;
		}
		if (sum2 >= 4)
		{
			return true;
		}

		return false;
	};

	bool fourInRow(int row, int column)
	{
		if (matrix[row][column] == 0)
			return false;
		if (diagonal(row, column) == true)
		{
			return true;
		}
		if (latitude(row, column) == true)
		{
			return true;
		}
		if (longitude(row, column) == true)
		{
			return true;
		}
		return false;
	};

	int endGame()
	{
		for (int i = 0; i < matrix_size; i++)
			for (int j = 0; j < matrix_size; j++)
				if (fourInRow(i, j))
					return matrix[i][j];
		return 0;
	};

	void printMatrix()
	{
		for (int i = 0; i < matrix_size; i++)
		{
			for (int j = 0; j < matrix_size; j++)
				printf("%d ", matrix[i][j]);
			printf("\n");
		}
		printf("\n0 1 2 3 4 5 6 7\n");
	};

	void printMatrix2(int fd)
	{
		for (int i = 0; i < 8; i++)
			for (int j = 0; j < 8; j++)
				write(fd, &matrix[i][j], sizeof(matrix[i][j]));
	};
};

void gamePlayerTurn2(utilizator Player, int fdEnemy, int PlayerIndex, int myAmountOfWins, int enemyAmountOfWins)
{
	gamePlay Game;
	Game.setupMatrix(8);
	printf("[In Game]");
	int decizie;
	int aux;
	int raspuns;
	int playerFlag = 2;
	int enemyFlag = 3;
	int turn = 0;
	write(Player.getLink(), &myAmountOfWins, sizeof(myAmountOfWins));
	write(Player.getLink(), &enemyAmountOfWins, sizeof(enemyAmountOfWins));
	//gameTestConnection(Player, fdEnemy, PlayerIndex);
	do
	{
		write(Player.getLink(), &turn, sizeof(turn));
		Game.printMatrix2(Player.getLink());
		if (turn == PlayerIndex)
		{
			do
			{
				read(Player.getLink(), &decizie, sizeof(decizie));
				if (decizie == 100)
					break;
				if (Game.columnFull(decizie))
				{
					raspuns = 0;
					write(Player.getLink(), &raspuns, sizeof(raspuns));
				}
				else
					raspuns = 1;
			} while (raspuns == 0);

			Game.addInColumn(decizie, playerFlag);
			aux = Game.endGame();
			if (aux != 0)
				raspuns = aux;
			write(fdEnemy, &decizie, sizeof(decizie));
			write(Player.getLink(), &raspuns, sizeof(raspuns));
		printf("Raspuns %d\n",raspuns);
		}
		else
		{
			read(Player.getLink(), &decizie, sizeof(decizie));
			Game.addInColumn(decizie, enemyFlag);
			raspuns = 1;
			aux = Game.endGame();
			if (aux != 0)
				raspuns = aux;
			write(Player.getLink(), &raspuns, sizeof(raspuns));
		printf("Raspuns %d\n",raspuns);
		}
		turn = 1 - turn;
		printf("Raspuns %d\n",raspuns);
	}while (raspuns != 2 && raspuns != 3);

	printf("OutOfWhile\n");
	int theWin;
	if(raspuns==2)theWin=0;
	else theWin=1;

	int playAgain;
	read(Player.getLink(), &playAgain, sizeof(playAgain));
	
     printf("[Player:%s]The playAgain:%d\n",Player.getName(),playAgain);
	write(fdEnemy, &playAgain, sizeof(playAgain));
	
     printf("[Player:%s]The playAgain:%d\n",Player.getName(),playAgain);
	read(Player.getLink(), &playAgain, sizeof(playAgain));
	
    printf("[Player:%s]The playAgain:%d\n",Player.getName(),playAgain);

	if (playAgain==1)
		gamePlayerTurn2(Player, fdEnemy, PlayerIndex, myAmountOfWins + theWin, enemyAmountOfWins + (1 - theWin));

}

void gamePlayerTurn(utilizator Player, int fdEnemy, int PlayerIndex, int myAmountOfWins, int enemyAmountOfWins)
{
	printf("[In Game]");
	int len;
	int a;
	int turn = 0;
	write(Player.getLink(), &myAmountOfWins, sizeof(myAmountOfWins));
	write(Player.getLink(), &enemyAmountOfWins, sizeof(enemyAmountOfWins));
	//gameTestConnection(Player, fdEnemy, PlayerIndex);
	while (1)
	{
		if (turn == PlayerIndex)
		{
			read(Player.getLink(), &len, sizeof(len));
			if (len == 100)
				break;
			//read(Player.getLink(), &message, len);
			printf("[Game][Player:%s][Message]%d\n", Player.getName(), len);
			write(fdEnemy, &len, sizeof(len));
			//write(fdEnemy, &message, len);
		}
		else
		{
			read(Player.getLink(), &a, sizeof(a));
			if (a == 100)
				break;
		}
		turn = 1 - turn;
	}
	int theWin;
	read(Player.getLink(), &theWin, sizeof(theWin));
	int decizie;
	read(Player.getLink(), &decizie, sizeof(decizie));
	write(fdEnemy, &decizie, sizeof(decizie));
	read(Player.getLink(), &decizie, sizeof(decizie));

	if (decizie == 1)
		gamePlayerTurn(Player, fdEnemy, PlayerIndex, myAmountOfWins + theWin, enemyAmountOfWins + (1 - theWin));
}

void createGame(utilizator &Utilizator)
{
	printf("[User:%s]A ales sa creeze joc\n", Utilizator.getName());
	gameLobby lobby;

	int turn;
	read(Utilizator.getLink(), &turn, sizeof(turn));
	lobby.startLobby(Utilizator);
	lobby.setPlayer1Turn(turn);
	addLobbyInList(lobby);

	int fdEnemy;
	read(Utilizator.getLink(), &fdEnemy, sizeof(fdEnemy));
	printf("fd:%d\n", Utilizator.getLink());
	printf("fd:%d\n", fdEnemy);
	printf("[User:%s] Lobby full , Game will start \n", Utilizator.getName());

	lobby.setGame();
	turn = lobby.Player1Turn;
	cleanLobby(lobby);
	gamePlayerTurn2(Utilizator, fdEnemy, turn, 0, 0);
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
	a = Lobbys[decision].Player2Turn;
	write(Utilizator.getLink(), &a, sizeof(a));
	printf("[User:%s] [GameStart] \n", Utilizator.getName());

	gamePlayerTurn2(Utilizator, Lobbys[decision].getUtilizator1().getLink(), Lobbys[decision].Player2Turn, 0, 0);
	return true;
};
bool quit(utilizator Utilizator)
{
	for (int i = 0; i < 6; i++)
		if (Utilizatori[i].getId() == Utilizator.getId())
		{

			Utilizator.setExit(1);
			printf("[Player %s]Just Quit |  P1.index:%d  P1:%d  |  P2:%d  \n",
				   Utilizator.getName(), i, Utilizatori[i].getId(), Utilizator.getId());
			utilizator emptyUtilizator;
			Utilizatori[i] = emptyUtilizator;
			return true;
		}
}

bool Servire(int fd, sockaddr_in c_socket)
{
	char ipAdr[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &c_socket.sin_addr, ipAdr, INET_ADDRSTRLEN);
	printf("Clinetul cu IP-ul %s s-a conectat la file-descriptor-ul %d.\n", ipAdr, fd);
	int rd;
	bool quits = false;

	fflush(stdin);
	int desc;

	utilizator Utilizator;

	setUser(fd, Utilizator);
	while (!quits && !server_quit && Utilizator.getExit() == 0)
	{
		read(fd, &desc, sizeof(desc));

		if (desc == 1)
		{
			joinGame(Utilizator);
		}
		else if (desc == 0)
		{
			createGame(Utilizator);
		}
		else if (desc == 2)
		{
			quit(Utilizator);
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

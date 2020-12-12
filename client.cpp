#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h> 
#include <netinet/in.h>
#include <iostream>
#include <signal.h>
#include <errno.h>
#include <arpa/inet.h>
#include <cstring>
#include <csignal>

bool myExit=false;
using namespace std;
int SocketClient;

struct utilizator
{
    char username[256];
    char parola[256];
    bool logat=false;
    
    bool este_logat()
    {
        return logat;
    };

    bool logout()
    {
        logat=false;
        return true;
    };
    bool login()
    {
        logat=true;
        return true;
    }

    bool SetParola(char *password){
    strcpy(parola,password);
    return true;
    };

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


const char *const comenzi[]={
    "Exit","exit",
    "Help","help",
    "LogIn","Login","logIn","login",
    "LogOut","Logout","logOut","logout",
    "Register","register",
    "CreateGame","cretegame","createGame","Creategame",
    "joingame","JoinGame","joinGame","Joingame"
};

bool isAComand(const char *comanda){
    for(auto com:comenzi)
    {
        if(strcmp(comanda,com)==0)
        {
             return true;
        }
    }
    return false;
}

bool LogIn(int fd, utilizator Utilizator)
{
    char nume_utilizator[256],parola[256];
   
    do{
    std::cout<<"Usernameul dumneavoastra:";
    std::cin>>nume_utilizator;
    }while(nume_utilizator==NULL);

    do{
    std::cout<<"Parola dumneavoastra:";
    std::cin>>parola;
    }while(parola==NULL);

    //Trimitem datele
    int length=strlen(nume_utilizator);
    write(fd, &length, sizeof(length));
    write(fd, &nume_utilizator, length);
    
    length=strlen(parola);
    write(fd, &length, sizeof(length));
    write(fd, &parola, length);
    
    int semnal;
    read(fd,&semnal, sizeof(semnal));
    
    if(semnal==LOGIN::_Connectat){
    Utilizator.login();
    std::cout<<"Ati fost connectat";
    Utilizator.login();
    return true;    
    }
    if(semnal==LOGIN::_Eroare)
    {
    std::cout<<"Username sau parola incorecta";
    return false;    
    }
    if(semnal==LOGIN::_Sloturi)
    {
    std::cout<<"Serverul este plin";
    return false;
    }

}

bool Register(int fd,utilizator Utilizator){
    char username[20]="\0";
    char password[20]="\0";
        
    printf("Enter username: ");std::cin>>username;
    printf("Enter parola: ");std::cin>>password;
    

int length = strlen(username); 
    write(fd,&length,sizeof(length));
    write(fd,&username,length);
   
    length =strlen(password);
    write(fd,&length,sizeof(length));
    write(fd,&password,length);

    int semnal;
    read(fd,&semnal,sizeof(semnal));
    
    if(semnal==REGISTER::_Username){
    printf("Usernameul a fost folosit deja");
    return false;    
    }
    if(semnal==REGISTER::_Allok){
    printf("Everything went good");
    return true;
    }
    

}

    bool audentify(int fd, utilizator &Utilizator){
        char nullSpace[20];
        char decizie[20];
        int myDecision;
        while(!Utilizator.este_logat()){
             printf("\nPuteti alege din :\n");
             printf("Login,\nRegister\n\n");
             printf("Voi alege :");    std::cin.getline(decizie,20,'\n');

             if(strcmp(decizie,"Login")==0||strcmp(decizie,"login")==0)
                    {   
                        myDecision=Protocol::_LogIn;
                        write(fd,&myDecision,sizeof(myDecision));
                        LogIn(fd,Utilizator);
                    }
             if(strcmp(decizie,"Register")==0||strcmp(decizie,"register")==0)
                   {
                        myDecision=Protocol::_Register;
                        write(fd,&myDecision,sizeof(myDecision));
                        Register(fd,Utilizator);
                   }     
            }
    }

    bool decision(int fd, utilizator Utilizator)
    {
        if(!Utilizator.este_logat())
            audentify(fd,Utilizator);
     return true;
    }

	void signalHandler(int signum)
	{
		exit(signum);
	}

	bool Exit(int fd)
	{
		int semnal = Protocol::_Exit1;
		write(fd, &semnal, sizeof(semnal));
		std::cout << "\n Conexiune pierduta \n";
		return true;
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
        server.sin_family=AF_INET;
        server.sin_port=htons(2024);
        server.sin_addr.s_addr=inet_addr("127.0.0.1");

		int test;
		if ((test=connect(SocketClient, (struct sockaddr*)&server, sizeof(server))) == -1)
		{ 
			std::cerr << "[CLIENT]Eroare la conectare\n";
			return errno;
		}

		char buffer[512];
        utilizator User;
		while (!myExit)
		{
            decision(SocketClient,User);
        }
}

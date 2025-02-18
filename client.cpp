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

bool myExit = false;
using namespace std;
int SocketClient;

struct game
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
        while (i >= 0 && matrix[row][i] == color){
            sum++;
            i--;
        }
        while (j < matrix_size && matrix[row][j] == color){
            sum++;
            j++;
        }
        if (sum >= 4)
            return true;
        return false;
    };

    bool longitude(int row, int column)
    {
        int i = row - 1, j = row + 1;
        int color = matrix[row][column];
        int sum = 1;
        while (i >= 0 && matrix[i][column] == color){
            sum++;
            i--;
        }
        while (j < matrix_size && matrix[j][column] == color){
            sum++;
            j++;
        }
        if (sum >= 4)
            return true;
        return false;
    };

    bool diagonal(int row, int column){   
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
            return true;

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
            return true;

        return false;
    };

    bool fourInRow(int row, int column)
    {
        if (matrix[row][column] == 0) return false;
        if (diagonal(row, column) == true) return true;
        if (latitude(row, column) == true) return true;
        if (longitude(row, column) == true) return true;
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
        for (int i = 0; i < matrix_size; i++){
            for (int j = 0; j < matrix_size; j++)
                printf("%d ", matrix[i][j]);
            printf("\n");
            }
        printf("\n0 1 2 3 4 5 6 7\n");
    };
};

struct utilizator
{
    bool loged = false;
    char username[20];
    int fd;

    bool setUser(int link)
    {
        fd = link;
    };

    bool setName(char name[20])
    {
        strcpy(username, name);
        return true;
    };

    bool setUp(int fd, char name[20])
    {
        this->seLogheaza();
        this->setName(name);
        this->setUser(fd);
        return true;
    };

    int getLink()
    {
        return fd;
    };
    char getName()
    {
        return *username;
    };
    bool esteLogat()
    {
        return loged;
    };

    bool seLogheaza()
    {
        loged = true;
        return true;
    };
};

void readAfisareMatrix(int fd)
{
    int value;
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            read(fd, &value, sizeof(value));
            printf("%d ", value);
        }
        printf("\n");
    }
    printf("\n0 1 2 3 4 5 6 7\n");
}

void gamePlayerTurn2(int fd,int PlayerIndex){
    int turn;
    int decizie;
    int raspuns;
    int myWins;
    int enemyWins;
    read(fd, &myWins, sizeof(myWins));
    read(fd, &enemyWins, sizeof(enemyWins));
    do{
        read(fd,&turn,sizeof(turn));
        printf("Score is %d You | %d Enemy\n\n",myWins,enemyWins);
        readAfisareMatrix(fd);
        if(turn==PlayerIndex)
        {
            do{
                printf("Choose a column: ");
                cin>>decizie;
                write(fd,&decizie,sizeof(decizie));
                read(fd,&raspuns,sizeof(raspuns));
            }while(raspuns==0);
        }
        else{
            read(fd,&decizie,sizeof(decizie));
            write(fd,&decizie,sizeof(decizie));
            read(fd,&raspuns,sizeof(raspuns));
        }
    }while(raspuns!=2&&raspuns!=3);

    printf("GameOver\n");
    printf("Would you like to play again with your enemy ?\n0 for NO, 1 for YES\n");
 
    int playAgain;
    int playAgainEnemy;
    do{
        cin >> playAgain;
    } while (playAgain != 0 && playAgain != 1);

    write(fd, &playAgain, sizeof(playAgain));
    read(fd, &playAgainEnemy, sizeof(playAgainEnemy));

    if (playAgain == 1 && playAgainEnemy == 1)
    {
        write(fd, &playAgain, sizeof(playAgain));
        gamePlayerTurn2(fd, PlayerIndex);
    }
    else
    {
        int nogame = 0;
        write(fd, &nogame, sizeof(nogame));
    }
};

void gamePlayerTurn(int fd, int PlayerIndex)
{
    printf("[GameStarted]\n");
    //initializam runda cu 0 si variabilele ajutatoare
    int turn = 0;
    int enemyDecizie;
    int decizie;
    int flag;
    int enemyflag;
    int winner = 0;
    if (PlayerIndex == 0)
    {
        flag = 1;
        enemyflag = 2;
    }
    else
    {
        flag = 2;
        enemyflag = 1;
    }

    game Game;
    Game.setupMatrix(8);
    int myWins;
    int enemyWins;
    read(fd, &myWins, sizeof(myWins));
    read(fd, &enemyWins, sizeof(enemyWins));
    while (winner == 0)
    {
        decizie = -1;
        system("clear");
        printf("Score: You:%d | Enemy: %d\n", myWins, enemyWins);
        printf("Your flag is : %d \n", flag);
        printf("Enemy's flag is : %d\n", enemyflag);
        Game.printMatrix();
        cin.clear();
        cin.sync();
        fflush(stdin);
        //in cazul in care este runda noastra
        if (turn == PlayerIndex)
        {
            do
            {
                printf("Is your turn!\n");
                //Decidem un mesaj
                printf("[Your Message]: ");
                cin >> decizie;
            } while (decizie > 8 || decizie < 0 || Game.columnFull(decizie));
            write(fd, &decizie, sizeof(decizie));
            Game.addInColumn(decizie, flag);
        }
        //in cazul in care este runda inamicului
        else
        {
            printf("Is enemys's turn !\n");
            printf("[EnemyMessage]: ");

            //citim si afisam mesajul inamicului de la server
            read(fd, &enemyDecizie, sizeof(enemyDecizie));
            printf("%d\n", enemyDecizie);
            Game.addInColumn(enemyDecizie, enemyflag);
            //anuntam serverul ca am primit mesajul
            write(fd, &turn, sizeof(turn));
        }

        //Next turn
        turn = 1 - turn;
        winner = Game.endGame();
    }
    int win;
    int endGameServer = 100;
    write(fd, &endGameServer, sizeof(endGameServer));
    system("clear");
    Game.printMatrix();
    if (flag == winner)
    {
        printf("You Won!\n\n");
        win = 1;
    }
    else
    {
        printf("You Lost!\n\n");
        win = 0;
    }

    printf("Would you like to play again with your enemy ?\n0 for NO, 1 for YES\n");
    write(fd, &win, sizeof(win));
    int decizie;
    int decizieEnemy;

    do
    {
        cin >> decizie;
    } while (decizie != 0 && decizie != 1);

    write(fd, &decizie, sizeof(decizie));
    read(fd, &decizieEnemy, sizeof(decizieEnemy));
    printf("Enemy decision:%d\n", decizieEnemy);
    if (decizie == 1 && decizieEnemy == 1)
    {
        write(fd, &decizie, sizeof(decizie));
        gamePlayerTurn(fd, PlayerIndex);
    }
    else
    {
        int nogame = 0;
        write(fd, &nogame, sizeof(nogame));
    }
}

bool joinGame(int fd, utilizator &Utilizator)
{
    printf("[In Join Function]\n");
    //citim numarul de lobbyuri disponibile
    int numberOfGames = 0;
    read(fd, &numberOfGames, sizeof(numberOfGames));
    printf("Number of open lobbys:%d\n", numberOfGames);
    //daca nu exista nici un lobby in lista, iesim si reluam dicizia
    if (numberOfGames == 0)
    {
        printf("[Server]No Lobbys\n");
        return false;
    }

    char name[20];
    int len;
    int index;

    //afisam lista de lobbyuri cu indexi ei
    for (int i = 0; i < numberOfGames; i++)
    {
        read(fd, &index, sizeof(index));
        read(fd, &len, sizeof(len));
        read(fd, &name, len);
        name[len] = 0;
        printf("%d %s \n", index, name);
    }

    //alegem un lobby
    printf("Decizia este:");
    int decizia_mia;
    cin >> decizia_mia;

    //anuntam serverul in ce lobby dorim sa intram
    write(fd, &decizia_mia, sizeof(decizia_mia));
    //incepem jocul cu playerul din lobby
    int PlayerIndex;
    read(fd, &PlayerIndex, sizeof(PlayerIndex));
    gamePlayerTurn2(fd, PlayerIndex);
}

bool createGame(int fd, utilizator &Utilizator)
{
    int turn;
    printf("[In CreateGame Function] ");
    printf("Waiting For Server Response\n");

    do
    {
        printf("You want to be first? Press 0 !\n");
        printf("You want to be second? Press 1 !\n");
        cin >> turn;
    } while (turn != 0 && turn != 1);
    write(fd, &turn, sizeof(turn));
    //asteptam sa fim anuntati de catre player ca intra in
    //lobby, pe urma anuntam serverul ca putem incepe jocul
    int response = 1;
    read(fd, &response, sizeof(response));
    printf("response:%d\n", response);
    write(fd, &response, sizeof(response));

    //incepem jocul cu playerul ce a intrat in lobby
    gamePlayerTurn2(fd, turn);
}
// gcc -pthread server.cpp -lstdc++ -o server.o
// gcc -pthread client.cpp -lstdc++ -o client.o

bool decision(int fd, utilizator &Utilizator)
{
    if (!Utilizator.esteLogat())
    {
        char nume[20];
        int usedName = 0;
        do
        {
            if (usedName != 0)
                printf("Name allready used, try another one\n");
            printf("Enter your name: ");
            std::cin.getline(nume, 20, '\n');

            int len = strlen(nume);

            write(fd, &len, sizeof(len));
            write(fd, &nume, len);
            read(fd, &usedName, sizeof(usedName));
        } while (usedName != 0);
        Utilizator.setUp(fd, nume);
        printf("Your name is :%s  \n", Utilizator.username);
    }
    else
    {
        int decision = 2;
        do
        {
            printf("If Exit Enter 2\nIf Join Enter 1\nIf CreateGame Enter 0\n");
            printf("Your decision is:");
            cin >> decision;
        } while (decision != 0 && decision != 1 && decision != 2);

        write(fd, &decision, sizeof(decision));
        if (decision == 1)
        {
            printf(" ~~> Join!\n");
            joinGame(fd, Utilizator);
        }
        else if (decision == 0)
        {
            printf(" ~~> CreateGame!\n");
            createGame(fd, Utilizator);
        }
        else if (decision == 2)
        {
            myExit = true;
        }
    }
}

void signalHandler(int signal)
{
    exit(signal);
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
    server.sin_port = htons(2024);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    int test;
    if ((test = connect(SocketClient, (struct sockaddr *)&server, sizeof(server))) == -1)
    {
        std::cerr << "[CLIENT]Eroare la conectare\n";
        return errno;
    }

    char buffer[512];
    utilizator User;
    while (!myExit)
    {
        decision(SocketClient, User);
    }
}
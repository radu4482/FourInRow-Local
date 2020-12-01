#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

int main()
{

    //create server socket
    int network_socket;
    server_socket = socket(AF_INET,SOCK_STREAM,0);

    //define the server address
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_addres.sin_port = HTONS(9002);
    server_addres.sin_addr.s_addr = INADDR_ANY;

    //bind the socket to out specified IP and port
    bind(server_socket,(struct sockaddr*) &server_address,sizeof(server_address));

    listen(server_socket,5);

    int client_socket,5);

    int client_socket;
    client_socket=accept(server_socket,NULL,NULL);

    //send the message
    send(client_socket,server_message,sizeof(server_message),0);

    //close the socket
    close(server_socket);

    return 0;

}
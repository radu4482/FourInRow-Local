#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

struct login{
    char nume[200];
    char prenume[200];
    char username[20];
    char password[20];
};

void login()
{
    char username[200],password[200];
    FILE *log;
    log=fopen("login12.txt","r");
    struct login l;
    printf("Enter username: ");
    scanf("%s",username);
    printf("Enter parola: ");
    scanf("%s",password);

    while(fread(&l,sizeof(l),1,log))
{
    if(strcmp(username,l.username)==0&&strcmp(password,l.password)==0)
{
    printf("Succesful login\n");
}
    else{
    printf("Please Enter correct username and password");
}
}
 fclose(log);   
}


void registe(){

    FILE *log;
    log=fopen("login12.txt","w");
    struct login l;

    printf("Enter nume: ");scanf("%s",l.nume);
    printf("Enter prenume: ");scanf("%s",l.prenume);
    printf("Enter username: ");scanf("%s",l.username);
    printf("Enter parola: ");scanf("%s",l.password);
    fwrite(&l,sizeof(l),1,log);

    login();    
}


int main()
{
    int cho;
    
    printf("Press '1' for register\nPress'2'for login\n\n");
    scanf("%d",&cho);
    if(cho==1)
    {
    registe();
}
    else if (cho==2)
{
    login();
}
return 0;
}
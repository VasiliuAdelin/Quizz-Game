/* cliTCPIt.c - Exemplu de client TCP
   Trimite un numar la server; primeste de la server numarul incrementat.
         
   Autor: Lenuta Alboaie  <adria@infoiasi.ro> (c)2009
*/
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <signal.h>
#include <poll.h>
#include <time.h>
#include <unistd.h>

#define time_until_next_question 2


/* codul de eroare returnat de anumite apeluri */
extern int errno;

/* portul de conectare la server*/
int port; int logged_in=0,quit=0;

struct answer
{
  int correctitude;
  char text[100];
}ans[4];

int main (int argc, char *argv[])
{
  int sd;			// descriptorul de socket
  struct sockaddr_in server;	// structura folosita pentru conectare 
  		// mesajul trimis
  int nr=0;
  char buf[10];

  /* exista toate argumentele in linia de comanda? */
  if (argc != 3)
    {
      printf ("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
      return -1;
    }

  /* stabilim portul */
  port = atoi (argv[2]);

  /* cream socketul */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("Eroare la socket().\n");
      return errno;
    }

  /* umplem structura folosita pentru realizarea conexiunii cu serverul */
  /* familia socket-ului */
  server.sin_family = AF_INET;
  /* adresa IP a serverului */
  server.sin_addr.s_addr = inet_addr(argv[1]);
  /* portul de conectare */
  server.sin_port = htons (port);
  
  /* ne conectam la server */
  if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1)
    {
      perror ("[client]Eroare la connect().\n");
      return errno;
    }

  /* citirea mesajului */
  int quit=0;
  while(quit==0&&logged_in==0)
  {
    printf ("[client]Introduceti comanda dorita:\n ");
    printf("1 - Login\n");
    printf("2 - Inregistrare\n");
    printf("3 - Iesire\n");
    fflush (stdout);
    read (0, buf, sizeof(buf));
    nr=atoi(buf);
  
      switch(nr)
      {
        case 1:
        {
          write(sd,&nr,sizeof(int));
          char user_name_login[100], user_password_login[100];
          bzero(user_name_login,100);bzero(user_password_login,100);
          printf("Ati ales sa va logati.\n Introduceti numele de utilizator:\n");
          read(0,user_name_login,100);
          printf("Intoduceti parola:\n");
          read(0,user_password_login,100);
          printf("%d",(int)(strlen(user_name_login)));printf("%d\n",(int)(strlen(user_password_login)));
            /* trimiterea mesajului la server */
            if ((write (sd,user_name_login,sizeof(user_name_login)) <= 0)||(write (sd,user_password_login,sizeof(user_name_login)) <= 0))
        {
          perror ("[client]Error login data writting.\n");
          return errno;
        }

        //testare BD daca se gaseste utilizatorul.
        int login_approval=0;
        user_name_login[strlen(user_name_login)-1]='\0';
        if(read(sd, &login_approval,sizeof(int)) <= 0)   //logged_in=0-nelogat; logged_in=1-logat;logged_in=2-date inexistente
        {
          perror("[client]Error receiving login response.\n");
        }
        if(login_approval==0)
        printf("Nume de utilizator sau parola gresite.\n");
        if(login_approval==1)
        {
          logged_in=1;
          printf("V-ati logat cu success. Acum puteti participaa la Quizz.\n");
        }
        if(login_approval==2)
        printf("Utilizatorul cu numele %s,este deja conectat.\n",user_name_login);
        }break;

        case 2:
        {
          write(sd,&nr,sizeof(int));
          char user_name_register[100], user_password_register[100];
          bzero(user_password_register,100);bzero(user_name_register,100);
          printf("Ati ales sa va inregistrati.\n Introduceti numele de utilizator:\n");
          read(0,user_name_register,100);
          printf("Intoduceti parola:\n");
          read(0,user_password_register,100);
          if ((write (sd,user_name_register,sizeof(user_name_register)) <= 0)||(write (sd,user_password_register,sizeof(user_name_register)) <= 0))
          {
            perror ("[client]Error register data writting.\n");
            return errno;
          }
          int register_approval=0;
          if(read(sd, &register_approval,sizeof(int)) <= 0)
        {
          perror("[client]Error receiving rregister approval.");
        }
        if(register_approval==1)
          printf("V-ati inregistrat cu succes. Acum puteti folosi numele si parola pentru a va loga.\n");
        if(register_approval==0)
          printf("Numele de utilizator este deja folosit.\n");
        }break;
        case 3:
        {
          write(sd,&nr,sizeof(int));
          quit=1;
        }break;
      }
  }
  system("clear");
  if(logged_in==1)
  {
    printf("\tJocul de quiz va incepe in curand!\n");
    struct pollfd mypoll = { STDIN_FILENO, POLLIN|POLLPRI };
    char question[100],ans[100];int quiz=1,response,diff;
    time_t start,end;
    if(read(sd,&quiz,sizeof(quiz))<=0)
    {
      printf("Server indisponibil.\n");
      quiz=5;
    }
    else
    printf("\tJocul de Quiz a inceput!\n");
    
    while(quiz<=4)
    {
      if(read(sd,&quiz,sizeof(int))<=0)
        quiz=3;
      else
      if(quiz==1)
      {
        bzero(question,100);
        if(read(sd,question,sizeof(question)) <= 0)
        {
          quiz=3;   //inseamna ca serverul a picat.
        }
        else
        {
          printf("\t%s\n",question);
          response=0;
          question[strlen(question)-1]='\0';
          system("clear");
          if(strcmp(question,"afk")==0)
          {
            printf("Ne pare rau dar ati fost afk pentru prea mult timp.\n\tAti fost deconectat!\n");
            quiz=2;
          }
          else
          {
            printf("\tIntrebarea este:\n");
            printf("\t%s\n",question);
            for(int i=1;i<=4;i++)
            {
              bzero(ans,100);
              read(sd,ans,sizeof(ans));
              printf("%s\n",ans);
            }
            printf("Aveti 10 secunde pentru a raspunde.\n\t\tStart!\n");
            response=0;
            time(&start);
            if( poll(&mypoll, 1, 10000) )
            { 
                scanf("%d", &response);
                time(&end);
            }
            else 
            {
                time(&end);
                printf("\nTimpul s-a scurs. Nu ati reusit sa raspundeti la intrebare.\n"); 
            }  
            diff= difftime(end,start);
            printf("\nAti raspuns la intrebare in %d secunde.\n",diff);
            write(sd,&response,sizeof(response));
            write(sd,&diff,sizeof(diff));
            char msg[100];
            bzero(msg,100);
            read(sd,msg,sizeof(msg));
            printf("%s\n",msg);
            sleep(time_until_next_question);
          }
        }        
      }
      else if (quiz==2)
      {
        printf("Ati fost inactiv o perioada prea lunga de timp.\n\tAti fost deconectat!");
        quiz=3;
      }
      else if (quiz==3)
      {
          printf("Serverul a picat!");
          close(sd);
          quiz=5;
      }
      
      else if (quiz==4)
      {
        printf("Jocul de Quiz s-a terminat.\n");
        char aux_castigator[100];int aux_max_punctaj;
        read(sd,aux_castigator,sizeof(aux_castigator));
        read(sd,&aux_max_punctaj,sizeof(aux_max_punctaj));
        printf("Castigatorul este: %s si are punctajul:",aux_castigator);
        printf("%d\n",aux_max_punctaj);
        quiz=5;
      }
      
      
    }
  }

  /* inchidem conexiunea, am terminat */
  close (sd);
}
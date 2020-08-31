/* servTCPConcTh2.c - Exemplu de server TCP concurent care deserveste clientii
   prin crearea unui thread pentru fiecare client.
   Asteapta un numar de la clienti si intoarce clientilor numarul incrementat.
	Intoarce corect identificatorul din program al thread-ului.
  
   
   Autor: Lenuta Alboaie  <adria@infoiasi.ro> (c)2009
*/
/*clang-format on*/
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <sqlite3.h> 


/* portul folosit */
#define PORT 2908
#define quiz_length 5
#define question_id_starter 1;


int login_approval,register_approval,clients_logged_in=0,score[]={50,45,40,35,30,25,20,15,10,5,0},total_raspunsuri_BD,quiz_is_on=0;

void quiz_game(void *arg);

struct answer
{
    char text[100];
    int id;
    int correctitude;
}answers[4];

struct question
{
    char text[100];
    int id;
    int value;
}question;

static int answers_extract(void *NotUsed, int argc, char **argv, char **azColName) 
{
   int i;
   
   
   for(i = 0; i < argc; i++)
   {
       if(argv[i]!=NULL)
       {
           answers[total_raspunsuri_BD].id=atoi(argv[i]);
           strcpy(answers[total_raspunsuri_BD].text,argv[++i]);
           answers[total_raspunsuri_BD].correctitude=atoi(argv[++i]);
           total_raspunsuri_BD++;
       }
   }
   return 0;
}

static int question_extract(void *NotUsed, int argc, char **argv, char **azColName) 
{
   int i;
   
   for(i = 0; i < argc; i++)
   {
       if(argv[i]!=NULL)
       {
           question.id=atoi(argv[i]);
           strcpy(question.text,argv[++i]);
           question.value=atoi(argv[++i]);
       }
   }
   return 0;
}


static int callback_login_verify(void *NotUsed, int argc, char **argv, char **azColName) 
{
   int i;
   for(i = 0; i < argc; i++)
   {
        if(argv[i]!=NULL)
        login_approval=1;
   }
   return 0;
}

static int callback_register_verify(void *NotUsed, int argc, char **argv, char **azColName) 
{
   int i;
   for(i = 0; i < argc; i++)
   {
        if(argv[i]!=NULL)
        register_approval;
   }
   return 0;
}

/* codul de eroare returnat de anumite apeluri */
extern int errno;

typedef struct thData{
	int idThread; //id-ul thread-ului tinut in evidenta de acest program
	int cl; //descriptorul intors de accept
  char nume_jucator[100];
  int punctaj_jucator;
  int logged_in;
  int afk;
}thData;

int th_counter=0;
thData th_vector[100];

static void *treat(void *); /* functia executata de fiecare thread ce realizeaza comunicarea cu clientii */
void login_and_register(void *);
void afiseaza_clienti(void *);

int main ()
{
  struct sockaddr_in server;	// structura folosita de server
  struct sockaddr_in from;	
  int sd;		//descriptorul de socket 
  int pid;
  pthread_t th[100];    //Identificatorii thread-urilor care se vor crea
	int i=0;
  
  

  /* crearea unui socket */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("[server]Eroare la socket().\n");
      return errno;
    }
  /* utilizarea optiunii SO_REUSEADDR */
  int on=1;
  setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
  
  /* pregatirea structurilor de date */
  bzero (&server, sizeof (server));
  bzero (&from, sizeof (from));
  
  /* umplem structura folosita de server */
  /* stabilirea familiei de socket-uri */
    server.sin_family = AF_INET;	
  /* acceptam orice adresa */
    server.sin_addr.s_addr = htonl (INADDR_ANY);
  /* utilizam un port utilizator */
    server.sin_port = htons (PORT);
  
  /* atasam socketul */
  if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
    {
      perror ("[server]Eroare la bind().\n");
      return errno;
    }

  /* punem serverul sa asculte daca vin clienti sa se conecteze */
  if (listen (sd, 2) == -1)
    {
      perror ("[server]Eroare la listen().\n");
      return errno;
    }
  /* servim in mod concurent clientii...folosind thread-uri */
  while (1)
    {
      int client;
      thData * td; //parametru functia executata de thread     
      int length = sizeof (from);

      printf ("[server]Asteptam la portul %d...\n",PORT);
      fflush (stdout);

      // client= malloc(sizeof(int));
      /* acceptam un client (stare blocanta pina la realizarea conexiunii) */
      if ( (client = accept (sd, (struct sockaddr *) &from, &length)) < 0)
	{
	  perror ("[server]Eroare la accept().\n");
	  continue;
	}
	
        /* s-a realizat conexiunea, se astepta mesajul */
    
	// int idThread; //id-ul threadului
	// int cl; //descriptorul intors de accept

	td=(struct thData*)malloc(sizeof(struct thData));
	td->idThread=i++;
	td->cl=client;
  th_vector[th_counter].idThread=td->idThread;
  th_vector[th_counter++].cl=td->cl;

	pthread_create(&th[i], NULL, &treat, td);
				
	}//while
};				
static void *treat(void * arg)
{		
		struct thData tdL; 
		tdL= *((struct thData*)arg);	
		printf ("[thread]- %d - Asteptam mesajul...\n", tdL.idThread);
		fflush (stdout);
		pthread_detach(pthread_self());
		login_and_register((struct thData*)arg);int t=0;
    quiz_game((struct thData*)arg);
    /* am terminat cu acest client, inchidem conexiunea */
    t++;
    if(quiz_is_on)
    printf("Al %d-lea client asteapta terminarea quiz-ului.\n",t);
    else
    printf("Am terminat cu toti clientii.\n");
		//close ((intptr_t)arg);
		return(NULL);	
  		
};

void quiz_game(void *arg)
{
  sqlite3 *db;
  char *zErrMsg = 0;
  int rc;
  char sql_aux[100], *sql;
  char buffer[100],aux[100];
  rc = sqlite3_open("test.db", &db);
  struct thData tdL;
  tdL= *((struct thData*)arg);
  int quiz=1,response,question_id=question_id_starter;
  int correct_answer;
  int questions_counter=0,temporal_score;                     //raspuns1|corectitudine raspuns2|corectitudine 

  if(clients_logged_in>=2)
  {
    quiz_is_on=1;
    for(int k=0; k<th_counter;k++)
          {
            write(th_vector[k].cl,&quiz,sizeof(int));
          }
    sleep(5);
    while(quiz==1&&questions_counter<=quiz_length)
    {
      sleep(5);
      for(int k=0; k<th_counter;k++)
      {
        if(write(th_vector[k].cl,&quiz,sizeof(int))<=0)
        {
          th_vector[k].afk=5;
          close(th_vector[k].cl);
        }
      }
      strcpy(sql_aux,"select * from questions where id=");
      sprintf(buffer, "%d", question_id);
      strcat(sql_aux,buffer);
      strcat(sql_aux," ;");
      sql=*(&sql_aux);
      rc = sqlite3_exec(db, sql, question_extract, 0, &zErrMsg);
      printf("%s\n",question.text);
      strcpy(sql_aux,"select * from answers where id=");
      sprintf(buffer, "%d", question_id);
      strcat(sql_aux,buffer);
      strcat(sql_aux," ;");
      sql=*(&sql_aux);
      rc = sqlite3_exec(db, sql, answers_extract, 0, &zErrMsg);
       
      //writes
      for(int k=0; k<th_counter;k++)
      {
        if(th_vector[k].afk==3)
        {
          printf("Userul %s a fost deconectat pentru inactivitate.\n",th_vector[k].nume_jucator);
          write(th_vector[k].cl,"afkk",sizeof("afkk"));
          close(th_vector[k].cl);
        }
        else if(th_vector[k].afk<=3)
        {
          write(th_vector[k].cl,question.text,sizeof(question.text));
          int aux_c=1;
          for(int t=0;t<=total_raspunsuri_BD;t++,aux_c++)
          {
            if(answers[t].id==question_id)
            {
              write(th_vector[k].cl,answers[t].text,sizeof(answers[t].text));
              if(answers[t].correctitude==1)
              {
                correct_answer=aux_c;
              }
            }
          }
        }
      }
      
      for(int k=0; k<th_counter;k++)
      {
        if(th_vector[k].afk<3)
        if(read(th_vector[k].cl,&response,sizeof(response))<=0)
        {
          close(th_vector[k].cl);
          th_vector[k].afk=5;
          continue;
        }
        else
          read(th_vector[k].cl,&temporal_score,sizeof(temporal_score));
        
        if(response==0)
        {
          th_vector[k].afk++;
        }
        printf("Userul: %s are scorul:\t",th_vector[k].nume_jucator);
        if(response==correct_answer%4)
        {
          th_vector[k].punctaj_jucator+=score[temporal_score];
          write(th_vector[k].cl,"Corect!",sizeof("Corect!"));
        }
        else
          write(th_vector[k].cl,"Gresit!",sizeof("Gresit!"));
          printf("%d\n",th_vector[k].punctaj_jucator);
      }
      
      question_id++;
      questions_counter++;
      if(questions_counter>=quiz_length)
      {
        for(int k=0; k<th_counter;k++)
        {
          quiz=4;
          write(th_vector[k].cl,&quiz,sizeof(int));
          int aux_max_punctaj=0;
          char aux_castigator[100];
          for(int k=0;k<th_counter;k++)
          {
            if(th_vector[k].punctaj_jucator>aux_max_punctaj)
            {
              bzero(aux_castigator,100);
              aux_max_punctaj=th_vector[k].punctaj_jucator;
              strcpy(aux_castigator,th_vector[k].nume_jucator);
              strcat(aux_castigator,"\0");
            }
          }
          printf("%d\n",aux_max_punctaj);
          printf("%s\n",aux_castigator);
          write(th_vector[k].cl,aux_castigator,sizeof(aux_castigator));
          write(th_vector[k].cl,&aux_max_punctaj,sizeof(aux_max_punctaj));
          close(th_vector[k].cl);
          quiz_is_on=0;
        }
      }

    }
  }
}


void login_and_register(void *arg)
{ 
  sqlite3 *db;
  char *zErrMsg = 0,sql_aux[100], *sql;
  int rc;
  rc = sqlite3_open("test.db", &db);
  int user_choice,i=0,client_exit=0,quit=0;
	struct thData tdL; 
	tdL= *((struct thData*)arg);
  th_vector[tdL.idThread].logged_in=0;
  while(th_vector[tdL.idThread].logged_in==0&&quit==0)
  {
	 if (read (tdL.cl, &user_choice,sizeof(int)) <= 0)
			{
			  printf("[Thread %d]\n",tdL.idThread);
			  perror ("[server][citire_comanda_client]Eroare la read() de la client.\n");
			}
  switch(user_choice)
  {
    case 1:
    {
      login_approval=0;
      char user_name_login[100], user_pass_login[100];

      if ((read (tdL.cl, user_name_login,sizeof(user_name_login)) <= 0)||(read(tdL.cl,user_pass_login,sizeof(user_pass_login)) <= 0))
      {
        printf("[Thread %d]\n",tdL.idThread);
        perror ("[server]Error read login data\n");
      }
      user_name_login[strlen(user_name_login)-1]='\0';
      user_pass_login[strlen(user_pass_login)-1]='\0';
      printf("Am primit de la client numele:  %s\n", user_name_login);
      printf("Am primit de la client parola:  %s\n", user_pass_login);
      sql = "select name from users where name=\"";
      strcpy(sql_aux,sql);
      strcat(sql_aux,user_name_login);
      strcat(sql_aux,"\" and pass=\"");
      strcat(sql_aux,user_pass_login);
      strcat(sql_aux,"\";");
      sql=*(&sql_aux);
      rc = sqlite3_exec(db, sql, callback_login_verify, 0, &zErrMsg);
      for(int k=0;k<th_counter;k++)
      {
        if(strcmp(th_vector[k].nume_jucator,user_name_login)==0&&th_vector[k].logged_in==1)
        login_approval=2;
      }
          /* returnam mesajul clientului */
      if (write (tdL.cl, &login_approval, sizeof(int)) <= 0)
        {
        printf("[Thread %d] ",tdL.idThread);
        perror ("[server]Eroare la raspuns login\n");
        }
        if(login_approval==1)
        {
          clients_logged_in++;
          strcpy(th_vector[tdL.idThread].nume_jucator,user_name_login);
          th_vector[tdL.idThread].logged_in=1;
          th_vector[tdL.idThread].punctaj_jucator=0;
          th_vector[tdL.idThread].afk=0;
        }
    }break;

    case 2:
    {
      register_approval=1;
      char user_name_register[100], user_pass_register[100];
      if ((read (tdL.cl, user_name_register,sizeof(user_name_register)) <= 0)||(read(tdL.cl,user_pass_register,sizeof(user_pass_register)) <= 0))
      {
        printf("[Thread %d]\n",tdL.idThread);
        perror ("[server]Error read login data\n");
      }
      user_name_register[strlen(user_name_register)-1]='\0';
      user_pass_register[strlen(user_pass_register)-1]='\0';
      printf("Am primit de la client numele:  %s\n", user_name_register);
      printf("Am primit de la client parola:  %s\n", user_pass_register);
      sql = "select name from users where name=\"";
      strcpy(sql_aux,sql);
      strcat(sql_aux,user_name_register);
      strcat(sql_aux,"\";");
      sql=*(&sql_aux);
      rc = sqlite3_exec(db, sql, callback_register_verify, 0, &zErrMsg);
      if(register_approval==0)
      {
        if (write (tdL.cl, &register_approval, sizeof(int)) <= 0)
        {
        printf("[Thread %d] ",tdL.idThread);
        perror ("[server]Eroare la raspuns inregistrare.\n");
        }
      }
      else if(register_approval==1)
      {
        sql= "insert into users values(\"";
        strcpy(sql_aux,sql);
        strcat(sql_aux,user_name_register);
        strcat(sql_aux,"\",\"");
        strcat(sql_aux,user_pass_register);
        strcat(sql_aux,"\",0)");
        sql=*(&sql_aux);
        printf("%s\n",sql);
        rc = sqlite3_exec(db, sql, callback_register_verify, 0, &zErrMsg);
        if (write (tdL.cl, &register_approval, sizeof(int)) <= 0)
        {
        printf("[Thread %d] ",tdL.idThread);
        perror ("[server]Eroare la raspuns inregistrare.\n");
        }
      }
    }break;

    case 3: quit=1; break;
    default: break;
  }
  }
  }
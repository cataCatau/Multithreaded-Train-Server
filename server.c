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
#include <stdbool.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <time.h>
#include "shared.h"

#define PORT 3010 // folosim portul 3010 in client si in server
extern int errno; // codul de eroare pentru apeluri

static void *treat(void *); /* functia executata de fiecare thread ce realizeaza comunicarea cu clientii */

tren Trenuri[1024];
int nr_trenuri;
typedef struct
{
  pthread_t idThread;
  int thCount;
} Thread;

Thread *threadsPool; // un array de structuri Thread

int sd;                                                // descriptorul de socket de ascultare
int nthreads;                                          // numarul de threaduri
pthread_mutex_t file_lock = PTHREAD_MUTEX_INITIALIZER; // mutex pentru accesul la fisier
pthread_mutex_t ram_lock = PTHREAD_MUTEX_INITIALIZER;  // mutex pentru modificare ram bilete/intarziere/avans
pthread_mutex_t mlock = PTHREAD_MUTEX_INITIALIZER;     // variabila mutex ce va fi partajata de threaduri

void raspunde(int cl, int idThread, ClientState *state);

int main(int argc, char *argv[])
{

  struct sockaddr_in server; // structura folosita de server
  void threadCreate(long);

  nthreads = 50;
  threadsPool = calloc(sizeof(Thread), nthreads);

  load_trains_from_xml("database.xml");

  /* crearea unui socket */
  if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror("[server]Eroare la socket().\n");
    return errno;
  }
  /* utilizarea optiunii SO_REUSEADDR */
  int on = 1;
  setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

  /* pregatirea structurilor de date */
  memset(&server, 0, sizeof(server));

  /* umplem structura folosita de server */
  /* stabilirea familiei de socket-uri */
  server.sin_family = AF_INET;
  /* acceptam orice adresa */
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  /* utilizam un port utilizator */
  server.sin_port = htons(PORT);

  /* atasam socketul */
  if (bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
  {
    perror("[server]Eroare la bind().\n");
    return errno;
  }

  /* punem serverul sa asculte daca vin clienti sa se conecteze */
  if (listen(sd, 2) == -1)
  {
    perror("[server]Eroare la listen().\n");
    return errno;
  }

  fflush(stdout);
  for (int i = 0; i < nthreads; i++)
  {
    threadCreate(i);
  }

  /* servim in mod concurent clientii...folosind thread-uri */
  for (;;)
  {
    printf("[server]Asteptam la portul %d...\n", PORT);
    pause();
  }
};

void threadCreate(long i)
{
  void *treat(void *);

  pthread_create(&threadsPool[i].idThread, NULL, &treat, (void *)i);
  return; /* threadul principal returneaza */
}
void *treat(void *arg)
{
  int client;

  struct sockaddr_in from;
  bzero(&from, sizeof(from));
  printf("[thread]- %ld - pornit...\n", (long)arg);
  fflush(stdout);

  for (;;)
  {
    socklen_t length = sizeof(from);
    // pthread_mutex_lock(&mlock);
    if ((client = accept(sd, (struct sockaddr *)&from, &length)) < 0)
    {
      perror("[thread]Eroare la accept().\n");
    }
    // pthread_mutex_unlock(&mlock);
    threadsPool[(long)arg].thCount++;

    ClientState state = {.is_logged_in = false, .is_admin = false};
    raspunde(client, (long)arg, &state); // procesarea cererii
    /* am terminat cu acest client, inchidem conexiunea */
    close(client);
    return NULL;
  }
}

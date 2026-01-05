#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>

extern int errno; // codul de eroare returnat de apeluri

int port = 3010; // portul de conectare la server

int main(int argc, char *argv[])
{
  int sd;                    // descriptorul de socket
  struct sockaddr_in server; // structura folosita pentru conectare
  char buf[1024];            // trimitem mesajul prin buffer

  if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) // creem socketul
  {
    perror("Eroare la socket().\n");
    return errno;
  }

  /* umplem structura folosita pentru realizarea conexiunii cu serverul */
  /* familia socket-ului */
  server.sin_family = AF_INET;
  /* adresa IP a serverului */
  server.sin_addr.s_addr = inet_addr("127.0.0.1");
  /* portul de conectare */
  server.sin_port = htons(port);

  /* ne conectam la server */
  if (connect(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
  {
    perror("[client]Eroare la connect().\n");
    return errno;
  }

  printf("Conectat la server. Introduceti 'help' pentru lista de comenzi valabile\n");

  while (1)
  {
    printf("\n[client] Comanda: ");
    fflush(stdout);

    int bytes_read = read(0, buf, sizeof(buf) - 1);

    if (bytes_read <= 0)
    {
      printf("[client] Eroare la citirea comenzii sau EOF.\n");
      break;
    }

    // Curatam buffer-ul de orice \n ramas de la citirea anterioara
    // si ne asiguram ca nu trimitem \n la server
    if (bytes_read > 0 && buf[bytes_read - 1] == '\n')
    {
      bytes_read--;
    }
    buf[bytes_read] = '\0'; // Terminare corecta

    // trimitem comanda la server
    if (write(sd, buf, bytes_read) <= 0)
    {
      perror("[client]Eroare la write() spre server.\n");
      break;
    }

    // citim raspunsul de la server
    if (strncmp(buf, "get_schedule", 12) == 0 ||
        strcmp(buf, "get_delays") == 0 ||
        strcmp(buf, "get_earlys") == 0 ||
        strncmp(buf, "get_departures ", 15) == 0 ||
        strncmp(buf, "get_arrivals ", 13) == 0)
    {
      while (1)
      {
        int bytes_received = read(sd, buf, sizeof(buf) - 1);
        if (bytes_received <= 0)
          break;

        buf[bytes_received] = '\0'; // Punem terminatorul

        // Verificam daca am primit semnalul de final
        char *ptr = strstr(buf, "<GATA>");
        if (ptr != NULL)
        {
          *ptr = '\0';
          printf("%s", buf);
          break;
        }

        // Afisam bucata primita
        printf("%s", buf);
      }
    }
    else
    {
      int bytes_received;
      if ((bytes_received = read(sd, buf, sizeof(buf) - 1)) <= 0)
      {
        if (strncmp(buf, "exit", 4) == 0)
        {
          printf("[client] Clientul s-a deconectat.\n");
          break; // Iesim din bucla
        }
        if (bytes_received == 0)
        {
          printf("[client] Serverul s-a deconectat.\n");
        }
        else
        {
          perror("[client]Eroare la read() de la server.\n");
        }
        break; // Iesim din bucla
      }
      // Terminare corecta a raspunsului primit
      buf[bytes_received] = '\0';

      /* afisam mesajul primit */
      printf("[client]Raspuns primit: \n%s\n", buf);
      // Verificam comanda de iesire (exit)
    }

    // Curatam buffer-ul pentru urmatoarea comanda
    memset(buf, 0, sizeof(buf));
  }

  /* inchidem socketul */
  close(sd);
  return 0;
}

#ifndef SHARED_H
#define SHARED_H

#include <stdbool.h>
#include <pthread.h>
#include <netinet/in.h>

typedef struct
{
    char nume_statie[50];
    char ora_sosire[6];
    char ora_plecare[6];
} Oprire;

typedef struct
{
    char id[10];
    char statie_plecare[50];
    char statie_destinatie[50];
    char ora_plecare[6];
    char ora_sosire[6];
    int intarziere;
    int early;
    int locuri_libere;
    Oprire ruta[30];
    int nr_opriri;
} tren;

typedef struct
{
    bool is_logged_in;
    bool is_admin;
} ClientState;

extern tren Trenuri[1024];
extern int nr_trenuri;
extern pthread_mutex_t file_lock;
extern pthread_mutex_t ram_lock;

void load_trains_from_xml(const char *filename);
void update_in_xml(const char *filename, const char *camp, const char *id, int val);

void raspunde(int cl, int idThread, ClientState *state);

#endif
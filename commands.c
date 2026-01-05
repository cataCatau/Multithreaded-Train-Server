#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include "shared.h"

void login_user(char *buf, ClientState *state)
{
    if (state->is_logged_in == true)
    {
        strcpy(buf, "Esti deja logat");
    }
    else
    {
        char nume[50], parola[50], garbage[50];
        if (sscanf(buf + 6, "%49s %49s %s", nume, parola, garbage) == 2)
        {
            FILE *f_users = fopen("users.txt", "r");
            if (f_users == NULL)
            {
                printf("[Eroare] Nu se poate deschide fisierul users.txt");
                return;
            }
            char f_nume[50], f_parola[50];
            int tip_cont;
            bool user_gasit = false;
            while (fscanf(f_users, "%s %s %d", f_nume, f_parola, &tip_cont) != EOF)
            {
                if (strcmp(nume, f_nume) == 0)
                {
                    user_gasit = true;
                    if (strcmp(parola, f_parola) == 0)
                    {

                        state->is_logged_in = true;
                        if (tip_cont == 1)
                            state->is_admin = true;
                        else
                            state->is_admin = false;
                        user_gasit = true;
                        strcpy(buf, "Autentificare cu succes");
                        break;
                    }
                    else
                    {
                        strcpy(buf, "Parola este gresita");
                        break;
                    }
                }
            }
            fclose(f_users);

            if (!user_gasit)
            {
                strcpy(buf, "Acest nume nu exista in baza de date");
            }
        }
        else
            strcpy(buf, "Format gresit, formatul corect este login <nume> <parola>");
    }
}
void register_user(char *buf, ClientState *state)
{
    if (state->is_logged_in == true)
    {
        strcpy(buf, "Esti deja logat");
        return;
    }
    char nume[50], parola[50], garbage[50];
    if (sscanf(buf + 9, "%49s %49s %s", nume, parola, garbage) != 2)
    {
        strcpy(buf, "Format gresit, formatul corect este register <nume> <parola>");
        return;
    }

    FILE *f_check = fopen("users.txt", "r");
    if (f_check)
    {
        char f_nume[50], f_parola[50];
        int tip;
        while (fscanf(f_check, "%s %s %d", f_nume, f_parola, &tip) != EOF)
        {
            if (strcmp(f_nume, nume) == 0)
            {
                strcpy(buf, "Acest nume este deja folosit.");
                fclose(f_check);
                return;
            }
        }
        fclose(f_check);
    }

    pthread_mutex_lock(&file_lock);
    FILE *f_users = fopen("users.txt", "a");
    if (f_users == NULL)
    {
        pthread_mutex_unlock(&file_lock);
        printf("[EROARE] Nu pot deschide users.txt pentru scriere.\n");
        strcpy(buf, "Eroare interna server.");
        return;
    }
    fprintf(f_users, "%s %s 0\n", nume, parola);

    fclose(f_users);
    pthread_mutex_unlock(&file_lock);

    sprintf(buf, "Contul %s a fost creat cu succes", nume);
}
void update_hour(char *temp, int index)
{
    strcpy(temp, "");
    int ora, minut;
    sscanf(Trenuri[index].ora_sosire, "%d:%d", &ora, &minut);
    int total_minute = ora * 60 + minut;
    total_minute += Trenuri[index].intarziere;
    total_minute -= Trenuri[index].early;
    ora = total_minute / 60;
    minut = total_minute % 60;
    if (minut > 9)
        sprintf(temp, "%d:%d", ora, minut);
    else
        sprintf(temp, "%d:0%d", ora, minut);
}
void get_train_info(char *buf)
{
    bool tren_gasit = false;
    char temp[1024];
    for (int i = 0; i < nr_trenuri; i++)
    {
        if (strcmp(buf + 15, Trenuri[i].id) == 0)
        {
            strcpy(buf, "");
            sprintf(temp, "Traseul complet pentru trenul %s:\n\n", Trenuri[i].id);
            strcat(buf, temp);
            sprintf(temp, "%-20s | %-10s | %-10s\n", "STATIA", "SOSIRE", "PLECARE");
            strcat(buf, temp);
            sprintf(temp, "%-20s | %-10s | %-10s\n",
                    Trenuri[i].statie_plecare,
                    "-",
                    Trenuri[i].ora_plecare);
            strcat(buf, temp);
            for (int k = 0; k < Trenuri[i].nr_opriri; k++)
            {
                sprintf(temp, "%-20s | %-10s | %-10s\n",
                        Trenuri[i].ruta[k].nume_statie,
                        Trenuri[i].ruta[k].ora_sosire,
                        Trenuri[i].ruta[k].ora_plecare);
                strcat(buf, temp);
            }
            sprintf(temp, "%-20s | %-10s | %-10s\n\n",
                    Trenuri[i].statie_destinatie,
                    Trenuri[i].ora_sosire,
                    "-");
            strcat(buf, temp);
            if (Trenuri[i].intarziere != 0)
                sprintf(temp, "Trenul %s are o intarziere de %d minute.Acesta este estimat sa soseasca la statia finala la ora ", Trenuri[i].id, Trenuri[i].intarziere);
            else if (Trenuri[i].early != 0)
                sprintf(temp, "Trenul %s are un avans de %d minute.Acesta este estimat sa soseasca la statia finala la ora ", Trenuri[i].id, Trenuri[i].early);
            else
                sprintf(temp, "Trenul %s este estimat sa soseasca la statia finala conform orarului, la ora: ", Trenuri[i].id);
            strcat(buf, temp);
            update_hour(temp, i);
            strcat(buf, temp);
            tren_gasit = true;
            break;
        }
    }
    if (tren_gasit == false)
    {
        strcpy(buf, "ID invalid");
    }
}

void get_schedule(int cl, char *buf)
{
    int nr_arg;
    char s_plecare[50], s_destinatie[50], garbage[50];
    nr_arg = sscanf(buf, "%49s %49s %s", s_plecare, s_destinatie, garbage);
    char temp[512];
    if (nr_arg == 3)
    {
        strcpy(temp, "Format gresit, formatul corect este get_schedule *<statie> *<statie>\n<GATA>");
    }
    else
    {
        if (nr_arg < 0)
            nr_arg = 0;
        sprintf(temp, "%-6s | %-18s | %-18s | %-5s | %-5s | %-8s | %-10s\n",
                "ID", "Plecare", "Destinatie", "Ora P", "Ora S", "Avans", "Intarziere");
        write(cl, temp, strlen(temp));

        sprintf(temp, "------------------------------------------------------------------------------------------\n");
        write(cl, temp, strlen(temp));
        for (int i = 0; i < nr_trenuri; i++)
        {
            bool afiseaza = false;
            if (nr_arg <= 0)
                afiseaza = true;
            else if (nr_arg == 1)
            {
                if (strcmp(Trenuri[i].statie_plecare, s_plecare) == 0)
                    afiseaza = true;
            }
            else if (nr_arg == 2)
            {
                if (strcmp(Trenuri[i].statie_plecare, s_plecare) == 0 &&
                    strcmp(Trenuri[i].statie_destinatie, s_destinatie) == 0)
                    afiseaza = true;
            }
            if (afiseaza)
            {
                sprintf(temp, "%-6s | %-18s | %-18s | %-5s | %-5s | %-3d min  | %-3d min   \n",
                        Trenuri[i].id,
                        Trenuri[i].statie_plecare,
                        Trenuri[i].statie_destinatie,
                        Trenuri[i].ora_plecare,
                        Trenuri[i].ora_sosire,
                        Trenuri[i].early, Trenuri[i].intarziere);

                write(cl, temp, strlen(temp));
                usleep(1000);
            }
        }
        strcpy(temp, "<GATA>");
    }
    write(cl, temp, strlen(temp));
}

void get_departures(int cl, char *nume_statie)
{
    char temp[256];
    sprintf(temp, "Trenuri din %s in urmatoarea ora:\n", nume_statie);
    write(cl, temp, strlen(temp));

    sprintf(temp, "ID     | Destinatie      | Plecare | Peste(min)\n");
    write(cl, temp, strlen(temp));

    sprintf(temp, "------------------------------------------------\n");
    write(cl, temp, strlen(temp));

    time_t rawtime;
    struct tm *timp_curent;
    time(&rawtime);
    timp_curent = localtime(&rawtime);
    int curent_minute_total = timp_curent->tm_hour * 60 + timp_curent->tm_min;

    bool gasit = false;

    for (int i = 0; i < nr_trenuri; i++)
    {
        if (strcmp(nume_statie, Trenuri[i].statie_plecare) == 0)
        {
            int tren_ora, tren_minute;
            sscanf(Trenuri[i].ora_plecare, "%d:%d", &tren_ora, &tren_minute);
            int tren_minute_total = tren_ora * 60 + tren_minute;
            int diferenta = tren_minute_total - curent_minute_total;

            if (diferenta < 0)
                diferenta += 1440;

            if (diferenta >= 0 && diferenta <= 60)
            {
                gasit = true;
                sprintf(temp, "%-6s | %-15s | %-7s | %d min\n",
                        Trenuri[i].id,
                        Trenuri[i].statie_destinatie,
                        Trenuri[i].ora_plecare, diferenta);
                write(cl, temp, strlen(temp));
            }
        }
    }
    if (!gasit)
    {
        sprintf(temp, "Nu sunt trenuri din %s in urmatoarea ora.\n", nume_statie);
        write(cl, temp, strlen(temp));
    }

    strcpy(temp, "<GATA>");
    write(cl, temp, strlen(temp));
}

void get_arrivals(int cl, char *nume_statie)
{
    char temp[256];
    sprintf(temp, "Trenuri care ajung in %s in urmatoarea ora:\n", nume_statie);
    write(cl, temp, strlen(temp));

    sprintf(temp, "ID     | Din Statia      | Sosire  | Peste(min)\n");
    write(cl, temp, strlen(temp));

    sprintf(temp, "------------------------------------------------\n");
    write(cl, temp, strlen(temp));

    time_t rawtime;
    struct tm *timp_curent;
    time(&rawtime);
    timp_curent = localtime(&rawtime);
    int curent_minute_total = timp_curent->tm_hour * 60 + timp_curent->tm_min;

    bool gasit = false;

    for (int i = 0; i < nr_trenuri; i++)
    {
        if (strcmp(nume_statie, Trenuri[i].statie_destinatie) == 0)
        {
            int tren_ora, tren_minute;
            sscanf(Trenuri[i].ora_sosire, "%d:%d", &tren_ora, &tren_minute);
            int tren_minute_total = tren_ora * 60 + tren_minute;
            int diferenta = tren_minute_total - curent_minute_total;
            if (diferenta < 0)
                diferenta += 1440;
            if (diferenta >= 0 && diferenta <= 60)
            {
                gasit = true;
                sprintf(temp, "%-6s | %-15s | %-7s | %d min\n",
                        Trenuri[i].id,
                        Trenuri[i].statie_plecare,
                        Trenuri[i].ora_sosire,
                        diferenta);
                write(cl, temp, strlen(temp));
            }
        }
    }
    if (!gasit)
    {
        sprintf(temp, "Nu sunt trenuri care ajung in %s in urmatoarea ora.\n", nume_statie);
        write(cl, temp, strlen(temp));
    }
    strcpy(temp, "<GATA>");
    write(cl, temp, strlen(temp));
}

void get_delays(int cl)
{
    char temp[256];
    sprintf(temp, "%-6s | %-15s\n", "ID", "Intarziere (min)");
    write(cl, temp, strlen(temp));

    sprintf(temp, "-------------------------\n");
    write(cl, temp, strlen(temp));
    bool gasit = false;
    for (int i = 0; i < nr_trenuri; i++)
    {
        if (Trenuri[i].intarziere > 0)
        {
            sprintf(temp, "%-6s | %-15d\n",
                    Trenuri[i].id,
                    Trenuri[i].intarziere);
            write(cl, temp, strlen(temp));
            gasit = true;
        }
    }
    if (!gasit)
    {
        strcpy(temp, "Niciun tren nu are intarziere in acest moment.\n");
        write(cl, temp, strlen(temp));
    }
    strcpy(temp, "<GATA>");
    write(cl, temp, strlen(temp));
}

void get_earlys(int cl)
{
    char temp[256];
    sprintf(temp, "%-6s | %-15s\n", "ID", "Avans (min)");
    write(cl, temp, strlen(temp));

    sprintf(temp, "-------------------------\n");
    write(cl, temp, strlen(temp));

    bool gasit = false;
    for (int i = 0; i < nr_trenuri; i++)
    {
        if (Trenuri[i].early > 0)
        {
            sprintf(temp, "%-6s | %-15d\n",
                    Trenuri[i].id,
                    Trenuri[i].early);
            write(cl, temp, strlen(temp));
            gasit = true;
        }
    }

    if (!gasit)
    {
        strcpy(temp, "Niciun tren nu a ajuns mai devreme.\n");
        write(cl, temp, strlen(temp));
    }
    strcpy(temp, "<GATA>");
    write(cl, temp, strlen(temp));
}

void report(char *buf, const char *comanda_curenta)
{
    char id_tren[10], garbage[50];
    int minute;
    int nr_arg = sscanf(buf + 13, "%9s %d %s", id_tren, &minute, garbage);
    if (nr_arg != 2)
    {
        strcpy(buf, "Format incorect, formatul corect este report_delay/report_early <ID> <minute>");
    }
    else
    {
        if (minute <= 0 || minute >= 100)
        {
            strcpy(buf, "valoare incorecta pentru intarziere/early");
            return;
        }
        int index = -1;
        for (int i = 0; i < nr_trenuri; i++)
        {
            if (strcmp(id_tren, Trenuri[i].id) == 0)
            {
                if (strcmp(comanda_curenta, "intarziere") == 0)
                    Trenuri[i].intarziere = minute;
                else
                    Trenuri[i].early = minute;
                index = i;
                break;
            }
        }
        if (index != -1)
        {
            // mutex ca sa nu scrie doi deodata in fisier
            pthread_mutex_lock(&file_lock);
            if (Trenuri[index].early > Trenuri[index].intarziere)
            {
                Trenuri[index].early -= Trenuri[index].intarziere;
                Trenuri[index].intarziere = 0;
            }
            else
            {
                Trenuri[index].intarziere -= Trenuri[index].early;
                Trenuri[index].early = 0;
            }
            update_in_xml("database.xml", "intarziere", id_tren, Trenuri[index].intarziere);
            update_in_xml("database.xml", "early", id_tren, Trenuri[index].early);
            pthread_mutex_unlock(&file_lock);
            strcpy(buf, "Actualizare realizata cu succes");
        }
        else
        {
            strcpy(buf, "Nu exista acest tren");
        }
    }
}
void buy_ticket(char *buf)
{
    char id_tren[10], garbage[50];
    bool gasit = false;
    int nr_arg = sscanf(buf + 11, "%9s %s", id_tren, garbage);
    if (nr_arg != 1)
    {
        strcpy(buf, "Format gresit, formatul corect este buy_ticket <ID>");
    }
    else
    {
        for (int i = 0; i < nr_trenuri; i++)
        {
            if (strcmp(Trenuri[i].id, id_tren) == 0)
            {
                pthread_mutex_lock(&file_lock);
                if (Trenuri[i].locuri_libere > 0)
                {
                    Trenuri[i].locuri_libere--;
                    update_in_xml("database.xml", "locuri_libere", id_tren, Trenuri[i].locuri_libere);
                    pthread_mutex_unlock(&file_lock);
                    sprintf(buf, "Ai cumparat un bilet la trenul %s.Mai sunt %d locuri libere", id_tren, Trenuri[i].locuri_libere);
                }
                else
                {
                    pthread_mutex_unlock(&file_lock);
                    strcpy(buf, "Nu mai sunt locuri libere");
                }
                gasit = true;
                break;
            }
        }
        if (!gasit)
            strcpy(buf, "ID invalid");
    }
}
void help(char *buf)
{
    strcpy(buf, ("lista comenzi:\n"));
    strcat(buf, ("exit\n"));
    strcat(buf, ("login <nume> <parola>\n"));
    strcat(buf, ("logout\n"));
    strcat(buf, ("register <nume> <parola>\n"));
    strcat(buf, ("buy_ticket <ID>\n"));
    strcat(buf, ("get_schedule *<statie> *<statie>\n"));
    strcat(buf, ("get_train_info <ID>\n"));
    strcat(buf, ("get_departures <statie>\n"));
    strcat(buf, ("get_arrivals <statie>\n"));
    strcat(buf, ("get_delays\n"));
    strcat(buf, ("get_earlys\n"));
    strcat(buf, ("report_delay <ID> <minute>\n"));
    strcat(buf, ("report_early <ID> <minute>\n"));
}
void raspunde(int cl, int idThread, ClientState *state)
{
    while (1)
    {
        char buf[8192]; // mesajul primit de trimis la client
        memset(buf, 0, sizeof(buf));
        int len;

        if ((len = read(cl, buf, sizeof(buf) - 1)) <= 0)
        {
            printf("[Thread %d]\n", idThread);
            perror("Eroare la read() de la client.\n");
            break;
        }
        buf[len] = '\0';
        if (strncmp(buf, "help", 4) == 0)
        {
            help(buf);
        }
        else if (strncmp(buf, "login ", 6) == 0)
        {
            login_user(buf, state);
        }
        else if (strcmp(buf, "logout") == 0)
        {
            if (state->is_logged_in == false)
                strcpy(buf, "Nu esti conectat");
            else
            {
                state->is_logged_in = false;
                strcpy(buf, "te-ai delogat");
            }
        }
        else if (strncmp(buf, "register ", 9) == 0)
        {
            register_user(buf, state);
        }
        else if (strncmp(buf, "get_schedule", 12) == 0)
        {
            if (buf[12] == '\0' || buf[12] == ' ' || buf[12] == '\n')
            {
                if (buf[12] == ' ')
                    get_schedule(cl, buf + 13);
                else
                    get_schedule(cl, "");
            }
            else
            {
                char *temp = "Format invalid, formatul corect este get_schedule <oras>\n<GATA>";
                write(cl, temp, strlen(temp));
            }
            continue;
        }
        else if (strncmp(buf, "get_train_info ", 15) == 0)
        {
            get_train_info(buf);
        }
        else if (strncmp(buf, "get_departures ", 15) == 0)
        {
            get_departures(cl, buf + 15);
            continue;
        }
        else if (strncmp(buf, "get_arrivals ", 13) == 0)
        {
            get_arrivals(cl, buf + 13);
            continue;
        }
        else if (strcmp(buf, "get_delays") == 0)
        {
            get_delays(cl);
            continue;
        }
        else if (strcmp(buf, "get_earlys") == 0)
        {
            get_earlys(cl);
            continue;
        }
        else if (strncmp(buf, "report_delay ", 13) == 0)
        {
            if (state->is_logged_in == true)
            {
                if (state->is_admin == true)
                    report(buf, "intarziere");
                else
                    strcpy(buf, "trebuie sa fii admin ca sa poti face asta");
            }
            else
                strcpy(buf, "trebuie sa fii logat pentru a apela aceasta comanda");
        }
        else if (strncmp(buf, "report_early ", 13) == 0)
        {
            if (state->is_logged_in == true)
            {
                if (state->is_admin == true)
                    report(buf, "early");
                else
                    strcpy(buf, "trebuie sa fii admin ca sa poti face asta");
            }
            else
                strcpy(buf, "trebuie sa fii logat pentru a apela aceasta comanda");
        }
        else if (strncmp(buf, "buy_ticket ", 11) == 0)
        {
            if (state->is_logged_in == true)
                buy_ticket(buf);
            else
                strcpy(buf, "trebuie sa fii logat pentru a apela aceasta comanda");
        }
        else if (strncmp(buf, "exit", 4) == 0)
        {
            strcpy(buf, "exit");
            break;
        }
        else
        {
            strcpy(buf, "Comanda necunoscuta foloseste /help petru o lista de comenzi valabile");
        }

        /* returnam mesajul clientului */
        if (write(cl, &buf, strlen(buf)) <= 0)
        {
            printf("[Thread %d] ", idThread);
            perror("[Thread]Eroare la write() catre client.\n");
        }
        }
}
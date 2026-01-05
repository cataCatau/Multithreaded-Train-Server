#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include "shared.h"
void load_trains_from_xml(const char *filename)
{
    xmlDoc *document;
    xmlNode *root, *first_child, *node;

    document = xmlReadFile(filename, NULL, 0);
    if (document == NULL)
    {
        printf("[SERVER] Eroare: Nu pot deschide %s\n", filename);
        return;
    }

    root = xmlDocGetRootElement(document);
    if (root == NULL)
    {
        printf("[SERVER] Eroare: Fisier XML gol\n");
        xmlFreeDoc(document);
        return;
    }

    first_child = root->children;
    int index = 0;

    for (node = first_child; node != NULL; node = node->next)
    {
        if (node->type == XML_ELEMENT_NODE && xmlStrcmp(node->name, (const xmlChar *)"tren") == 0)
        {
            xmlNode *child = node->children;
            while (child != NULL)
            {
                if (child->type == XML_ELEMENT_NODE)
                {
                    xmlChar *content = xmlNodeListGetString(document, child->xmlChildrenNode, 1);
                    if (content != NULL)
                    {
                        if (xmlStrcmp(child->name, (const xmlChar *)"id") == 0)
                            strcpy(Trenuri[index].id, (char *)content);
                        else if (xmlStrcmp(child->name, (const xmlChar *)"statie_plecare") == 0)
                            strcpy(Trenuri[index].statie_plecare, (char *)content);
                        else if (xmlStrcmp(child->name, (const xmlChar *)"statie_destinatie") == 0)
                            strcpy(Trenuri[index].statie_destinatie, (char *)content);
                        else if (xmlStrcmp(child->name, (const xmlChar *)"ora_plecare") == 0)
                            strcpy(Trenuri[index].ora_plecare, (char *)content);
                        else if (xmlStrcmp(child->name, (const xmlChar *)"ora_sosire") == 0)
                            strcpy(Trenuri[index].ora_sosire, (char *)content);
                        else if (xmlStrcmp(child->name, (const xmlChar *)"intarziere") == 0)
                            Trenuri[index].intarziere = atoi((char *)content);
                        else if (xmlStrcmp(child->name, (const xmlChar *)"early") == 0)
                            Trenuri[index].early = atoi((char *)content);
                        else if (xmlStrcmp(child->name, (const xmlChar *)"locuri_libere") == 0)
                            Trenuri[index].locuri_libere = atoi((char *)content);
                        else if (xmlStrcmp(child->name, (const xmlChar *)"ruta") == 0)
                        {
                            xmlNode *nod_ruta = child->children;
                            int k = 0; // Indexul opririi

                            while (nod_ruta != NULL && k < 20)
                            {
                                if (nod_ruta->type == XML_ELEMENT_NODE && xmlStrcmp(nod_ruta->name, (const xmlChar *)"oprire") == 0)
                                {
                                    xmlNode *detaliu = nod_ruta->children;
                                    while (detaliu != NULL)
                                    {
                                        if (detaliu->type == XML_ELEMENT_NODE)
                                        {
                                            xmlChar *info = xmlNodeListGetString(document, detaliu->xmlChildrenNode, 1);
                                            if (info)
                                            {
                                                if (xmlStrcmp(detaliu->name, (const xmlChar *)"nume") == 0)
                                                    strcpy(Trenuri[index].ruta[k].nume_statie, (char *)info);
                                                else if (xmlStrcmp(detaliu->name, (const xmlChar *)"sosire") == 0)
                                                    strcpy(Trenuri[index].ruta[k].ora_sosire, (char *)info);
                                                else if (xmlStrcmp(detaliu->name, (const xmlChar *)"plecare") == 0)
                                                    strcpy(Trenuri[index].ruta[k].ora_plecare, (char *)info);

                                                xmlFree(info);
                                            }
                                        }
                                        detaliu = detaliu->next;
                                    }
                                    k++; // Trecem la urmatoarea oprire
                                }
                                nod_ruta = nod_ruta->next;
                            }
                            Trenuri[index].nr_opriri = k; // Salvam cate opriri am gasit
                        }

                        xmlFree(content);
                    }
                }
                child = child->next;
            }
            index++;
        }
    }

    nr_trenuri = index;
    xmlFreeDoc(document);
    xmlCleanupParser();
    printf("[SERVER] S-au incarcat %d trenuri din XML in Memoria RAM.\n", nr_trenuri);
}
void update_in_xml(const char *filename, const char *camp_de_actualizat, const char *id_cautat, int valoare_noua)
{
    xmlDoc *doc = xmlReadFile(filename, NULL, 0);
    if (doc == NULL)
    {
        printf("[EROARE] Nu pot deschide fisierul XML pentru scriere.\n");
        return;
    }

    xmlNode *root = xmlDocGetRootElement(doc);
    xmlNode *node = root->children;

    int gasit = 0;

    for (; node != NULL; node = node->next)
    {
        if (node->type == XML_ELEMENT_NODE && xmlStrcmp(node->name, (const xmlChar *)"tren") == 0)
        {
            xmlNode *child = node->children;
            int este_trenul_cautat = 0;
            xmlNode *nod_de_actualizat = NULL;

            while (child != NULL)
            {
                if (child->type == XML_ELEMENT_NODE)
                {
                    if (xmlStrcmp(child->name, (const xmlChar *)"id") == 0)
                    {
                        xmlChar *id_xml = xmlNodeListGetString(doc, child->xmlChildrenNode, 1);
                        if (strcmp((char *)id_xml, id_cautat) == 0)
                        {
                            este_trenul_cautat = 1;
                        }
                        xmlFree(id_xml);
                    }
                    if (xmlStrcmp(child->name, (const xmlChar *)camp_de_actualizat) == 0)
                    {
                        nod_de_actualizat = child;
                    }
                }
                child = child->next;
            }

            if (este_trenul_cautat && nod_de_actualizat)
            {
                char buffer_nou[20];
                sprintf(buffer_nou, "%d", valoare_noua);

                xmlNodeSetContent(nod_de_actualizat, (const xmlChar *)buffer_nou);

                gasit = 1;
                if (strcmp("intarziere", camp_de_actualizat) == 0)
                    printf("[SERVER] Am actualizat intarzierea trenului %s in XML la %d min.\n", id_cautat, valoare_noua);
                else if (strcmp("early", camp_de_actualizat) == 0)
                    printf("[SERVER] Am actualizat valoarea early a trenului %s in XML la %d min.\n", id_cautat, valoare_noua);
                break;
            }
        }
    }

    if (gasit)
    {
        xmlSaveFormatFileEnc(filename, doc, "UTF-8", 1);
    }
    else
    {
        printf("[SERVER] Nu am gasit trenul %s pentru a actualiza XML-ul.\n", id_cautat);
    }

    xmlFreeDoc(doc);
}
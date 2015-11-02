/**
 * Webservice for Stereolab. The service will listen on
 * http://HOSTNAME:PORT/Stereolab
 */

#include "kernel.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>


/* Headerfiles für UNIX/Linux */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>


/* Portnummer */
#define PORT 1234

/* Puffer für eingehende Nachrichten */
#define RCVBUFSIZE 1024

struct metaData{
    int _sizeleftImage;
    int _sizerightImage;
    int colums;
    int rows;
    int b;
    int h;
    int tauMax;
    int s;

};



/* Die Funktion gibt den aufgetretenen Fehler aus und
 * beendet die Anwendung. */
static void error_exit(char *error_message) {
    fprintf(stderr, "%s: %s\n", error_message, strerror(errno));

    exit(EXIT_FAILURE);
}




int main()
{
    struct sockaddr_in server, client;
    int sock, fd;

    unsigned int len;


    /* Erzeuge das Socket. */
    sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0)
        error_exit("Fehler beim Anlegen eines Sockets");

    /* Erzeuge die Socketadresse des Servers. */
    memset( &server, 0, sizeof (server));
    /* IPv4-Verbindung */
    server.sin_family = AF_INET;
    /* INADDR_ANY: jede IP-Adresse annehmen */
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    /* Portnummer */
    server.sin_port = htons(PORT);

    /* Erzeuge die Bindung an die Serveradresse
     * (genauer: an einen bestimmten Port). */
    if(bind(sock,(struct sockaddr*)&server, sizeof( server)) < 0)
        error_exit("Kann das Socket nicht \"binden\"");

    /* Teile dem Socket mit, dass Verbindungswünsche
     * von Clients entgegengenommen werden. */
    if(listen(sock, 5) == -1 )
         error_exit("Fehler bei listen");


    /* Bearbeite die Verbindungswünsche von Clients
     * in einer Endlosschleife.
     * Der Aufruf von accept() blockiert so lange,
     * bis ein Client Verbindung aufnimmt. */



    for (;;) {
        printf("Server bereit - wartet auf Anfragen ...\n");

        len = sizeof(client);

        fd = accept(sock, (struct sockaddr*)&client, &len);
        if (fd < 0)
            error_exit("Fehler bei accept");
        printf("Bearbeite den Client mit der Adresse: %s\n",
           inet_ntoa(client.sin_addr));

        // Metadaten empfangen
        metaData temp;
        if((recv(fd, &temp, sizeof(temp),MSG_WAITALL)) != sizeof(temp)) {
            error_exit("Fehler bei recv()");
        }
            printf("Metadaten empfangen!\n", temp.tauMax);



        /* Bild Links empfangen */
        signed char* imageLeft = ( signed char* )malloc(sizeof(char) * temp._sizeleftImage);

        if (fd < 0)
            error_exit("Fehler bei accept");


        if((recv(fd, imageLeft, sizeof(char)*temp._sizeleftImage,MSG_WAITALL)) != temp._sizeleftImage) {
            error_exit("Groesse linkes Bild falsch");
        }
        printf("Linkes Bild empfangen! \n");

        /* Bild rechts empfangen */
        signed char* imageRight = ( signed char* )malloc(sizeof(char) * temp._sizerightImage);

        if (fd < 0)
            error_exit("Fehler bei accept");


        if((recv(fd, imageRight, sizeof(char)*temp._sizerightImage,MSG_WAITALL)) != temp._sizerightImage) {
            error_exit("Groesse rechtes Bild falsch");
        }
        printf("Rechtes Bild empfangen! Berechnung starten ... \n");


        int* profile = (int*) malloc(sizeof(int) * temp._sizeleftImage);
        unsigned char* validate = (unsigned char*) malloc(sizeof(char) * temp._sizeleftImage);

        doCUDACalc(imageLeft, imageRight, temp.rows, temp.colums,temp.tauMax, profile, validate, temp.b, temp.h, 0, 0, temp.s );

        //int i = 0;
        //for (i; i < temp._sizeleftImage;i++)
        //    *(validate+i) = 1;

        if (send(fd, profile, temp._sizeleftImage * sizeof(int), 0) != temp._sizeleftImage * sizeof(int)){
               error_exit("fehler!");
        } else {
            printf("Profil gesendet!\n");
        }

        if (send(fd, validate, temp._sizeleftImage, 0) != temp._sizeleftImage){
               error_exit("fehler!");
        } else {
            printf("Validate gesendet!\n");
        }

        /* Schließe die Verbindung. */

        close(fd);
    }

}


/*int __ns1__CalcImage(struct soap *soap, struct ns1__CalcImageParams *p, struct ns1__CalcImageResult *r)
{
    printf("Called CalcImage with spalten=%i, zeilen=%i, tauMax=%i, b=%i, h=%i, useS=%i, useF=%i, s=%i\n",
		p->columns, p->rows, p->tauMax, p->b,p->h,p->useS,p->useF,p->s);

    // some sanity checks to avoid surprises
    if (p->__sizeleftImage != p->rows * p->columns) {
	fprintf(stderr, "Bad size of array leftImage (%i instead of %i)\n", r->__sizeprofile, p->rows * p->columns);
	return SOAP_ERR;
    }	

    if (p->__sizerightImage != p->rows * p->columns) {
	fprintf(stderr, "Bad size of array rightImage (%i instead of %i)\n", r->__sizeprofile, p->rows * p->columns);
	return SOAP_ERR;
    }	

    // create return message (we assume gsoap will free this later :)    
    r->__sizeprofile = p->rows * p->columns;
    r->profile = (int *)calloc(p->rows * p->columns, sizeof(int));
    r->__sizevalid = p->rows * p->columns;
    r->valid = (bool *)calloc(p->rows * p->columns, sizeof(bool));
            
    // do calculation
    doCUDACalc((signed char*)p->leftImage, (signed char*)p->rightImage, p->rows, p->columns,p->tauMax, r->profile, (unsigned char*)r->valid, p->b, p->h, p->useS, p->useF, p->s);
    
    return SOAP_OK;
}*/


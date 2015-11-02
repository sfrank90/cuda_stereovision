#include <stdlib.h>
#include <jni.h>

//Network related includes:
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <string>
#include <string.h>
#include <cstring>
#include "cuda-jni.h"
#include "server-list.h"
#include <pthread.h>

using namespace std;
//#define DEBUG
#define PORT 9999;
#define HOST "izcip06";

struct metaData {
    int __sizeleftImage;
    int __sizerightImage;
    int columns;
    int rows;
    int b;
    int h;
    int tauMax;
    int s;
};

struct callParams {
    const char *server_address;
    metaData meta;

    char *leftImage;
    char *rightImage;

    bool *valid;
    int *profile;
};
                                

/**
 * Copy an image matrix (byte) from java mem to cuda mem
 */
void copyImageFromJNI(JNIEnv *env, signed char *dest, jobjectArray src, int zeilen, int spalten)
{
	for (int i = 0; i < zeilen; i++) {
//		printf("Getting row %i\n", i);
		jobject zeile = env->GetObjectArrayElement(src, i);
//		printf("Copying row %i\n", i);
		env->GetByteArrayRegion((_jbyteArray*)zeile, 0, spalten, dest+i*spalten);
	}

}

/**
 * Copy a boolean matrix from cuda mem to java mem
 */
void copyBooleanMatrixToJNI(JNIEnv *env, jobjectArray dest, bool *src, int zeilen, int spalten, int start_zeile)
{

    for (int i = 0; i < zeilen; i++) {
#ifdef DEBUG
	printf("V-Zeile %i: ",i);
	for (int j = 0; j < spalten; j++) {
	    printf(" %i",*(src+i*spalten+j));
        }
	printf("\n");
#endif
	jobject zeile = env->GetObjectArrayElement(dest, i+start_zeile);
	env->SetBooleanArrayRegion((_jbooleanArray*)zeile, 0, spalten, (jboolean*)((src+i*spalten)));
    }
}

/**
 * Copy a long matrix from cuda mem to java mem
 */
void copyIntMatrixToJNI(JNIEnv *env, jobjectArray dest, jint *src, int zeilen, int spalten, int start_zeile)
{

    for (int i = 0; i < zeilen; i++) {
#ifdef DEBUG
	printf("P-Zeile %i: ",i);
	for (int j = 0; j < spalten; j++) {
	    printf(" %i",*(src+i*spalten+j));
        }
	printf("\n");
#endif
	jobject zeile = env->GetObjectArrayElement(dest, i+start_zeile);
	env->SetIntArrayRegion((_jintArray*)zeile, 0, spalten, (src+i*spalten));
    }
}

void callServer(struct callParams *params) {
    printf("Calling server %s...\n", params->server_address);

    // create server and send image
    int port = 1234;
    int sock;
    struct sockaddr_in serv_addr;
    struct in_addr ipv4addr;
    struct hostent *server;

    server = gethostbyname(params->server_address);
    if (server == 0)
        printf("Error: no such host - %s\n", params->server_address);
    sock = socket(AF_INET,SOCK_STREAM,0);
    if (sock < 0)
        printf("Error opening socket\n");
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
             (char *)&serv_addr.sin_addr.s_addr,
             server->h_length);
    serv_addr.sin_port = htons(port);

    if (connect(sock, (const sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        printf("Error connecting...\n");
    //send meta data
    if (send(sock, &params->meta, sizeof(params->meta), 0)<0)
        printf("Error sending\n");

    //send left image
    if (send(sock, params->leftImage, params->meta.__sizeleftImage * sizeof(char), 0) < 0)
        printf("Error sending left image\n");

    //send right image
    if (send(sock, params->rightImage, params->meta.__sizerightImage * sizeof(char), 0) < 0)
        printf("Error sending right image\n");

    printf ("Waiting for result images..\n");

    if(recv(sock, params->profile, params->meta.__sizeleftImage*sizeof(int), MSG_WAITALL) < 0)
    {
        printf ("Error receiving profile matrix");
    }

    if(recv(sock, params->valid, params->meta.__sizeleftImage*sizeof(bool), MSG_WAITALL) < 0)
    {
        printf ("Error receiving valid matrix");
    }

    /*struct soap soap;
    soap_init(&soap);

        if (soap_call___ns1__CalcImage(&soap, params->server_address, "", &(params->params), &(params->ret)) != SOAP_OK) {
        soap_print_fault(&soap, stderr);
    } else {

        if (params->ret.__sizevalid == params->params.rows * params->params.columns)
            memcpy(params->valid, params->ret.valid,params->ret.__sizevalid*sizeof(bool));
        else
            fprintf(stderr, "Bad size of valid array: %i\n", params->ret.__sizevalid);

        if (params->ret.__sizeprofile == params->params.rows * params->params.columns)
            memcpy(params->profile, params->ret.profile, params->ret.__sizeprofile*sizeof(int));
        else
            fprintf(stderr, "Bad size of profile array: %i\n", params->ret.__sizeprofile);

    }*/
}

/**
 * This is the entry method. It will be called by Java when StereoLab wants to perform a computation
 *
 */
JNIEXPORT void JNICALL Java_stereolab_CUDADistribDiff_doCalculationNative
  (JNIEnv *env, jclass clazz, jobjectArray lbJ, jobjectArray rbJ, jint b, jint h, jint tauMax,
	jboolean useS, jboolean useF, jint s, jobjectArray profileJ, jobjectArray validJ)
{

	int zeilen = env->GetArrayLength(lbJ);

	jobject zeile1 = env->GetObjectArrayElement(lbJ, 0);
	int spalten = env->GetArrayLength((_jarray*)zeile1);

	printf("Performing distributed CUDA with spalten=%i, zeilen=%i, tauMax=%i, b=%i, h=%i, useS=%i, useF=%i, s=%i\n",
			spalten, zeilen, tauMax, b,h,useS,useF,s);

	signed char *lb = (signed char *)calloc(zeilen*spalten,sizeof(char));
	signed char *rb = (signed char *)calloc(zeilen*spalten,sizeof(char));

	printf("Copying left image from Java to C\n");
	copyImageFromJNI(env, lb, lbJ, zeilen, spalten);
	printf("Copying right image from Java to C\n");
	copyImageFromJNI(env, rb, rbJ, zeilen, spalten);

	printf("Starting %i threads\n", server_count);
	pthread_t tid[server_count]; 
    struct callParams params[server_count];
	for (int i=0; i<server_count; i++) {

	    int start_zeile = (i * zeilen - 1) / server_count - h; // 153
	    int stop_zeile = ((i+1) * zeilen - 1) / server_count  + h;
	    
	    if (start_zeile<0)
		start_zeile = 0;
		
	    if (stop_zeile>=zeilen)
		stop_zeile = zeilen-1;
		
	    int z = stop_zeile - start_zeile;

	    params[i].valid = (bool *)calloc(z*spalten,sizeof(bool));
	    params[i].profile = (int *)calloc(z*spalten,sizeof(int));

	    printf("Configuring params for %i\n", i);

        params[i].leftImage = (char *)lb+start_zeile*spalten;
        params[i].rightImage = (char *)rb+start_zeile*spalten;

        params[i].meta.__sizeleftImage = spalten*z;
        params[i].meta.__sizerightImage = spalten*z;
        params[i].meta.columns = spalten;
        params[i].meta.rows = z;
        params[i].meta.b = b;
        params[i].meta.h = h;
        params[i].meta.tauMax = tauMax;
        params[i].meta.s = s;
        
	    params[i].server_address = servers[i];
	    printf("Starting thread %i\n", i);
            pthread_create(&tid[i], NULL, (void*(*)(void*))callServer, (void*)&params[i]); 
	}

	for (int i=0; i<server_count; i++) {
            printf("Waiting for thread %i to finish\n", i);

	    // waiting for call to finish
	    pthread_join(tid[i], NULL);

	    int start_zeile = (i * zeilen -1) / server_count;
	    int stop_zeile = ((i+1) * zeilen -1) / server_count;
	    
	    if (start_zeile<0)
		start_zeile = 0;
	    
	    if (stop_zeile>=zeilen)
		stop_zeile = zeilen-1;
		
	    int z = stop_zeile - start_zeile;
	    
	    int skip = start_zeile == 0 ? 0 : spalten*h;

            printf("Copying results back to Java\n");
    	    copyBooleanMatrixToJNI(env, validJ, params[i].valid + skip, z, spalten, start_zeile);
	    copyIntMatrixToJNI(env, profileJ, params[i].profile + skip, z, spalten, start_zeile);
	    
	    free(params[i].valid);
	    free(params[i].profile);

	}

	// clean up
        free(lb);
        free(rb);

}



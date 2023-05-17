#include "codec.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

void encrypt_pool(char *s,int key);
void decrypt_pool(char *s,int key);

typedef struct {
    char* data;
    int key;
} ThreadData;


void* encryptThread(void* arg) {
    ThreadData* job = (ThreadData*)arg;
    // encrypt data according to key
    encrypt(job->data, job->key);
    pthread_exit(NULL);
}

void* decryptThread(void* arg) {
    ThreadData* job = (ThreadData*)arg;
    // encrypt data according to key
    decrypt(job->data, job->key);
    pthread_exit(NULL);
}


void separateString(const char* originalString, char** separatedStrings) {
    int originalLength = strlen(originalString);
    int substringLength = originalLength / 8;

    int i;
    for (i = 0; i < 8; i++) {
        strncpy(separatedStrings[i], originalString + i * substringLength, substringLength);
        separatedStrings[i][substringLength] = '\0';
    }
}

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
	    printf("usage: key flag < file \n");
	    return 0;
	}
	printf("!! data more than 1024 char will be ignored !!\n");

    // bool value for encrypt-0 or decrypt-1
    int flag=0;

    if(strcmp(argv[2], "-d") == 0){
        printf("decrypt key\n");
        flag = 1;
    }
    else if(strcmp(argv[2], "-e") == 0){
        printf("encrypt key\n");
        flag = 0;
    }
    else{
        printf("usage: key flag < file \n");
        printf("No such flag, availabe flags: encrpyt '-e', decrypt '-d'\n");
        return 0;
    }

	int key = atoi(argv[1]);
	printf("key is %i \n",key);

	char c;
	int counter = 0;
	int dest_size = 1024;
	char data[dest_size]; 
	

	while ((c = getchar()) != EOF)
	{
	  data[counter] = c;
	  counter++;

	  if (counter == 1024){
        if(flag == 0){
            encrypt_pool(data,key);
			printf("\nencripted data: %s\n",data);
        }
        else if(flag == 1){
            decrypt_pool(data,key);
			printf("\ndecripted data: %s\n",data);
        }
    
		
		counter = 0;
		break;
	  }
	}
	
	if (counter > 0)
	{
		char lastData[counter];
		lastData[0] = '\0';
		strncat(lastData, data, counter);
	    if(flag == 0){
            encrypt_pool(lastData,key);
			printf("\nencripted data:\n %s\n",lastData);
        }
        else if(flag == 1){
            decrypt_pool(lastData,key);
			printf("\ndecripted data:\n %s\n",lastData);
        }
		
	}

	return 0;
}

void encrypt_pool(char *data, int key){
	int numCores = sysconf(_SC_NPROCESSORS_ONLN);
	int inputLength = strlen(data);
	int substringLength = inputLength / numCores;
    int remainingLength = inputLength % numCores;
	// seperate data
	char* separatedStrings[numCores];
    int i;
    for (i = 0; i < numCores; i++) {
		 // add 2 chars to length, 1 if there is division remainder and second for null terminator
        separatedStrings[i] = malloc((substringLength + 2) * sizeof(char));
    }
	// copy from data to substrings
    int startPos = 0;
    for (i = 0; i < numCores; i++) {
        int currentLength = substringLength;
        if (i < remainingLength) {
            currentLength++;
        }

        strncpy(separatedStrings[i], data + startPos, currentLength);
        separatedStrings[i][currentLength] = '\0';

        startPos += currentLength;
    }
	
    pthread_t threadPool[numCores];
    ThreadData threadData[numCores];

    for (i = 0; i < numCores; i++) {
        threadData[i].key = key;
        threadData[i].data = separatedStrings[i];

        pthread_create(&threadPool[i], NULL, encryptThread, (void*)&threadData[i]);
    }
	// wait for all encryptions to end
    for (i = 0; i < numCores; i++) {
        pthread_join(threadPool[i], NULL);
    }
	// combine encrypted substrings
	data[0] = '\0'; // make data an empty string
	for (i = 0; i < numCores; i++) {
        strcat(data, separatedStrings[i]);
		free(separatedStrings[i]);
    }

}


void decrypt_pool(char *data, int key){
	int numCores = sysconf(_SC_NPROCESSORS_ONLN);
	int inputLength = strlen(data);
	int substringLength = inputLength / numCores;
    int remainingLength = inputLength % numCores;
	// seperate data
	char* separatedStrings[numCores];
    int i;
    for (i = 0; i < numCores; i++) {
		 // add 2 chars to length, 1 if there is division remainder and second for null terminator
        separatedStrings[i] = malloc((substringLength + 2) * sizeof(char));
    }
	// copy from data to substrings
    int startPos = 0;
    for (i = 0; i < numCores; i++) {
        int currentLength = substringLength;
        if (i < remainingLength) {
            currentLength++;
        }

        strncpy(separatedStrings[i], data + startPos, currentLength);
        separatedStrings[i][currentLength] = '\0';

        startPos += currentLength;
    }
	
    pthread_t threadPool[numCores];
    ThreadData threadData[numCores];

    for (i = 0; i < numCores; i++) {
        threadData[i].key = key;
        threadData[i].data = separatedStrings[i];

        pthread_create(&threadPool[i], NULL, decryptThread, (void*)&threadData[i]);
    }
	// wait for all encryptions to end
    for (i = 0; i < numCores; i++) {
        pthread_join(threadPool[i], NULL);
    }
	// combine encrypted substrings
	data[0] = '\0'; // make data an empty string
	for (i = 0; i < numCores; i++) {
        strcat(data, separatedStrings[i]);
		free(separatedStrings[i]);
    }

}


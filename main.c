#include "codec.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void encrypt_pool(char *s,int key);
void decrypt_pool(char *s,int key);


int main(int argc, char *argv[])
{
	if (argc != 3)
	{
	    printf("usage: key flag < file \n");
	    printf("!! data more than 1024 char will be ignored !!\n");
	    return 0;
	}

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
        }
        else{
            decrypt_pool(data,key);
        }
		printf("encripted data: %s\n",data);
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
		    encrypt(data,key);
        }
        else{
            decrypt_pool(data,key);
        }

		printf("encripted data:\n %s\n",lastData);
	}

	return 0;
}

void encrypt_pool(char *s,int key){
    printf("i was ehr\n");
    encrypt(s,key);
}

void decrypt_pool(char *s,int key){
    decrypt(s,key);
}
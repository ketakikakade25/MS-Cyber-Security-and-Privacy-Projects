//Adhithya Sivanesh(as3423), Arman Gupta(ag986), Ketaki Vitthal Kakade(kk524)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
void main(int argc, char *argv[])
{   
    FILE *result;
    result=fopen("string1.txt","w");
    EVP_MD_CTX context;
    const EVP_MD *algo;
    int i=0,d,k1=0,k2=0,k3=0,k4=0;
    unsigned char num1[225], num2[225], num3[225], num4[225], temp[5]=""; 
    unsigned char given_md[]="acc078c68821d92947bbc24ed5c62024";
    static unsigned char hex_string[16];
    unsigned char digest_result[EVP_MAX_MD_SIZE]; 
    int digest_length;
    OpenSSL_add_all_digests();
    if(!argv[1]) {
        printf("This how you will have to run the code: %s digestname\n", argv[0]);
        exit(1);
    }
    algo = EVP_get_digestbyname(argv[1]);
    if(!algo) {
        printf("Couldn't recognize message digest %s\n", argv[1]);
        exit(1);
    }
    printf("\nBrute Force in progress. Please wait for the program to end....");
    for(d = 32; d <= 255; d++)
       {
           num2[i]=num3[i]=num4[i]=num1[i]=d;
           i++;
       }
    fprintf(result,"Hex     ");
    fprintf(result,": String\n");
    fprintf(result,"--------");
    fprintf(result,"  ------\n\n");
    printf("\n");
    for(k1=0;k1<224;k1++)
    {
        for(k2=0;k2<224;k2++)
        {
            for(k3=0;k3<224;k3++)
            {
                for(k4=0;k4<224;k4++)
                {
                    snprintf(temp,sizeof temp,"%c%c%c%c",num1[k1],num2[k2],num3[k3],num4[k4]);

				    EVP_MD_CTX_init(&context);
				    EVP_DigestInit_ex(&context, algo, NULL);
				    EVP_DigestUpdate(&context, temp, strlen(temp));
				    EVP_DigestFinal_ex(&context, digest_result, &digest_length);
				    EVP_MD_CTX_cleanup(&context);
					
				    for(i = 0; i < digest_length; i++) 
				    {
				 	sprintf(&hex_string[i*2],"%02x", digest_result[i]);					
				    }

				    if(strcmp(hex_string,given_md)==0)
				    {
					for(i=0;i<4;i++)	
					{
						fprintf(result,"%02x",temp[i]);
					}
					fprintf(result,": %s",temp);
		    		   }
                }
            } 
        }
    }
    printf("\nThe output has been stored in the file attached along with this program.\n");
}



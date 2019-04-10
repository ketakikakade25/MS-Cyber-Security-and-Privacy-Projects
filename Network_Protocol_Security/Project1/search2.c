//Adhithya Sivanesh(as3423), Arman Gupta(ag986), Ketaki Vitthal Kakade(kk524)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
void main(int argc, char *argv[])
{   
    FILE *result;
    result=fopen("string2.txt","w+");
    EVP_MD_CTX given_context, compare_context;
    const EVP_MD *algo;
    int i=0,j=0,l=0,d,k1=0,k2=0,k3=0,k4=0;
    unsigned char num1[225], num2[225], num3[225], num4[225], temp[5]=""; 
    unsigned char given_string[5]="harD"; 
    static unsigned char hex_string[16];
    static unsigned char compare_string[16];
    unsigned char given_digest[EVP_MAX_MD_SIZE], digest_result[EVP_MAX_MD_SIZE]; 

    int givendigest_length,digest_length;
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
    EVP_MD_CTX_init(&given_context);
    EVP_DigestInit_ex(&given_context, algo, NULL);
    EVP_DigestUpdate(&given_context, given_string, strlen(given_string));
    EVP_DigestFinal_ex(&given_context, given_digest, &givendigest_length);
    EVP_MD_CTX_cleanup(&given_context);
    for(i = 0; i < givendigest_length; i++) 
    {
        sprintf(&hex_string[i*2],"%02x", given_digest[i]);  
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

                    EVP_MD_CTX_init(&compare_context);
                    EVP_DigestInit_ex(&compare_context, algo, NULL);
                    EVP_DigestUpdate(&compare_context, temp, strlen(temp));
                    EVP_DigestFinal_ex(&compare_context, digest_result, &digest_length);
                    EVP_MD_CTX_cleanup(&compare_context);

                    for(j = 0; j<digest_length; j++) 
                    {

                       sprintf(&compare_string[j*2],"%02x", digest_result[j]);                  
                    }
 	 	    if(strncmp(hex_string,compare_string,4)==0)
		    {
 		            for(i=0;i<4;i++)    
		            {

		                fprintf(result,"%02x",temp[i]);
				
		            }
		            fprintf(result,": %s\n",temp);
		    }
                   
                }
            } 
        }
    }
    printf("\nThe output has been stored in the file attached along with this program.\n");
}



#include <stdio.h>
#include <string.h>
#include "decode.h"
#include "types.h"
#include "common.h"

Status open_decode_files(DecodeInfo *decInfo)
{
    // Stego Image file
    decInfo->fptr_stego = fopen(decInfo->stego_fname, "rb");
    // Do Error handling
    if (decInfo->fptr_stego == NULL)
    {
    	perror("fopen"); // It will show any error in the fopen
    	fprintf(stderr, "ERROR: Unable to open file %s\n",decInfo->stego_fname );

    	return e_failure;
    }

    // // Secret file
    // encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // // Do Error handling
    // if (encInfo->fptr_secret == NULL)
    // {
    // 	perror("fopen");
    // 	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

    // 	return e_failure;
    // }


    // No failure return e_success
    return e_success;
}

Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{

    // check stego.bmp
    if(argv[2] == NULL)
    {
        printf(".bmp  file is not passed");
        return e_failure;
    }
    if((strstr(argv[2],".bmp")) == NULL)
    {
        printf("Invalid image file name");
        return e_failure;
    }
    decInfo->stego_fname = argv[2];
    
    if(argv[3] == NULL)
    {
        decInfo->dest_fname = "Output";
    }
    else
    {
        decInfo->dest_fname =  argv[3];
    }

    return e_success;
    
}

Status do_decoding(DecodeInfo *decInfo)
{
    /*
    1.open stego file.
    2.call skip_bmp_header(decInfo->fptr_stego)
        check using ftell if the offset is present at the 54
    3.declare one empty array --> char magic_array[50];
    4. decode_magic_string(decInfo->fptr_stego,magic_string)
    5. read magic_string from user
    6.compare user enterd magic_string and decoded magic_string is same?
       no-> err: Invalid magic string
          return failure
    int ext_size;
    7.decode_extn_size(decInfo->fptr_stego,&extn_size)
    char_extn[10];
    8.decode_extn(decInfo->fptr_stego,extn,ext_size)
    9.concat both the output file name with decode extn
    12.open ouput file
    int file_size;
    13.decode_sec_file_size(decInfo->fptr_stego,&file_size);
    14.decode_sec_data(decInfo->fptr_stego,decInfo->fptr_dest,file_size);
    15.close files
    16.return success
    */

    int ret = open_decode_files(decInfo);
    if(ret == e_failure)
    {
        printf("Open file failed");
        return e_failure;
    }
    ret = skip_bmp_header(decInfo->fptr_stego);
    if(ret == e_failure)
    return e_failure;

    char magic_string[50];
    ret = decode_magic_string(decInfo->fptr_stego,magic_string);
    if(ret == e_failure)
    return e_failure;

    char user_magic_string[30];
    scanf("%s",user_magic_string);


    if(strcmp(user_magic_string,magic_string) != 0)
    {
       printf("Invalid magic string");
    }

    int extn_size;
    ret = decode_extn_size(decInfo->fptr_stego,&extn_size);
    if(ret == e_failure)
    return e_failure;

    char extn[10];
    ret = decode_extn(decInfo->fptr_stego,extn,extn_size);
    if(ret == e_failure)
    return e_failure;

    strcpy(decInfo->output_fname,decInfo->dest_fname);
    strcat(decInfo->output_fname,extn);

    decInfo->fptr_dest = fopen(decInfo->output_fname, "w");
    if (decInfo->fptr_dest == NULL)
    {
        perror("fopen");
        return e_failure;
    }

    printf("Concatenation done\n");

    int file_size;
    ret = decode_sec_file_size(decInfo->fptr_stego,&file_size);
    if(ret == e_failure)
    return e_failure;

    ret = decode_sec_data(decInfo->fptr_stego,decInfo->fptr_dest,file_size);
    if(ret == e_failure)
    return e_failure;

    // fclose(decInfo->fptr_stego);
    // fclose(decInfo->fptr_dest);
    // fclose(decInfo->output_fname);


    return e_success;
}
Status skip_bmp_header(FILE *fptr_stego)
{
    fseek(fptr_stego, 54, SEEK_SET);   

    printf("Current offset = %ld\n", ftell(fptr_stego)); 

    return e_success;
}

Status decode_magic_string(FILE *fptr_stego,char *magic_string)
{
    /*
    for(int i = 0;i < 2;i++)
    {
    1.read 8 bytes from buffer from stego file
    char temp[8];
    2.magic_string[i] = call lsb_to_byte(buffer) // collect byte by byte and store it in the magic string
    }*/
    char temp_buffer[8];

    for (int i = 0; i < 2; i++)
    {
        fread(temp_buffer, 8, 1, fptr_stego);
        magic_string[i] = lsb_to_byte(temp_buffer);
    }

    magic_string[2] = '\0';

    printf("Decoded magic string: %s\n", magic_string);

    printf("Decode_magic_string is Done");

    return e_success;
}
char lsb_to_byte(char *buffer) //buffer = 8bytes(ie. 1 byte= 8 bit)
{
    /*
    run a loop for 8 time 
        get lsb of each index of buffer and store to ith position of data varaiable


    ret data*/

    char data = 0;

    for (int i = 0; i < 8; i++)
    {
        char lsb = buffer[i] & 1;
        data = data | (lsb << (7 - i));
    }

    return data;
}

int lsb_to_size(char *buffer) //buffer =  32bytes(ie. 4byte = 32bits)
{
    int data = 0;

    for (int i = 0; i < 32; i++)
    {
        int lsb = buffer[i] & 1;
        data = data | (lsb << (31 - i));
    }

    return data;
}
Status decode_extn_size(FILE *stego,int *extn_size)
{
    /*
    1.read 32 bytes buffer
    2.*extn_size = lsb_to_size(buffer) //  extn_size is the pointer bcoz we are taking the address of it and modifying.
    */
   char temp_buffer[32];
   fread(temp_buffer,32,1,stego);
   *extn_size = lsb_to_size(temp_buffer);

   printf("Extn_size = %d\n",*extn_size);

   printf("Decode_extn_size done\n");

   return e_success;


    
}
Status decode_extn(FILE *stego,char *extn,int extn_size)
{
    /*
    run a loop for 0 to extn_size - 1 
    1.read 8 byte temp buffer;
    2.extn[i]=lsb_to_byte(temp_buffer);
    */
    char buffer[8];

    for (int i = 0; i < extn_size; i++)
    {
        fread(buffer, 8, 1, stego);
        extn[i] = lsb_to_byte(buffer);
    }

    extn[extn_size] = '\0';

    printf("Decoded extension = %s\n", extn);

    printf("Decode_extn done\n");


    return e_success;
}
Status decode_sec_file_size(FILE *stego,int *file_size)
{
    /*1.read 32 bytes buffer
    2.*fie_size = lsb_to_size(buffer);*/

    char temp_buffer[32];
    fread(temp_buffer,32,1,stego);
    *file_size = lsb_to_size(temp_buffer);

    printf("Decode_extn_size = %d\n",*file_size);

    printf("Decode_extn_size done\n");
    
    return e_success;
    

    
}
Status decode_sec_data(FILE *stego, FILE *dest,int file_size)
{
    /*
    run a loop for file_size times 
    1.read 8 byte temp buffer;
    2.char ch =lsb_to_byte(temp_buffer);
    3. write ch to dest file*/

    for(int i = 0; i < file_size;i++)
    {
        char temp_buffer[8];
        fread(temp_buffer,8,1,stego);
        char ch = lsb_to_byte(temp_buffer);
        fwrite(&ch,1,1,dest);
    }

    printf("Decode_sec_data Done\n");

    return e_success;
    
    
    
}



#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include "common.h"

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}
Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
    	perror("fopen"); // It will show any error in the fopen
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

    	return e_failure;
    }
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    if (encInfo->fptr_secret == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

    	return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

    	return e_failure;
    }

    // No failure return e_success
    return e_success;
}


Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
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
    encInfo->src_image_fname = argv[2];
    if(argv[3] == NULL)
    {
        printf(".txt file not passed");
        return e_failure;
    }
    if(strstr(argv[3],".") == NULL)
    {
        printf("invalid secret file name");
        return e_failure;
    }
    encInfo->secret_fname = argv[3];
    if(argv[4] == NULL)
    {
        encInfo->stego_image_fname = "stego.bmp" ;
    }
    else
    {
        if(strstr(argv[4],".bmp") == NULL)
        {
            printf("invalid stego file name");
            return e_failure;
        }
        encInfo->stego_image_fname = argv[4];
    }

    char *chr = strchr(encInfo->secret_fname, '.');
    strcpy(encInfo->extn_secret_file, chr);
    return e_success;
}

Status do_encoding(EncodeInfo *encInfo)
{
    int ret = open_files(encInfo);
   if(ret == e_failure)
   {
    printf("Open file failed");
    return e_failure;
   }
   ret = check_capacity(encInfo);
   if(ret == e_failure)
   return e_failure;
   ret = copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image);
   if(ret == e_failure)
   return e_failure;
   ret = encode_magic_string(MAGIC_STRING,encInfo);
   if(ret == e_failure)
   return e_failure;
   ret = encode_secret_file_extn_size(strlen(encInfo->extn_secret_file), encInfo);
   if(ret == e_failure)
   return e_failure;
   ret = encode_secret_file_extn(encInfo->extn_secret_file, encInfo);
   if(ret == e_failure)
   return e_failure;
   fseek(encInfo->fptr_secret , 0 ,SEEK_END);
   encInfo->size_secret_file = ftell(encInfo->fptr_secret);
   fseek(encInfo->fptr_secret,0,SEEK_SET);
   ret = encode_secret_file_size(encInfo->size_secret_file, encInfo);
   if(ret == e_failure)
   return e_failure;
   ret = encode_secret_file_data(encInfo);
   if(ret == e_failure)
   return e_failure;
   ret = copy_remaining_img_data(encInfo->fptr_src_image,encInfo->fptr_stego_image);
   if(ret == e_failure)
   return e_failure;
    
    return e_success;
}

Status check_capacity(EncodeInfo *encInfo){
    int magic_len = strlen(MAGIC_STRING);
    int extn_len  = strlen(encInfo->extn_secret_file);
    int image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);
    fseek(encInfo->fptr_src_image,0,SEEK_SET);

    fseek(encInfo->fptr_secret, 0, SEEK_END); //for finding the position or cursor
    encInfo->size_secret_file = ftell(encInfo->fptr_secret);
    rewind(encInfo->fptr_secret);

    int total_bytes = magic_len + sizeof(int) + extn_len + sizeof(int) + encInfo->size_secret_file;  

    int total_bits_required = (total_bytes * 8) + 54;

    printf("Total_bits_required = %d\n",total_bits_required);

    if (total_bits_required <= image_capacity)
    return e_success;
    
    
    return e_failure;
}      

Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    char temp[54];
    fread(temp, 54, 1, fptr_src_image);
    fwrite(temp,54, 1,fptr_dest_image);

    return e_success;
}
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
   for(int i = 0;i < 2;i++)
   {
    char temp[8];
    fread(temp, 8, 1, encInfo->fptr_src_image);
    encode_byte_to_lsb(MAGIC_STRING[i],temp);
    fwrite(temp,8,1,encInfo->fptr_stego_image);

   }
   printf("encode_magic_string done\n");
   return e_success;
}

Status encode_secret_file_extn_size(int file_extn_size, EncodeInfo *encInfo)
{
   char temp[32];
   fread(temp,32,1,encInfo->fptr_src_image);
   encode_size_to_lsb(file_extn_size,temp);
   fwrite(temp,32,1,encInfo->fptr_stego_image);


   printf("encode_secret_file_extn_size done\n");
   return e_success;

   
}
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    for(int i = 0;i < strlen(file_extn);i++)
    {
        char temp[8];
        fread(temp,8,1,encInfo->fptr_src_image);
        encode_byte_to_lsb(file_extn[i],temp);
        fwrite(temp,8,1,encInfo->fptr_stego_image);
    }

    printf("encode_secret_file_extn done\n");
    return e_success;

}
Status encode_secret_file_size(int file_size, EncodeInfo *encInfo)
{
   char temp[32];
   fread(temp,32,1,encInfo->fptr_src_image);
   encode_size_to_lsb(file_size,temp);
   fwrite(temp,32,1,encInfo->fptr_stego_image);


   printf("encode_secret_file_size done\n");
   return e_success;

}
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char ch;
    char temp[8];
    while ((ch = fgetc(encInfo->fptr_secret)) != EOF)
    {
        fread(temp,8,1,encInfo->fptr_src_image);
        encode_byte_to_lsb(ch ,temp);
        fwrite(temp,8,1,encInfo->fptr_stego_image);
    }

    printf("encode_secret_file_data done\n");
    return e_success;

    
}

Status encode_byte_to_lsb(char data, char *image_buffer)
{
    for(int i = 0; i < 8; i++)
    {
        char mask = 1 << (7 - i); 
        char bit = data & mask; 
        image_buffer[i] = image_buffer[i] & 0xFE; 
        bit = (unsigned)bit >> (7 - i); 
        image_buffer[i] = image_buffer[i] | bit;      
    }

    
}

Status encode_size_to_lsb(char data, char *image_buffer)
{
    for (int i = 0; i < 32; i++)
    {
        int mask = 1 << (31 - i);          
        int bit = data & mask;            
        image_buffer[i] = image_buffer[i] & 0xFE;   
        bit = (unsigned)bit >> (31 - i);    
        image_buffer[i] = image_buffer[i] | bit;    
    }
}

Status copy_remaining_img_data(FILE *fptr_src_image, FILE *fptr_dest_image)
{
   char ch;
   while (fread(&ch ,1,1,fptr_src_image) == 1)
   {
    fwrite(&ch ,1,1,fptr_dest_image);
   }
   
   printf("copy_remaining_img_data done\n");
   return e_success;

   

}






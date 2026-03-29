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

/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
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

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
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
//EncodeInfo *encInfo(pass by refrence bcoz we ar modifying the variable)
{
    /*
    1.check argv[2] == NULL
        yes --> print  .bmp file not passed
        return failure
    2.check strstr(argv[2],."bmp") == NULL
        yes --> print invalid image file name
        return failure
    3.encInfo->src_image_fname = argv[2];   //storing in the structure
    4.check argv[3] == NULL
        yes --> print  .txt file not passed
        return failure
    5.check strstr(argv[2],".txt") == NULL
        yes --> print invalid secret file name
        return failure
    6.encInfo->sec_fname = argv[3];
    7.check argv[3] == NULL
    yes --> encInfo->stego_image_fname = "stego.bmp" // stego.bmp type is a character pointer
    no -->  validate and store to encInfo
    8.copy sec file extn to encInfo->sec_fname
    9. return */

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
    /*
    1.open file
        ret = open_files(encInfo) // they given the function already we just need to call the function
        check ret =  failure
            yes-->print open file failure
            return failure
    2.check capacity
        ret = check_capacity(encInfo)
        check ret =  failure
            yes-->print check capacity failure
            return failure
    3.copy_bmp_header(encInfo->fptr_src_image,encInfo->fptr_stego_image)
    4.check encode_magic_string(magic_string,encInfo)
        ret = encode_magic_string(MAGIC STRING,encInfo)
        check ret =  failure
            yes-->print check maguic string failure
            return failure
    5.check encode_secret_file_extn_size(strlen(file_extn_size), EncodeInfo *encInfo)
    6.check encode_secret_file_extn(encinfo->file_extn, EncodeInfo *encInfo);

    */
    
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
   //cal sec_file_size --> make sure to set offset back to first byte
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
        /*
        1.calculate no of bytes needed for encoding from src files
           count = (magic_str_len + extn_size(int) + extn_len + 
           file_size(int) + file_data_len + 54) * 8 + 54 

        2. check count <= src_file_size
        get the src_file_size from the function it is already written */
        
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
    /*
    1.for(int i = 1;i<= 2;i++)
    {
        char temp[8];
        1. read a 8 byte buffer from src file and store to temp array
        2. byte to lsb(magic string[i],temp);
        3. write temp array 8 bytes to stego file
    }
    */
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
    /*
    char temp_buffer[32];
    1.read 32 bytes buffer from src file.
    2.call the function size to lsb(ile_extn_size,temp)
    3. write to temp_buffer to stego file.*/
    
   char temp[32];
   fread(temp,32,1,encInfo->fptr_src_image);
   encode_size_to_lsb(file_extn_size,temp);
   fwrite(temp,32,1,encInfo->fptr_stego_image);


   printf("encode_secret_file_extn_size done\n");
   return e_success;

   
}
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    /*
    1.Run a loop 0 to file_extn_len - 1 times
    {
       1.char temp[8];
       2.read 8 byte of buffer from src file
       3.call byte_to lsb(file_extn[i] , temp)
       4.write 8 bytes temp to stego file */
    
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
    /*
        1. char temp[32];
        2. read 32 bytes of buff from src file
        3. cal size_to_lsb(file_size, temp);
        4. write temp to stego file
    */
   char temp[32];
   fread(temp,32,1,encInfo->fptr_src_image);
   encode_size_to_lsb(file_size,temp);
   fwrite(temp,32,1,encInfo->fptr_stego_image);


   printf("encode_secret_file_size done\n");
   return e_success;

}
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    /*
        1. run a loop until sec_file reaching EOF
        {
            1. read a ch from src file
            2. read 8 bytes buff from src file
            3.call byte_to_lsb(ch, temp)
            4.write temp to stego file
            }*/
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
    /*
        1. run a loop until src file is reaching EOF
        {
            1. read a 1 byte from src file
            2.write 1 byte to dest file
        }
    */
   char ch;
   while (fread(&ch ,1,1,fptr_src_image) == 1)
   {
    fwrite(&ch ,1,1,fptr_dest_image);
   }
   
   printf("copy_remaining_img_data done\n");
   return e_success;

   

}






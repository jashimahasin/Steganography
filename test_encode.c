#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include "decode.h"

int main(int argc,char *argv[]) 
{
    EncodeInfo encInfo;
    DecodeInfo decInfo;
    int ret  = check_operation_type(argv);   


    if(ret == e_unsupported)
    {
        printf("Invalid args:\n");
        return 0;
    }

    if(ret == e_encode)
    {
        ret = read_and_validate_encode_args(argv,&encInfo);  
        if(ret == e_failure)
        {
            printf("Invalid args");
            return 0;
        }
        ret = do_encoding(&encInfo);
        if(ret == e_failure)
        {
            printf("Invalid args");
            return 0;
        }
        else{
            printf("Encoding is successfully done");
            return 0;
        }
    }
    
    if(ret == e_decode)
    {
        printf("decoding started");
        ret = read_and_validate_decode_args(argv,&decInfo);  
        if(ret == e_failure)
        {
            printf("Invalid args");
            return 0;
        }
        ret = do_decoding(&decInfo);
        if(ret == e_failure)
        {
            printf("Invalid args");
        }
        printf("Decoding is successfully done");
    }
    return 0;
}
OperationType check_operation_type(char *argv[])
{
    if(argv[1] == NULL)
        return e_unsupported;

    if(strcmp(argv[1],"-e") == 0)
        return e_encode;

    if(strcmp(argv[1],"-d") == 0)
        return e_decode;

        return e_unsupported;
}


    


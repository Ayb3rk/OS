#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "parser.h"
int main(void){
    char input[255];
    int is_bundle = 1;
    parsed_input result;
    while(1){
        scanf("%[^\n]%*c", input);
        parse(input, is_bundle, &result );
        printf("%s",result.command.bundle_name);
        break;
    }
    return 0;
}
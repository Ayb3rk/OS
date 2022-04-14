#include <iostream>
#include "parser.h"

using namespace std;
int main() {

    int is_creation = 0;

    while(true){
        char *buffer = new char[256];
        parsed_input output = parsed_input();
        cin.getline(buffer, 256);
        int size = strlen(buffer);
        buffer[size] = '\n';
        buffer[size+1] = '\0';
        parse(buffer, is_creation, &output);

        if(output.command.type == QUIT){
            break;
        }

        if(output.command.type == PROCESS_BUNDLE_STOP){
            is_creation = 0;
        }

        if(output.command.type == PROCESS_BUNDLE_CREATE){
            is_creation = 1;
        }

        free(buffer);
    }
    return 0;
}

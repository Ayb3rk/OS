#include "parser.c"
#include <iostream>
#include <vector>
#include "bundle.h"
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

using namespace std;

void pipe_execute(const Bundle& bundle, int* input_pipe, int* output_pipe){
    unsigned process_count = bundle.commands.size();
    if(input_pipe == nullptr && output_pipe != nullptr){ // there is only output pipe
        close(output_pipe[0]); //close reading end of output pipe
        if(bundle.input_flag){ //there is an input file
            while(process_count){
                int fd = open(bundle.getInputFile(), O_RDONLY);
                char** command = bundle.commands[process_count-1];
                int pid = fork();
                if(pid == 0){ //child executes
                    dup2(fd, 0);
                    close(fd);
                    dup2(output_pipe[1], 1); //redirect the output to output pipe
                    close(output_pipe[1]);
                    execvp(command[0], command);
                }
                process_count--;
                //child never gets here
                //parent continues
            }
            while(wait(nullptr) > 0);

        }
        else{ //just output pipe
            //just fork and execute the processes
            while(process_count){
                char** command = bundle.commands[process_count-1];
                int pid = fork();
                if(pid == 0){ //child executes
                    dup2(output_pipe[1], 1); //redirect the output to output pipe
                    close(output_pipe[1]);
                    execvp(command[0], command);
                }
                process_count--;
                //child never gets here
                //parent continues
            }
            while(wait(nullptr) > 0);
        }
    }
    else if(output_pipe == nullptr && input_pipe != nullptr){ //this is the last bundle to execute
        close(input_pipe[1]); //close writing end of input file
        if(bundle.output_flag){ //there is an output file
            int fd = open(bundle.getOutputFile(), O_WRONLY | O_CREAT, 0666);
            if(process_count > 1){ //repeater needed between input pipe and bundle
                vector<char> buffer;
                while(true){
                    char temp;
                    if(read(input_pipe[0], &temp, 1) < 1){
                        break;
                    }
                    buffer.push_back(temp);
                }
                close(input_pipe[0]); //close read end of the input pipe
                int repeated_pipes[process_count][2];
                for(int i = 0; i < process_count; i++){
                    pipe(repeated_pipes[i]);
                    for(auto const temp: buffer){
                        write(repeated_pipes[i][1], &temp, 1);
                    }
                    close(repeated_pipes[i][1]); //close write end of the pipe
                }

                for(int i = 0; i < process_count; i++){
                    int pid = fork();
                    if(pid == 0){ //child
                        dup2(fd, 1);
                        dup2(repeated_pipes[i][0], 0); //redirect
                        for(int j = 0; j < process_count; j++){
                            close(repeated_pipes[j][0]);
                        }
                        execvp(bundle.commands[i][0], bundle.commands[i]);
                    }
                }
                for(int j = 0; j < process_count; j++){
                    close(repeated_pipes[j][0]); //close unused pipes
                }
                while(wait(nullptr) > 0);

            }
            else{ //no need to repeat the input
                char** command = bundle.commands[0];
                int pid = fork();
                if(pid == 0){ //child executes
                    dup2(fd, 1); //redirect the output to output file
                    dup2(input_pipe[0], 0); //redirect the input from input pipe
                    close(input_pipe[0]);
                    execvp(command[0], command);
                }
                //child never gets here
                //parent continues
                wait(nullptr);
                close(fd);
            }
        }
        else{ //there is neither output file nor output pipe, just input pipe
            if(process_count > 1){
                vector<char> buffer;
                while(true){
                    char temp;
                    if(read(input_pipe[0], &temp, 1) < 1){
                        break;
                    }
                    buffer.push_back(temp);
                }
                close(input_pipe[0]);
                vector<int*> repeated_pipes;
                for(int i = 0; i < process_count; i++){
                    int *new_pipe = new int[2];
                    pipe(new_pipe);
                    repeated_pipes.push_back(new_pipe);
                    for(auto const temp: buffer){
                        write(repeated_pipes[i][1], &temp, 1);
                    }
                    close(repeated_pipes[i][1]); //close writing end of the pipe
                }

                for(int i = 0; i < process_count; i++){
                    int pid = fork();
                    if(pid == 0){ //child
                        dup2(repeated_pipes[i][0], 0); //redirect
                        for(int j = 0; j < process_count; j++){
                            close(repeated_pipes[j][0]); //close unused pipes
                        }
                        execvp(bundle.commands[i][0], bundle.commands[i]);
                    }
                }
                for(int i = 0; i < process_count; i++){
                    close(repeated_pipes[i][0]);
                }
                while(wait(nullptr) > 0);

            }
            else{
                char** command = bundle.commands[0];
                int pid = fork();
                if(pid == 0){ //child executes
                    dup2(input_pipe[0], 0); //redirect the input from input pipe
                    close(input_pipe[0]);
                    execvp(command[0], command);
                }
                //child never gets here
                //parent continues
                while(wait(nullptr) > 0);
            }

        }
    }
    else{ //both input pipe and output pipe
        close(output_pipe[0]); //close reading end of output file
        close(input_pipe[1]); //close writing end of input file
        if(process_count > 1){ //repeater needed between input pipe and bundle
            vector<char> buffer;
            while(true){
                char temp;
                if(read(input_pipe[0], &temp, 1) < 1){
                    break;
                }
                buffer.push_back(temp);
            }
            close(input_pipe[0]);
            vector<int*> repeated_pipes;
            for(int i = 0; i < process_count; i++){
                int *new_pipe = new int[2];
                pipe(new_pipe);
                repeated_pipes.push_back(new_pipe);
                for(auto const temp: buffer){
                    write(repeated_pipes[i][1], &temp, 1);
                }
                close(repeated_pipes[i][1]); //close writing end of the pipe
            }

            for(int i = 0; i < process_count; i++){
                int pid = fork();
                if(pid == 0){ //child
                    dup2(repeated_pipes[i][0], 0); //redirect
                    dup2(output_pipe[1], 1);
                    close(output_pipe[1]);
                    for(int j = 0; j < process_count; j++){
                        close(repeated_pipes[j][0]); //close unused pipes
                    }
                    execvp(bundle.commands[i][0], bundle.commands[i]);
                }
            }
            for(int i = 0; i < process_count; i++){
                close(repeated_pipes[i][0]);
            }
            while(wait(nullptr) > 0);
        }
        else{ //no need to repeat the input
            char** command = bundle.commands[0];
            int pid = fork();
            if(pid == 0){ //child executes

                dup2(input_pipe[0], 0); //redirect
                dup2(output_pipe[1], 1); //redirect the output to output pipe
                close(input_pipe[0]);
                close(output_pipe[1]);
                execvp(command[0], command);
            }
            //child never gets here
            //parent continues
            while (wait(nullptr) > 0);
        }
    }
}

void execute(const vector<Bundle>& bundles_to_execute){
    unsigned bundle_count = bundles_to_execute.size();
    if(bundle_count == 1){ //no pipe
        vector<char**> commands_to_execute = bundles_to_execute[0].getCommands();
        unsigned wait_holder = commands_to_execute.size();
        unsigned command_count = commands_to_execute.size();
        if(bundles_to_execute[0].input_flag == 0 && bundles_to_execute[0].output_flag == 0){ //no redirection
            while(command_count){
                char** command = commands_to_execute[command_count-1];
                int pid = fork();
                if(pid == 0){ //child executes
                    execvp(command[0], command);
                }
                command_count--;
                //child never gets here
                //parent continues
            }
            while(wait(nullptr) > 0);
        }
        if(bundles_to_execute[0].input_flag && bundles_to_execute[0].output_flag){
            int write_file = open(bundles_to_execute[0].getOutputFile(), O_WRONLY | O_CREAT , 0666);
            while(command_count){
                int read_file = open(bundles_to_execute[0].getInputFile(), O_RDONLY );
                char** command = commands_to_execute[command_count-1];
                int pid = fork();
                if(pid == 0){ //child executes
                    dup2(read_file, 0);
                    close(read_file);
                    dup2(write_file, 1);
                    execvp(command[0], command);
                }
                command_count--;
                //child never gets here
                //parent continues
            }
            while(wait(nullptr) > 0);
            close(write_file);
        }
        if(bundles_to_execute[0].input_flag && !bundles_to_execute[0].output_flag){ //there is input redirection
            
            while(command_count){
                int fd = open(bundles_to_execute[0].getInputFile(), O_RDONLY);
                char** command = commands_to_execute[command_count-1];
                int pid = fork();
                if(pid == 0){ //child executes
                    dup2(fd, 0);
                    close(fd);
                    execvp(command[0], command);
                }
                command_count--;
                //child never gets here
                //parent continues
            }
            while(wait(nullptr) > 0);
            
        }
        if(bundles_to_execute[0].output_flag && !bundles_to_execute[0].input_flag){ //there is output redirection
            int fd = open(bundles_to_execute[0].getOutputFile(), O_WRONLY | O_CREAT , 0666);
            while(command_count){
                char** command = commands_to_execute[command_count-1];
                int pid = fork();
                if(pid == 0){ //child executes
                    dup2(fd, 1);
                    execvp(command[0], command);
                }
                command_count--;
                //child never gets here
                //parent continues
            }
            while(wait(nullptr) > 0);
            close(fd);
        }
    }
    else{ //pipe will be implemented
        vector<int*> pipes;
        unsigned count = bundle_count;
        int index = 0; //starting bundle
        for(int i = 0; i < bundle_count-1; i++){ //create the necessary pipes
            int *new_pipe = new int[2];
            pipe(new_pipe);
            pipes.push_back(new_pipe);
        }

        while(count){
            int pid = fork();
            if(pid == 0){//child
                if(index == 0){ //first bundle has no input pipe
                    for(int i = 0; i < bundle_count-1; i++){
                        if(i != index){
                            close(pipes[i][0]); //close unused pipes for this child
                            close(pipes[i][1]);
                        }
                    }
                    pipe_execute(bundles_to_execute[index], nullptr, pipes[index]);
                    exit(0);
                }
                else if(index == bundle_count-1){ //last bundle has no output pipe
                    for(int i = 0; i < bundle_count-1; i++){
                        if(i != index-1){
                            close(pipes[i][0]); //close unused pipes for this child
                            close(pipes[i][1]);
                        }
                    }
                    pipe_execute(bundles_to_execute[index], pipes[index-1], nullptr);
                    exit(0);
                }
                else{
                    for(int i = 0; i < bundle_count-1; i++){
                        if(i != index && i != index-1){
                            close(pipes[i][0]); //close unused pipes for this child
                            close(pipes[i][1]);
                        }
                    }
                    pipe_execute(bundles_to_execute[index], pipes[index-1], pipes[index]);
                    exit(0);
                }

            }
            count--;
            index++;
        }
        for(auto & pipe : pipes){
            close(pipe[0]);
            close(pipe[1]);
        }
        while(wait(nullptr) > 0);
    }
}
int main() {

    vector<Bundle> bundles; //all bundles created inserted here

    int is_creation = 0;

    while(true){
        char *buffer = new char[256];
        parsed_input output = parsed_input();
        cin.getline(buffer, 256);
        unsigned size = strlen(buffer);
        buffer[size] = '\n';
        buffer[size+1] = '\0';
        parse(buffer, is_creation, &output);

        if(output.command.type == QUIT){
            break;
        }

        else if(output.command.type == PROCESS_BUNDLE_STOP){
            is_creation = 0;
        }

        else if(output.command.type == PROCESS_BUNDLE_EXECUTION){
            int bundle_count = output.command.bundle_count;
            for(int i = 0; i < bundle_count; i++){
                if(output.command.bundles[i].input){ //if there is an input file, assign it
                    for(auto & bundle : bundles){ //find the right bundle to set the file
                        if(bundle.name == output.command.bundles[i].name){
                            bundle.setInputFile(output.command.bundles[i].input);
                            bundle.input_flag = 1;
                        }
                    }
                }
                if(output.command.bundles[i].output){ //if there is an output file, assign it
                    for(auto & bundle : bundles){ //find the right bundle to set the file
                        if(bundle.name == output.command.bundles[i].name){
                            bundle.setOutputFile(output.command.bundles[i].output);
                            bundle.output_flag = 1;
                        }
                    }
                }
            }
            vector<Bundle> bundles_to_execute;
            bundle_execution* output_bundles = output.command.bundles;
            for(int i = 0; i < bundle_count; i++){
                for(auto const& bundle: bundles){
                    if(output_bundles[i].name == bundle.name){
                        bundles_to_execute.push_back(bundle);

                    }
                }
            }
            execute(bundles_to_execute);
            for(int i = 0; i < bundles_to_execute.size(); i++){
                for(int j = 0; j < bundles.size(); j++){
                    if(bundles_to_execute[i].name == bundles[i].name){
                        bundles.erase(bundles.begin()+i);
                    }
                }
            }
        }

        else if(output.command.type == PROCESS_BUNDLE_CREATE){
            is_creation = 1; //creation has started
            Bundle new_bundle = Bundle(output.command.bundle_name);
            bundles.push_back(new_bundle); //push the new bundle to the list
        }

        else{ //if there is no command type, shell commands are given
            char** command = output.argv;
            bundles[bundles.size()-1].commands.push_back(command); //command must be given to last bundle that is created
        }


        free(buffer);
    }
    return 0;
}


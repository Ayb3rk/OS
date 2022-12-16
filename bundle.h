//
// Created by Ayberk Gökmen on 9.04.2022.
//
#include <string>
#include <vector>


#ifndef UNTITLED_BUNDLE_H
#define UNTITLED_BUNDLE_H

using namespace std;
class Bundle {

    public:
        string name;
        vector<char**> commands;
        const char *input_file;
        int input_flag = 0;
        const char *output_file;
        int output_flag = 0;


    Bundle(const string &name);

    const string &getName() const;

    void setName(const string &name);

    const vector<char **> & getCommands() const;

    void setCommands(const vector<string> &commands);

    const char *const getInputFile() const;

    const char *const getOutputFile() const;

    void setOutputFile(char *&outputFile);


    void setInputFile(char *&inputFile);
};

const string &Bundle::getName() const {
    return name;
}

void Bundle::setName(const string &name) {
    Bundle::name = name;
}

const vector<char**> &Bundle::getCommands() const {
    return commands;
}


const char *const Bundle::getInputFile() const {
    return input_file;
}



void Bundle::setInputFile(char* &inputFile) {
    input_file = inputFile;
}

const char *const Bundle::getOutputFile() const {
    return output_file;
}

void Bundle::setOutputFile(char* &outputFile) {
    output_file = outputFile;
}

Bundle::Bundle(const string &name) : name(name) {}




#endif //UNTITLED_BUNDLE_H

#include <fstream>
#include <iostream>
#include <sstream>

#include "brainfuck.h"

bool ParseArgs(int argc, char** argv, std::string& in_path, std::string& out_path);

int main(int argc, char** argv) {
  std::string in_path, out_path;
  if(!ParseArgs(argc, argv, in_path, out_path)){
    exit(0);
  }
  std::ifstream file{in_path};
  if(!file.is_open()){
    std::cout<<"Unable to open the brainf**k program for reading.\n";
    exit(0);
  }
  // Read the file to a string, and Preprocess the program.
  std::stringstream buffer;
  buffer << file.rdbuf();
  std::string program = buffer.str();
  auto res = bf::Preprocess(program);
  // If the out path was specified, compile the program to that path.
  // Else interpret it.
  if(out_path != ""){
    bf::Compile(res, out_path);
  }else{
    bf::Interpret(res, 30000);
  }
}


bool ParseArgs(int argc, char** argv, std::string& in_path, std::string& out_path) {
  if(argc < 2){
    std::cout<<"There was no input file specified.\n";
    return false;
  }
  in_path = std::string{argv[1]};
  if(argc == 3) {
    out_path = std::string{argv[2]};
  } 
  return true;
}

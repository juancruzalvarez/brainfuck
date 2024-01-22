#ifndef BRAINFUCK_H_
#define BRAINFUCK_H_

#include <vector>
#include <string>

namespace bf{ 

enum class Operator {
  IncrementPtr,        // <> gets stored in the same Operator, 
                       // because we can just use a positive or negative value
                       // depending on if we are moving left or righ.
  IncrementValue,      // +- IDEM as IncrementPtr
  JumpIfZero,          // [
  JumpIfNotZero,       // ]
  Read,                // .
  Write,               // ,
};


// +++   -> Operation{IncrementValue, 3}
// ----+ -> Operation{IncrementValue, -3}
// >>><  -> Operation{IncrementPtr, 2}
// for [ and ] the value will the intruction we will jump to.
// the first instruction will be at 0.
struct Operation {
  Operator op;
  int value;
};

// Preprocess transforms a string with the program as characters, to a vector of Operation.
// This makes the process of intepreting and compiling the program easier.
std::vector<Operation> Preprocess(const std::string& program);

// Intepretes the program passed. buffer_size specifies the size of the internal buffer used for the program.
void Interpret(const std::vector<Operation>& operations, int buffer_size);

// Compile compiles the program to asm. Stores the compiled code to the path specified in out_file_path
void Compile(const std::vector<Operation>& program, const std::string& out_file_path);
};


#endif

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

std::vector<Operation> Preprocess(const std::string& program);
void Interpret(const std::vector<Operation>& operations, int buffer_size);
void Compile(const std::vector<Operation>& program, const std::string& out_file_path);
};


#endif

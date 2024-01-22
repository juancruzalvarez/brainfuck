#include "brainfuck.h"

#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stack>
#include <string>
#include <vector>


struct Conditional{
  bf::Operator kind;
  int jump_position;
};

bool IsIncrementValueChar(char c) { return c == '+' || c == '-';};
bool IsIncrementPtrChar(char c) { return c == '>' || c == '<';};
bool IsValidChar(char c) { return IsIncrementValueChar(c) || IsIncrementPtrChar(c) || c == '.' || c == ',' || c == '[' || c == ']';};

// Transform a string of characters that represent the bf program
// to a vector of Operations that can then be intepreted or compiled.
std::vector<bf::Operation> bf::Preprocess(const std::string &program) {
  std::vector<bf::Operation> ret;
  std::stack<Conditional> conditionals;

  int operation_num = 0;  
  int i = 0; // i is the index to the string
  while(i < program.size()) {
    // Any characters that are not valid bf operators are skipped.
    if(!IsValidChar(program[i])) {
      i++;
      continue;
    }

    if(IsIncrementValueChar(program[i])){
      // read a list of '+' and '-', adding or substracting to value.
      int value = 0;
      while(i < program.size() && IsIncrementValueChar(program[i])) {
        // ascci: + , -
        value += ',' - program[i];
        i++;
      }
      ret.push_back(bf::Operation{bf::Operator::IncrementValue, value});
      operation_num++;
      continue; // We continue here, becouse the while loops already puts i in the next position
    }else if(IsIncrementPtrChar(program[i])) {
      int value = 0;
      // same as with value chars, read a list of < and >.
      while(i < program.size() && IsIncrementPtrChar(program[i])){ 
        // ascci: < = >
        value += program[i] - '=';
        i++;
      }
      ret.push_back(bf::Operation{bf::Operator::IncrementPtr, value});
      operation_num++;
      continue; // We also continue here for the same reason.
    }else if(program[i] == ','){
      ret.push_back(bf::Operation{bf::Operator::Read, 0});
    }else if(program[i] == '.') {
      ret.push_back(bf::Operation{bf::Operator::Write, 0});
    }else if(program[i] == '[') {
      // Add conditional to the stack.
      // We also add an operation. The value is temporal, as it will be set correctly
      // when the conditional is popped.
      conditionals.push({bf::Operator::JumpIfZero, operation_num});
      ret.push_back(bf::Operation{bf::Operator::JumpIfZero, operation_num});
    }else if(program[i] == ']') {
      // Pop from the stack the corresponding '['.
      // Change the temporal value set in the operation for the current operation number.
      // And add an operation with the value of the popped '['.
      Conditional popped = conditionals.top();
      conditionals.pop();
      if(popped.kind != bf::Operator::JumpIfZero) {
        std::cerr<<"Invalid program. Unexpected closing bracket at "<<i<<".\n";
        exit(0);
      }
      ret[popped.jump_position].value = operation_num;
      ret.push_back(bf::Operation{bf::Operator::JumpIfNotZero, popped.jump_position});
    }
    i++;
    operation_num++;
  }

  // After parsing the entire program, there should be no conditionals in the stacks.
  if(!conditionals.empty()) { 
    std::cerr<<"Conditional jumps are not balanced.\n";
    exit(0);
  }

  return ret;
};

void bf::Interpret(const std::vector<Operation> &operations, int buffer_size) {
  // Initalize the buffer and the pointer.
  // We also create a operation_num, that indexes operations for running the current operation.
  int operation_num = 0;
  int ptr = 0;
  uint8_t buffer[buffer_size];
  memset(buffer, 0, buffer_size);
  while(operation_num < operations.size()) {
    auto operation = operations[operation_num];
    switch (operation.op) {
      case bf::Operator::IncrementValue: {
        buffer[ptr] += operation.value; 
        break;
      }
      case bf::Operator::IncrementPtr: {
        ptr += operation.value; 
        // If we increment the ptr to be outside the buffer, go to the other end of it.
        // Maybe should be an error.
        // ?
        if(ptr < 0) ptr += buffer_size;
        if(ptr >= buffer_size) ptr -= buffer_size;
        break;
      }
      case bf::Operator::Read: {
        std::cin>>(&buffer[ptr]);
        break;
      }
      case bf::Operator::Write: {
        std::cout<<(buffer[ptr]);
        break;
      }
      // In both cases of jump the current operation its set to its value.
      // as in the Preprocess step, the operation number stored in each jump operation
      // its set to itself.
      // So after the switch statement is over the operation_number gets increased
      // we are after the jump, which is the expected behaviour.
      case bf::Operator::JumpIfZero: {
        if(buffer[ptr] == 0) operation_num = operation.value;
        break;
      }
      case bf::Operator::JumpIfNotZero: {
        if(buffer[ptr] != 0) operation_num = operation.value;
        break;
      }
    }
    operation_num++;
  }
}

void bf::Compile(const std::vector<bf::Operation>& program, const std::string& out_file_path){
  std::ofstream f{out_file_path};
  // Structure of the asm program.
  // Creates the buffer, and sets the start point of the program.

  f<<"section .data\n";
  f<<"buffer: times 30000 db 0\n";
  f<<"section .text\n";
  f<<"global _start\n";
  f<<"_start:\n";
  int operation_num = 0;
  while(operation_num < program.size()) {
    auto op = program[operation_num];
    switch (op.op) {
      case bf::Operator::IncrementValue: {
        f<<";increment or decrement by value.\n";
        f<<"add [buffer+ebx], byte "<<op.value<<"\n";
        break;
      }
      case bf::Operator::IncrementPtr: {
        f<<";increment or decrement pointer by value.\n";
        f<<"add ebx, "<<op.value<<"\n";
        break;
      }
      case bf::Operator::Read: {
        f<<";read character to buffer.\n";
        f<<"mov ecx, buffer\n";
        f<<"add ecx, ebx\n";
        f<<"push ebx\n";
        f<<"mov eax, 0x3\n";
        f<<"mov ebx, 0x1\n";
        f<<"mov edx, 0x1\n";
        f<<"int 80h\n";
        f<<"pop ebx\n";
        break;
      }
      case bf::Operator::Write: {
        f<<";print character from buffer.\n";
        f<<"mov ecx, buffer\n";
        f<<"add ecx, ebx\n";
        f<<"push ebx\n";
        f<<"mov eax, 0x4\n";
        f<<"mov ebx, 0x1\n";
        f<<"mov edx, 0x1\n";
        f<<"int 80h\n";
        f<<"pop ebx\n";
        break;
      }
      // In the conditional operators, after the comparison and jump instructions are added,
      // there is a label set, with the format:
      // op+op_number:
      // that sets the destination of the jump, for the corresponding conditional.
      case bf::Operator::JumpIfZero: {
        f<<";jump if current value is 0.\n";
        f<<"cmp [buffer+ebx], byte 0\n";
        f<<"je op"<<op.value<<"\n";
        f<<"op"<<operation_num<<":\n";
        break;
      }
      case bf::Operator::JumpIfNotZero: {
        f<<";jump if current value is not 0.\n";
        f<<"cmp [buffer+ebx],byte 0\n";
        f<<"jne op"<<op.value<<"\n";
        f<<"op"<<operation_num<<":\n";
        break;
      }
    }
    operation_num++;
  }
  // call linux syscall for exiting the program.
  f<<";end program\n";
  f<<"mov ebx, 0\n";
  f<<"mov eax, 1\n";
  f<<"int 80h";
  f.close();

}














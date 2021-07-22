#ifndef BATCH_CALCULATOR_PARSER_H
#define BATCH_CALCULATOR_PARSER_H

#include <string>
#include "stack_machine.h"

bool StackMachineParser(const std::string &prog, StackMachine &calc);

#endif//BATCH_CALCULATOR_PARSER_H

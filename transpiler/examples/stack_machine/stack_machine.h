// Copyright 2021 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.


#ifndef STACK_MACHINE_H
#define STACK_MACHINE_H

// Result codes
#define STATUS_HALT                 -1
#define STATUS_OK                    0
#define STATUS_MAX_COMMANDS          1
#define STATUS_EMPTY_STACK           2
#define STATUS_EXCEEDED_MAX_OPERANDS 3
#define STATUS_NOT_ENOUGH_OPERANDS   4
#define STATUS_RAN_OUT_OF_GAS        5
#define STATUS_UNKNOWN_OPCODE        6
#define STATUS_MAX_CALLS             7

// Opcodes
#define OP_CALL  0 // push next instruction address to callstack, jump to argument
#define OP_RET   1 // pop op1 from callstack, continue at op1
#define OP_PUSH  2 // push arg on top of stack
#define OP_POP   3 // pop top of stack
#define OP_J     4 // jump to arg location
#define OP_JZ    5 // pop op1, jump to arg location if op1 == 0
#define OP_JNZ   6 // pop op1, jump to arg location if op1 != 0
#define OP_DUP   7 // duplicate the top of the stack
#define OP_SWAP  8 // swap the two topmost stack elements
#define OP_NOT   9 // consumes the top operand and pushes !op on the stack
#define OP_BNOT 10 // consumes the top operand and pushes a bitwise not

#define BINARY_OP_BASE 11

#define OP_EQ   11 // consumes the top two operands and pushes 1 if they are =, 0 otherwise
#define	OP_ADD  12 // pop op1, pop op2, push op1 + op2
#define	OP_SUB  13 // pop op1, pop op2, push op2 - op1
#define	OP_MUL  14 // pop op1, pop op2, push op1 * op2
#define OP_AND  15 // pop op1, pop op2, push op1 && op2
#define OP_OR   16 // pop op1, pop op2, push op1 || op2
#define OP_BAND 17 // pop op1, pop op2, push (bitwise) op1 and op2
#define OP_BOR  18 // pop op1, pop op2, push (bitwise) op1 or op2
#define OP_BXOR 19 // pop op1, pop op2, push (bitwise) op1 xor op2

#define MAX_OP  20

// Max depth of operand stack
#define MAX_OPD 50
// Maximum number of commands and command-operands (i.e. "push 1" counts as 2 entries)
#define MAX_CMD 50
// Maximum number of nested function call
#define MAX_CALLS 10

struct StackMachine {

	StackMachine(int entry = 0) : step(entry), top(0), call(0), status(STATUS_OK) {}

	int commands[MAX_CMD];
	int step;

	int stack[MAX_OPD];
	int top;

	int callstack[MAX_CALLS];
	int call;

	int status;
};

static bool inline stack_machine_result(const StackMachine &sm, int &res) {
	if (sm.status != STATUS_HALT) {
		return false;
	}
	if (!sm.top) {
		return false;
	}
	res = sm.stack[sm.top - 1];
	return true;
}

// NOTE: The fhe_cc_library bazel command can handle only one file.  Since we
// want to build two FHE targets, one with tick() as the top function, and the
// other with compute() as the top function, we include the implementation in
// the header, then include the header in one source file for each FHE type,
// and annotate the functions in those types with #pragma hls_top.

#ifndef EXCLUDE_STACK_MACHINE_IMPLEMENTATION

#include "stack_machine_switch_impl.h"

#else//defined EXCLUDE_STACK_MACHINE_IMPLEMENTATION

StackMachine stack_machine_tick(const StackMachine &calc);
StackMachine stack_machine_compute(const StackMachine &calc, int max_gas);

#endif//EXCLUDE_STACK_MACHINE_IMPLEMENTATION

#endif//STACK_MACHINE_H

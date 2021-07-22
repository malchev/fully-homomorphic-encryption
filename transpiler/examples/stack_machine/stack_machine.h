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
#define OP_CALL '@' // push next instruction address to callstack, jump to argument
#define OP_RET  '.' // pop op1 from callstack, continue at op1
#define	OP_ADD  '+' // pop op1, pop op2, push op1 + op2
#define	OP_SUB  '-' // pop op1, pop op2, push op2 - op1
#define	OP_MUL  '*' // pop op1, pop op2, push op1 * op2
#define OP_PUSH 'P' // push arg on top of stack
#define OP_POP  'p' // pop top of stack
#define OP_J    'j' // jump to arg location
#define OP_JZ   'Z' // pop op1, jump to arg location if op1 == 0
#define OP_JNZ  'z' // pop op1, jump to arg location if op1 != 0
#define OP_DUP  'd' // duplicate the top of the stack
#define OP_SWAP 'x' // swap the two topmost stack elements
#define OP_EQ   '=' // consumes the top two operands and pushes 1 if they are =, 0 otherwise
#define OP_NOT  '!' // consumes the top operand and pushed !op on the stack
#define OP_AND  'A' // pop op1, pop op2, push op1 && op2
#define OP_OR   'O' // pop op1, pop op2, push op1 || op2

// Max number of commands, counting a final END, not counting operands
#define MAX_GAS 999
// Max depth of operand stack
#define MAX_OPD 999
// Maximum number of commands and command-operands (i.e. "push 1" counts as 2 entries)
#define MAX_CMD 999
// Maximum number of nested function call
#define MAX_CALLS 999
	
struct StackMachine {

	StackMachine() : top(0), call(0), step(0) {}

	int commands[MAX_CMD];
	int stack[MAX_OPD];
	int top;
	int callstack[MAX_CALLS];
	int call;
	int step;

	static int tick(StackMachine &calc, int &result);
	static int compute(StackMachine &calc, int &result);
};

#endif//STACK_MACHINE_H

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

// Max depth of operand stack
#define MAX_OPD 1024
// Maximum number of commands and command-operands (i.e. "push 1" counts as 2 entries)
#define MAX_CMD 1024
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
	static int compute(StackMachine &calc, int max_gas, int &result);
};

#ifndef STACK_MACHINE_NOIMPL

// NOTE: The fhe_cc_library bazel command can handle only one file.  Since we
// want to build two FHE targets, one with tick() as the top function, and the
// other with compute() as the top function, we include the implementation in
// the header, then include the header in one source file for each FHE type,
// and annotate the functions in those types with #pragma hls_top.

#undef DEBUG

#ifdef DEBUG
#include <cstdio>
#define PRINT(...) fprintf(stdout, __VA_ARGS__)
#else
#define PRINT(...) do {} while(false)
#endif

int StackMachine::tick(StackMachine &calc, int &result) {
#ifdef DEBUG
	do {
		printf("%-8d %c:", gas, calc.commands[calc.step]);
		for (int n = 0; n < calc.top; n++) {
			printf(" %-8d", calc.stack[n]);
		}
		printf("\n");
	} while(false);
#endif

	if (calc.step == MAX_CMD) {
		return STATUS_MAX_COMMANDS;
	}
	int cmd = calc.commands[calc.step];
	calc.step++;
	if (cmd == OP_CALL) {
		if (calc.step == MAX_CMD) {
			return STATUS_MAX_COMMANDS;
		}
		if (calc.call == MAX_CALLS) {
			return STATUS_MAX_CALLS;
		}
		int target = calc.commands[calc.step];
		calc.step++;
		calc.callstack[calc.call] = calc.step;
		calc.call++;
		calc.step = target;
	}
	else if (cmd == OP_RET) {
		if (calc.call == 0) {
			if (calc.top == 0) {
				return STATUS_EMPTY_STACK;
			}
			calc.top--;
			result = calc.stack[calc.top];
			return STATUS_HALT;
		}
		else {
			--calc.call;
			calc.step = calc.callstack[calc.call];
		}
	}
	else if (cmd == OP_PUSH) {
		if (calc.step == MAX_CMD) {
			return STATUS_MAX_COMMANDS;
		}
		if (calc.top == MAX_OPD) {
			return STATUS_EXCEEDED_MAX_OPERANDS;
		}
		int val = calc.commands[calc.step];
		calc.step++;
		calc.stack[calc.top] = val;
		calc.top++;
	}
	else if (cmd == OP_POP) {
		if (calc.top == 0) {
			return STATUS_EMPTY_STACK;
		}
		--calc.top;
	}
	else if (cmd == OP_DUP) {
		if (calc.top == 0) {
			return STATUS_EMPTY_STACK;
		}
		int top = calc.stack[calc.top-1];
		calc.stack[calc.top] = top;
		calc.top++;
	}
	else if (cmd == OP_J) {
		if (calc.step == MAX_CMD) {
			return STATUS_MAX_COMMANDS;
		}
		calc.step = calc.commands[calc.step];
	}
	else if (cmd == OP_JZ) {
		if (calc.step == MAX_CMD) {
			return STATUS_MAX_COMMANDS;
		}
		if (calc.top == 0) {
			return STATUS_EMPTY_STACK;
		}
		int target = calc.commands[calc.step];
		calc.step++;
		--calc.top;
		int top = calc.stack[calc.top];
		if (!top) {
			calc.step = target;
		}
	}
	else if (cmd == OP_JNZ) {
		if (calc.step == MAX_CMD) {
			return STATUS_MAX_COMMANDS;
		}
		if (calc.top == 0) {
			return STATUS_EMPTY_STACK;
		}
		int target = calc.commands[calc.step];
		calc.step++;
		--calc.top;
		int top = calc.stack[calc.top];
		if (top) {
			calc.step = target;
		}
	}
	else if (cmd == OP_NOT) {
		if (calc.top < 1) {
			return STATUS_NOT_ENOUGH_OPERANDS;
		}
		--calc.top;
		int op1 = calc.stack[calc.top];
		calc.stack[calc.top] = !op1;
		calc.top++;
	}
	else if (cmd == OP_AND) {
		if (calc.top < 2) {
			return STATUS_NOT_ENOUGH_OPERANDS;
		}
		--calc.top;
		int op1 = calc.stack[calc.top];
		--calc.top;
		int op2 = calc.stack[calc.top];
		calc.stack[calc.top] = op1 && op2;
		calc.top++;
	}
	else if (cmd == OP_OR) {
		if (calc.top < 2) {
			return STATUS_NOT_ENOUGH_OPERANDS;
		}
		--calc.top;
		int op1 = calc.stack[calc.top];
		--calc.top;
		int op2 = calc.stack[calc.top];
		calc.stack[calc.top] = op1 || op2;
		calc.top++;
	}
	else if (cmd == OP_SWAP) {
		if (calc.top < 2) {
			return STATUS_NOT_ENOUGH_OPERANDS;
		}
		--calc.top;
		int ontop = calc.stack[calc.top];
		--calc.top;
		int below = calc.stack[calc.top];
		calc.stack[calc.top] = ontop;
		calc.top++;
		calc.stack[calc.top] = below;
		calc.top++;
	}
	else if (cmd == OP_EQ) {
		if (calc.top < 2) {
			return STATUS_NOT_ENOUGH_OPERANDS;
		}
		--calc.top;
		int op1 = calc.stack[calc.top];
		--calc.top;
		int op2 = calc.stack[calc.top];
		calc.stack[calc.top] = op1 == op2;
		calc.top++;
	}
	else if (cmd == OP_ADD) {
		if (calc.top < 2) {
			return STATUS_NOT_ENOUGH_OPERANDS;
		}
		--calc.top;
		int op1 = calc.stack[calc.top];
		--calc.top;
		int op2 = calc.stack[calc.top];
		calc.stack[calc.top] = op1 + op2;
		calc.top++;
	}
	else if (cmd == OP_SUB) {
		if (calc.top < 2) {
			return STATUS_NOT_ENOUGH_OPERANDS;
		}
		--calc.top;
		int op2 = calc.stack[calc.top];
		--calc.top;
		int op1 = calc.stack[calc.top];
		calc.stack[calc.top] = op1 - op2;
		calc.top++;
	}
	else if (cmd == OP_MUL) {
		if (calc.top < 2) {
			return STATUS_NOT_ENOUGH_OPERANDS;
		}
		--calc.top;
		int op2 = calc.stack[calc.top];
		--calc.top;
		int op1 = calc.stack[calc.top];
		calc.stack[calc.top] = op1 * op2;
		calc.top++;
	}
	else {
		return STATUS_UNKNOWN_OPCODE;
	}

	return STATUS_OK;
}

int StackMachine::compute(StackMachine &calc, int max_gas, int &result) {
	for (int gas = 0; (max_gas >= 0 && gas < max_gas) || (max_gas < 0); gas++) {
		int status = tick(calc, result);
		if (status != STATUS_OK)
			return status;
	}
	return STATUS_RAN_OUT_OF_GAS;
}

#endif//STACK_MACHINE_NOIMPL

#endif//STACK_MACHINE_H

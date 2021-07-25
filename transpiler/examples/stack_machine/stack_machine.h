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
#define MAX_OPD 1024
// Maximum number of commands and command-operands (i.e. "push 1" counts as 2 entries)
#define MAX_CMD 1024
// Maximum number of nested function call
#define MAX_CALLS 999

struct StackMachine;
typedef int (*stack_machine_handler_t)(StackMachine &calc, int &result);
typedef int (*handle_binary_op_t)(int op1, int op2);
	
struct StackMachine {

	StackMachine();

	int commands[MAX_CMD];
	int stack[MAX_OPD];
	int top;
	int callstack[MAX_CALLS];
	int call;
	int step;

	static stack_machine_handler_t dispatch[BINARY_OP_BASE];
	static handle_binary_op_t binary_dispatch[MAX_OP - BINARY_OP_BASE];
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

StackMachine::StackMachine() 
	: top(0), call(0), step(0) {
}

int handle_call(StackMachine &calc, int &result) {
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
	return STATUS_OK;
}

int handle_ret(StackMachine &calc, int &result) {
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
	return STATUS_OK;
}

int handle_push(StackMachine &calc, int &result) {
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
	return STATUS_OK;
}

int handle_pop(StackMachine &calc, int &result) {
	if (calc.top == 0) {
		return STATUS_EMPTY_STACK;
	}
	--calc.top;
	return STATUS_OK;
}

int handle_dup(StackMachine &calc, int &result) {
	if (calc.top == 0) {
		return STATUS_EMPTY_STACK;
	}
	int top = calc.stack[calc.top-1];
	calc.stack[calc.top] = top;
	calc.top++;
	return STATUS_OK;
}

int handle_j(StackMachine &calc, int &result) {
	if (calc.step == MAX_CMD) {
		return STATUS_MAX_COMMANDS;
	}
	calc.step = calc.commands[calc.step];
	return STATUS_OK;
}

int handle_j_cond(StackMachine &calc, int &result, bool want_zero) {
	if (calc.step == MAX_CMD) {
		return STATUS_MAX_COMMANDS;
	}
	if (calc.top == 0) {
		return STATUS_EMPTY_STACK;
	}
	int target = calc.commands[calc.step];
	calc.step++;
	--calc.top;
	bool is_zero = !calc.stack[calc.top];
	if (!(want_zero ^ is_zero)) {
		calc.step = target;
	}
	return STATUS_OK;
}

int handle_jz(StackMachine &calc, int &result) {
	return handle_j_cond(calc, result, true);
}

int handle_jnz(StackMachine &calc, int &result) {
	return handle_j_cond(calc, result, false);
}

int handle_not(StackMachine &calc, int &result) {
	if (calc.top < 1) {
		return STATUS_NOT_ENOUGH_OPERANDS;
	}
	--calc.top;
	int op1 = calc.stack[calc.top];
	calc.stack[calc.top] = !op1;
	calc.top++;
	return STATUS_OK;
}

int handle_bnot(StackMachine &calc, int &result) {
	if (calc.top < 1) {
		return STATUS_NOT_ENOUGH_OPERANDS;
	}
	--calc.top;
	int op1 = calc.stack[calc.top];
	calc.stack[calc.top] = ~op1;
	calc.top++;
	return STATUS_OK;
}

int handle_swap(StackMachine &calc, int &result) {
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
	return STATUS_OK;
}

int do_add(int op1, int op2) { return op1 + op2; }
int do_sub(int op1, int op2) { return op2 - op1; }
int do_mul(int op1, int op2) { return op1 * op2; }
int do_and(int op1, int op2) { return op1 && op2; }
int do_or(int op1, int op2) { return op1 || op2; }
int do_band(int op1, int op2) { return op1 & op2; }
int do_bor(int op1, int op2) { return op1 | op2; }
int do_bxor(int op1, int op2) { return op1 ^ op2; }
int do_eq(int op1, int op2) { return op1 == op2; }

#if 1 // only trivial initializers are supported
handle_binary_op_t StackMachine::binary_dispatch[MAX_OP - BINARY_OP_BASE] = {
	do_eq,
	do_add,
	do_sub,
	do_mul,
	do_and,
	do_or,
	do_band,
	do_bor,
	do_bxor,
};
#else
handle_binary_op_t StackMachine::binary_dispatch[MAX_OP - BINARY_OP_BASE] = {
	[OP_ADD  - BINARY_OP_BASE] = do_add,
	[OP_SUB  - BINARY_OP_BASE] = do_sub,
	[OP_MUL  - BINARY_OP_BASE] = do_mul,
	[OP_AND  - BINARY_OP_BASE] = do_and,
	[OP_OR   - BINARY_OP_BASE] = do_or,
	[OP_BAND - BINARY_OP_BASE] = do_band,
	[OP_BOR  - BINARY_OP_BASE] = do_bor,
	[OP_BXOR - BINARY_OP_BASE] = do_bxor,
	[OP_EQ   - BINARY_OP_BASE] = do_eq,
};
#endif

int handle_binary_op(StackMachine &calc, int &result, int op) {
	if (calc.top < 2) {
		return STATUS_NOT_ENOUGH_OPERANDS;
	}
	--calc.top;
	int op1 = calc.stack[calc.top];
	--calc.top;
	int op2 = calc.stack[calc.top];
	calc.stack[calc.top] = StackMachine::binary_dispatch[op](op1, op2);
	calc.top++;
	return STATUS_OK;
}

#if 1 // only trivial initializers are supported
stack_machine_handler_t StackMachine::dispatch[BINARY_OP_BASE] = {
	handle_call, 
	handle_ret, 
	handle_push, 
	handle_pop, 
	handle_j, 
	handle_jz, 
	handle_jnz, 
	handle_dup, 
	handle_swap, 
	handle_not, 
	handle_bnot, 
};
#else
stack_machine_handler_t StackMachine::dispatch[BINARY_OP_BASE] = {
	[OP_CALL]  = handle_call, 
	[OP_RET]   = handle_ret, 
	[OP_PUSH]  = handle_push, 
	[OP_POP]   = handle_pop, 
	[OP_J]     = handle_j, 
	[OP_JZ]    = handle_jz, 
	[OP_JNZ]   = handle_jnz, 
	[OP_DUP]   = handle_dup, 
	[OP_SWAP]  = handle_swap, 
	[OP_NOT]   = handle_not, 
	[OP_BNOT]  = handle_bnot, 
};
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

	if (cmd < 0) {
		return STATUS_UNKNOWN_OPCODE;
	}
	if (cmd < BINARY_OP_BASE) {
		return StackMachine::dispatch[cmd](calc, result);
	}
	if (cmd < MAX_OP) {
		return handle_binary_op(calc, result, cmd - BINARY_OP_BASE);
	}
	return STATUS_UNKNOWN_OPCODE;
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

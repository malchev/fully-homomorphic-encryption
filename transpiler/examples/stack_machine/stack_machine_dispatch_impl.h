#ifndef STACK_MACHINE_DISPATCH_IMPL
#define STACK_MACHINE_DISPATCH_IMPL

#include "stack_machine.h"

#undef DEBUG

#ifdef DEBUG
#include <cstdio>
#define PRINT(...) fprintf(stdout, __VA_ARGS__)
#else
#define PRINT(...) do {} while(false)
#endif

int handle_call(StackMachine &calc, int &res) {
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

int handle_ret(StackMachine &calc, int &res) {
	if (calc.call == 0) {
		if (calc.top == 0) {
			return STATUS_EMPTY_STACK;
		}
		calc.top--;
		res = calc.stack[calc.top];
		return STATUS_HALT;
	}
	else {
		--calc.call;
		calc.step = calc.callstack[calc.call];
	}
	return STATUS_OK;
}

int handle_push(StackMachine &calc, int &res) {
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

int handle_pop(StackMachine &calc, int &res) {
	if (calc.top == 0) {
		return STATUS_EMPTY_STACK;
	}
	--calc.top;
	return STATUS_OK;
}

int handle_dup(StackMachine &calc, int &res) {
	if (calc.top == 0) {
		return STATUS_EMPTY_STACK;
	}
	int top = calc.stack[calc.top-1];
	calc.stack[calc.top] = top;
	calc.top++;
	return STATUS_OK;
}

int handle_j(StackMachine &calc, int &res) {
	if (calc.step == MAX_CMD) {
		return STATUS_MAX_COMMANDS;
	}
	calc.step = calc.commands[calc.step];
	return STATUS_OK;
}

int handle_j_cond(StackMachine &calc, int &res, bool want_zero) {
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

int handle_jz(StackMachine &calc, int &res) {
	return handle_j_cond(calc, res, true);
}

int handle_jnz(StackMachine &calc, int &res) {
	return handle_j_cond(calc, res, false);
}

int handle_not(StackMachine &calc, int &res) {
	if (calc.top < 1) {
		return STATUS_NOT_ENOUGH_OPERANDS;
	}
	--calc.top;
	int op1 = calc.stack[calc.top];
	calc.stack[calc.top] = !op1;
	calc.top++;
	return STATUS_OK;
}

int handle_bnot(StackMachine &calc, int &res) {
	if (calc.top < 1) {
		return STATUS_NOT_ENOUGH_OPERANDS;
	}
	--calc.top;
	int op1 = calc.stack[calc.top];
	calc.stack[calc.top] = ~op1;
	calc.top++;
	return STATUS_OK;
}

int handle_swap(StackMachine &calc, int &res) {
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


typedef int (*handle_binary_op_t)(int op1, int op2);
#if 1 // only trivial initializers are supported
handle_binary_op_t stack_machine_binary_dispatch[MAX_OP - BINARY_OP_BASE] = {
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
handle_binary_op_t stack_machine_binary_dispatch[MAX_OP - BINARY_OP_BASE] = {
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

int handle_binary_op(StackMachine &calc, int &res, int op) {
	if (calc.top < 2) {
		return STATUS_NOT_ENOUGH_OPERANDS;
	}
	--calc.top;
	int op1 = calc.stack[calc.top];
	--calc.top;
	int op2 = calc.stack[calc.top];
	calc.stack[calc.top] = stack_machine_binary_dispatch[op](op1, op2);
	calc.top++;
	return STATUS_OK;
}

typedef int (*stack_machine_handler_t)(StackMachine &calc, int &res);
#if 1 // only trivial initializers are supported
stack_machine_handler_t stack_machine_dispatch[BINARY_OP_BASE] = {
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
stack_machine_handler_t stack_machine_dispatch[BINARY_OP_BASE] = {
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

int stack_machine_tick(StackMachine &calc, int &res) {
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
		return stack_machine_dispatch[cmd](calc, res);
	}
	if (cmd < MAX_OP) {
		return handle_binary_op(calc, res, cmd - BINARY_OP_BASE);
	}
	return STATUS_UNKNOWN_OPCODE;
}

int stack_machine_compute(StackMachine &calc, int max_gas, int &res) {
	for (int gas = 0; (max_gas >= 0 && gas < max_gas) || (max_gas < 0); gas++) {
		int status = stack_machine_tick(calc, res);
		if (status != STATUS_OK)
			return status;
	}
	return STATUS_RAN_OUT_OF_GAS;
}

#endif//STACK_MACHINE_DISPATCH_IMPL

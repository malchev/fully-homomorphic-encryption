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

int handle_call(StackMachine &sm) {
	if (sm.step == MAX_CMD) {
		return STATUS_MAX_COMMANDS;
	}
	if (sm.call == MAX_CALLS) {
		return STATUS_MAX_CALLS;
	}
	int target = sm.commands[sm.step];
	sm.step++;
	sm.callstack[sm.call] = sm.step;
	sm.call++;
	sm.step = target;
	return STATUS_OK;
}

int handle_ret(StackMachine &sm) {
	if (sm.call == 0) {
		if (sm.top == 0) {
			return STATUS_EMPTY_STACK;
		}
		return STATUS_HALT;
	}
	else {
		--sm.call;
		sm.step = sm.callstack[sm.call];
	}
	return STATUS_OK;
}

int handle_push(StackMachine &sm) {
	if (sm.step == MAX_CMD) {
		return STATUS_MAX_COMMANDS;
	}
	if (sm.top == MAX_OPD) {
		return STATUS_EXCEEDED_MAX_OPERANDS;
	}
	int val = sm.commands[sm.step];
	sm.step++;
	sm.stack[sm.top] = val;
	sm.top++;
	return STATUS_OK;
}

int handle_pop(StackMachine &sm) {
	if (sm.top == 0) {
		return STATUS_EMPTY_STACK;
	}
	--sm.top;
	return STATUS_OK;
}

int handle_dup(StackMachine &sm) {
	if (sm.top == 0) {
		return STATUS_EMPTY_STACK;
	}
	int top = sm.stack[sm.top-1];
	sm.stack[sm.top] = top;
	sm.top++;
	return STATUS_OK;
}

int handle_j(StackMachine &sm) {
	if (sm.step == MAX_CMD) {
		return STATUS_MAX_COMMANDS;
	}
	sm.step = sm.commands[sm.step];
	return STATUS_OK;
}

int handle_j_cond(StackMachine &sm, bool want_zero) {
	if (sm.step == MAX_CMD) {
		return STATUS_MAX_COMMANDS;
	}
	if (sm.top == 0) {
		return STATUS_EMPTY_STACK;
	}
	int target = sm.commands[sm.step];
	sm.step++;
	--sm.top;
	bool is_zero = !sm.stack[sm.top];
	if (!(want_zero ^ is_zero)) {
		sm.step = target;
	}
	return STATUS_OK;
}

int handle_jz(StackMachine &sm) {
	return handle_j_cond(sm, true);
}

int handle_jnz(StackMachine &sm) {
	return handle_j_cond(sm, false);
}

int handle_not(StackMachine &sm) {
	if (sm.top < 1) {
		return STATUS_NOT_ENOUGH_OPERANDS;
	}
	--sm.top;
	int op1 = sm.stack[sm.top];
	sm.stack[sm.top] = !op1;
	sm.top++;
	return STATUS_OK;
}

int handle_bnot(StackMachine &sm) {
	if (sm.top < 1) {
		return STATUS_NOT_ENOUGH_OPERANDS;
	}
	--sm.top;
	int op1 = sm.stack[sm.top];
	sm.stack[sm.top] = ~op1;
	sm.top++;
	return STATUS_OK;
}

int handle_swap(StackMachine &sm) {
	if (sm.top < 2) {
		return STATUS_NOT_ENOUGH_OPERANDS;
	}
	--sm.top;
	int ontop = sm.stack[sm.top];
	--sm.top;
	int below = sm.stack[sm.top];
	sm.stack[sm.top] = ontop;
	sm.top++;
	sm.stack[sm.top] = below;
	sm.top++;
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

int handle_binary_op(StackMachine &sm, int op) {
	if (sm.top < 2) {
		return STATUS_NOT_ENOUGH_OPERANDS;
	}
	--sm.top;
	int op1 = sm.stack[sm.top];
	--sm.top;
	int op2 = sm.stack[sm.top];
	sm.stack[sm.top] = stack_machine_binary_dispatch[op](op1, op2);
	sm.top++;
	return STATUS_OK;
}

typedef int (*stack_machine_handler_t)(StackMachine &sm);
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

StackMachine stack_machine_tick(StackMachine &sm) {
	StackMachine next = sm;

	if (next.status != STATUS_OK) {
		return next;
	}

	if (next.step == MAX_CMD) {
		return STATUS_MAX_COMMANDS;
	}
	int cmd = next.commands[next.step];
	next.step++;

	if (cmd < 0) {
		return STATUS_UNKNOWN_OPCODE;
	}
	if (cmd < BINARY_OP_BASE) {
		next.status = stack_machine_dispatch[cmd](next);
		return next;
	}
	if (cmd < MAX_OP) {
		next.status = handle_binary_op(next, cmd - BINARY_OP_BASE);
		return next;
	}
	next.status = STATUS_UNKNOWN_OPCODE;
	return next;
}

StackMachine stack_machine_compute(StackMachine &sm, int max_gas) {
	StackMachine next = sm;
	for (int gas = 0; (max_gas >= 0 && gas < max_gas) || (max_gas < 0); gas++) {
#ifdef DEBUG
		do {
			printf("%-8d %c:", gas, next.commands[next.step]);
			for (int n = 0; n < next.top; n++) {
				printf(" %-8d", next.stack[n]);
			}
			printf("\n");
		} while(false);
#endif
		next = stack_machine_tick(next);
		if (next.status != STATUS_OK)
			return next;
	}
	next.status = STATUS_RAN_OUT_OF_GAS;
	return next;
}

#endif//STACK_MACHINE_DISPATCH_IMPL

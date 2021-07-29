#ifndef STACK_MACHINE_SWITCH_IMPL
#define STACK_MACHINE_SWITCH_IMPL

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

int handle_binary_op(StackMachine &sm, int op) {
	if (sm.top < 2) {
		return STATUS_NOT_ENOUGH_OPERANDS;
	}
	--sm.top;
	int op1 = sm.stack[sm.top];
	--sm.top;
	int op2 = sm.stack[sm.top];
	switch(op) {
		case OP_ADD  : { sm.stack[sm.top] = do_add(op1, op2); break; }
		case OP_SUB  : { sm.stack[sm.top] = do_sub(op1, op2); break; }
		case OP_MUL  : { sm.stack[sm.top] = do_mul(op1, op2); break; }
		case OP_AND  : { sm.stack[sm.top] = do_and(op1, op2); break; }
		case OP_OR   : { sm.stack[sm.top] = do_or(op1, op2); break; }
		case OP_BAND : { sm.stack[sm.top] = do_band(op1, op2); break; }
		case OP_BOR  : { sm.stack[sm.top] = do_bor(op1, op2); break; }
		case OP_BXOR : { sm.stack[sm.top] = do_bxor(op1, op2); break; }
		case OP_EQ   : { sm.stack[sm.top] = do_eq(op1, op2); break; }
		default:
			return STATUS_UNKNOWN_OPCODE; //internal error
	}
	sm.top++;
	return STATUS_OK;
}

StackMachine stack_machine_tick(const StackMachine &sm) {
	StackMachine next = sm;

	if (next.step == MAX_CMD) {
		next.status = STATUS_MAX_COMMANDS;
		return next;
	}
	int cmd = next.commands[next.step];
	next.step++;

	if (cmd < 0) {
		next.status = STATUS_UNKNOWN_OPCODE;
		return next;
	}
	if (cmd < BINARY_OP_BASE) {
		switch(cmd) {
			case OP_CALL:  next.status = handle_call(next); return next;
			case OP_RET:   next.status = handle_ret(next);  return next;
			case OP_PUSH:  next.status = handle_push(next); return next;
			case OP_POP:   next.status = handle_pop(next);  return next;
			case OP_J:     next.status = handle_j(next);    return next;
			case OP_JZ:    next.status = handle_jz(next);   return next;
			case OP_JNZ:   next.status = handle_jnz(next);  return next;
			case OP_DUP:   next.status = handle_dup(next);  return next;
			case OP_SWAP:  next.status = handle_swap(next); return next;
			case OP_NOT:   next.status = handle_not(next);  return next;
			case OP_BNOT:  next.status = handle_bnot(next); return next;
		default:
			next.status = STATUS_UNKNOWN_OPCODE; //internal error
			return next;
		}
	}
	if (cmd < MAX_OP) {
		next.status = handle_binary_op(next, cmd);
		return next;
	}
	next.status = STATUS_UNKNOWN_OPCODE;
	return next;
}

StackMachine stack_machine_compute(const StackMachine &sm, int max_gas) {
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

#endif//STACK_MACHINE_SWITCH_IMPL

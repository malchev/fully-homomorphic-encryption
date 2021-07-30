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

void handle_call(StackMachine &sm) {
	if (sm.step == MAX_CMD) {
		sm.status = STATUS_MAX_COMMANDS; return;
	}
	if (sm.call == MAX_CALLS) {
		sm.status = STATUS_MAX_CALLS; return;
	}
	int target = sm.commands[sm.step];
	sm.step++;
	sm.callstack[sm.call] = sm.step;
	sm.call++;
	sm.step = target;
	sm.status = STATUS_OK; return;
}

void handle_ret(StackMachine &sm) {
	if (sm.call == 0) {
		if (sm.top == 0) {
			sm.status = STATUS_EMPTY_STACK; return;
		}
		sm.status = STATUS_HALT; return;
	}
	else {
		--sm.call;
		sm.step = sm.callstack[sm.call];
	}
	sm.status = STATUS_OK; return;
}

void handle_push(StackMachine &sm) {
	if (sm.step == MAX_CMD) {
		sm.status = STATUS_MAX_COMMANDS; return;
	}
	if (sm.top == MAX_OPD) {
		sm.status = STATUS_EXCEEDED_MAX_OPERANDS; return;
	}
	int val = sm.commands[sm.step];
	sm.step++;
	sm.stack[sm.top] = val;
	sm.top++;
	sm.status = STATUS_OK; return;
}

void handle_pop(StackMachine &sm) {
	if (sm.top == 0) {
		sm.status = STATUS_EMPTY_STACK; return;
	}
	--sm.top;
	sm.status = STATUS_OK; return;
}

void handle_dup(StackMachine &sm) {
	if (sm.top == 0) {
		sm.status = STATUS_EMPTY_STACK; return;
	}
	int top = sm.stack[sm.top-1];
	sm.stack[sm.top] = top;
	sm.top++;
	sm.status = STATUS_OK; return;
}

void handle_j(StackMachine &sm) {
	if (sm.step == MAX_CMD) {
		sm.status = STATUS_MAX_COMMANDS; return;
	}
	sm.step = sm.commands[sm.step];
	sm.status = STATUS_OK; return;
}

void handle_j_cond(StackMachine &sm, bool want_zero) {
	if (sm.step == MAX_CMD) {
		sm.status = STATUS_MAX_COMMANDS; return;
	}
	if (sm.top == 0) {
		sm.status = STATUS_EMPTY_STACK; return;
	}
	int target = sm.commands[sm.step];
	sm.step++;
	--sm.top;
	bool is_zero = !sm.stack[sm.top];
	if (!(want_zero ^ is_zero)) {
		sm.step = target;
	}
	sm.status = STATUS_OK; return;
}

void handle_jz(StackMachine &sm) {
	return handle_j_cond(sm, true);
}

void handle_jnz(StackMachine &sm) {
	return handle_j_cond(sm, false);
}

void handle_not(StackMachine &sm) {
	if (sm.top < 1) {
		sm.status = STATUS_NOT_ENOUGH_OPERANDS; return;
	}
	--sm.top;
	int op1 = sm.stack[sm.top];
	sm.stack[sm.top] = !op1;
	sm.top++;
	sm.status = STATUS_OK; return;
}

void handle_bnot(StackMachine &sm) {
	if (sm.top < 1) {
		sm.status = STATUS_NOT_ENOUGH_OPERANDS; return;
	}
	--sm.top;
	int op1 = sm.stack[sm.top];
	sm.stack[sm.top] = ~op1;
	sm.top++;
	sm.status = STATUS_OK; return;
}

void handle_swap(StackMachine &sm) {
	if (sm.top < 2) {
		sm.status = STATUS_NOT_ENOUGH_OPERANDS; return;
	}
	--sm.top;
	int ontop = sm.stack[sm.top];
	--sm.top;
	int below = sm.stack[sm.top];
	sm.stack[sm.top] = ontop;
	sm.top++;
	sm.stack[sm.top] = below;
	sm.top++;
	sm.status = STATUS_OK; return;
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

void handle_binary_op(StackMachine &sm, int op) {
	if (sm.top < 2) {
		sm.status = STATUS_NOT_ENOUGH_OPERANDS; return;
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
			sm.status = STATUS_UNKNOWN_OPCODE; return; //internal error
	}
	sm.top++;
	sm.status = STATUS_OK;
}

void stack_machine_tick(StackMachine &sm) {
	if (sm.step == MAX_CMD) {
		sm.status = STATUS_MAX_COMMANDS;
		return;
	}
	int cmd = sm.commands[sm.step];
	sm.step++;

	if (cmd < 0) {
		sm.status = STATUS_UNKNOWN_OPCODE;
		return;
	}
	if (cmd < BINARY_OP_BASE) {
		switch(cmd) {
			case OP_CALL: handle_call(sm); return;
			case OP_RET:  handle_ret(sm);  return;
			case OP_PUSH: handle_push(sm); return;
			case OP_POP:  handle_pop(sm);  return;
			case OP_J:    handle_j(sm);    return;
			case OP_JZ:   handle_jz(sm);   return;
			case OP_JNZ:  handle_jnz(sm);  return;
			case OP_DUP:  handle_dup(sm);  return;
			case OP_SWAP: handle_swap(sm); return;
			case OP_NOT:  handle_not(sm);  return;
			case OP_BNOT: handle_bnot(sm); return;
		default:
			sm.status = STATUS_UNKNOWN_OPCODE; //internal error
			return;
		}
	}
	if (cmd < MAX_OP) {
		handle_binary_op(sm, cmd);
		return;
	}
	sm.status = STATUS_UNKNOWN_OPCODE;
	return;
}

void stack_machine_compute(StackMachine &sm, int max_gas) {
	for (int gas = 0; (max_gas >= 0 && gas < max_gas) || (max_gas < 0); gas++) {
#ifdef DEBUG
		do {
			printf("%-8d %c:", gas, sm.commands[sm.step]);
			for (int n = 0; n < sm.top; n++) {
				printf(" %-8d", sm.stack[n]);
			}
			printf("\n");
		} while(false);
#endif
		stack_machine_tick(sm);
		if (sm.status != STATUS_OK)
			return;
	}
    sm.status = STATUS_RAN_OUT_OF_GAS;
	return;
}

#endif//STACK_MACHINE_SWITCH_IMPL

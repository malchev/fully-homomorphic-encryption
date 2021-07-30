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

void stack_machine_tick(StackMachine &sm) {
	if (sm.step == MAX_CMD) {
		sm.status = STATUS_MAX_COMMANDS;
		return;
	}
	int cmd = sm.commands[sm.step];
	sm.step++;

	if (cmd == OP_CALL) {
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
	}
	else if (cmd == OP_RET) {
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
	}
	else if (cmd == OP_PUSH) {
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
	}
	else if (cmd == OP_POP) {
		if (sm.top == 0) {
			sm.status = STATUS_EMPTY_STACK; return;
		}
		--sm.top;
	}
	else if (cmd == OP_DUP) {
		if (sm.top == 0) {
			sm.status = STATUS_EMPTY_STACK; return;
		}
		int top = sm.stack[sm.top-1];
		sm.stack[sm.top] = top;
		sm.top++;
	}
	else if (cmd == OP_J) {
		if (sm.step == MAX_CMD) {
			sm.status = STATUS_MAX_COMMANDS; return;
		}
		sm.step = sm.commands[sm.step];
	}
	else if (cmd == OP_JZ) {
		if (sm.step == MAX_CMD) {
			sm.status = STATUS_MAX_COMMANDS; return;
		}
		if (sm.top == 0) {
			sm.status = STATUS_EMPTY_STACK; return;
		}
		int target = sm.commands[sm.step];
		sm.step++;
		--sm.top;
		int top = sm.stack[sm.top];
		if (!top) {
			sm.step = target;
		}
	}
	else if (cmd == OP_JNZ) {
		if (sm.step == MAX_CMD) {
			sm.status = STATUS_MAX_COMMANDS; return;
		}
		if (sm.top == 0) {
			sm.status = STATUS_EMPTY_STACK; return;
		}
		int target = sm.commands[sm.step];
		sm.step++;
		--sm.top;
		int top = sm.stack[sm.top];
		if (top) {
			sm.step = target;
		}
	}
	else if (cmd == OP_NOT) {
		if (sm.top < 1) {
			sm.status = STATUS_NOT_ENOUGH_OPERANDS; return;
		}
		--sm.top;
		int op1 = sm.stack[sm.top];
		sm.stack[sm.top] = !op1;
		sm.top++;
	}
	else if (cmd == OP_AND) {
		if (sm.top < 2) {
			sm.status = STATUS_NOT_ENOUGH_OPERANDS; return;
		}
		--sm.top;
		int op1 = sm.stack[sm.top];
		--sm.top;
		int op2 = sm.stack[sm.top];
		sm.stack[sm.top] = op1 && op2;
		sm.top++;
	}
	else if (cmd == OP_OR) {
		if (sm.top < 2) {
			sm.status = STATUS_NOT_ENOUGH_OPERANDS; return;
		}
		--sm.top;
		int op1 = sm.stack[sm.top];
		--sm.top;
		int op2 = sm.stack[sm.top];
		sm.stack[sm.top] = op1 || op2;
		sm.top++;
	}
	else if (cmd == OP_BNOT) {
		if (sm.top < 1) {
			sm.status = STATUS_NOT_ENOUGH_OPERANDS; return;
		}
		--sm.top;
		int op1 = sm.stack[sm.top];
		sm.stack[sm.top] = ~op1;
		sm.top++;
	}
	else if (cmd == OP_BAND) {
		if (sm.top < 2) {
			sm.status = STATUS_NOT_ENOUGH_OPERANDS; return;
		}
		--sm.top;
		int op1 = sm.stack[sm.top];
		--sm.top;
		int op2 = sm.stack[sm.top];
		sm.stack[sm.top] = op1 & op2;
		sm.top++;
	}
	else if (cmd == OP_BOR) {
		if (sm.top < 2) {
			sm.status = STATUS_NOT_ENOUGH_OPERANDS; return;
		}
		--sm.top;
		int op1 = sm.stack[sm.top];
		--sm.top;
		int op2 = sm.stack[sm.top];
		sm.stack[sm.top] = op1 | op2;
		sm.top++;
	}
	else if (cmd == OP_BXOR) {
		if (sm.top < 2) {
			sm.status = STATUS_NOT_ENOUGH_OPERANDS; return;
		}
		--sm.top;
		int op1 = sm.stack[sm.top];
		--sm.top;
		int op2 = sm.stack[sm.top];
		sm.stack[sm.top] = op1 ^ op2;
		sm.top++;
	}

	else if (cmd == OP_SWAP) {
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
	}
	else if (cmd == OP_EQ) {
		if (sm.top < 2) {
			sm.status = STATUS_NOT_ENOUGH_OPERANDS; return;
		}
		--sm.top;
		int op1 = sm.stack[sm.top];
		--sm.top;
		int op2 = sm.stack[sm.top];
		sm.stack[sm.top] = op1 == op2;
		sm.top++;
	}
	else if (cmd == OP_ADD) {
		if (sm.top < 2) {
			sm.status = STATUS_NOT_ENOUGH_OPERANDS; return;
		}
		--sm.top;
		int op1 = sm.stack[sm.top];
		--sm.top;
		int op2 = sm.stack[sm.top];
		sm.stack[sm.top] = op1 + op2;
		sm.top++;
	}
	else if (cmd == OP_SUB) {
		if (sm.top < 2) {
			sm.status = STATUS_NOT_ENOUGH_OPERANDS; return;
		}
		--sm.top;
		int op2 = sm.stack[sm.top];
		--sm.top;
		int op1 = sm.stack[sm.top];
		sm.stack[sm.top] = op1 - op2;
		sm.top++;
	}
	else if (cmd == OP_MUL) {
		if (sm.top < 2) {
			sm.status = STATUS_NOT_ENOUGH_OPERANDS; return;
		}
		--sm.top;
		int op2 = sm.stack[sm.top];
		--sm.top;
		int op1 = sm.stack[sm.top];
		sm.stack[sm.top] = op1 * op2;
		sm.top++;
	}
	else {
		sm.status = STATUS_UNKNOWN_OPCODE; return;
	}

	sm.status = STATUS_OK;
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

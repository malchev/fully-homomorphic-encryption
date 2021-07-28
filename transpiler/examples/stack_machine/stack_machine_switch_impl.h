#ifndef STACK_MACHINE_SWITCH_IMPL
#define STACK_MACHINE_SWITCH_IMPL

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

int handle_binary_op(StackMachine &calc, int &res, int op) {
	if (calc.top < 2) {
		return STATUS_NOT_ENOUGH_OPERANDS;
	}
	--calc.top;
	int op1 = calc.stack[calc.top];
	--calc.top;
	int op2 = calc.stack[calc.top];
	switch(op) {
		case OP_ADD  : { calc.stack[calc.top] = do_add(op1, op2); break; }
		case OP_SUB  : { calc.stack[calc.top] = do_sub(op1, op2); break; }
		case OP_MUL  : { calc.stack[calc.top] = do_mul(op1, op2); break; }
		case OP_AND  : { calc.stack[calc.top] = do_and(op1, op2); break; }
		case OP_OR   : { calc.stack[calc.top] = do_or(op1, op2); break; }
		case OP_BAND : { calc.stack[calc.top] = do_band(op1, op2); break; }
		case OP_BOR  : { calc.stack[calc.top] = do_bor(op1, op2); break; }
		case OP_BXOR : { calc.stack[calc.top] = do_bxor(op1, op2); break; }
		case OP_EQ   : { calc.stack[calc.top] = do_eq(op1, op2); break; }
		default:
			return STATUS_UNKNOWN_OPCODE; //internal error
	}
	calc.top++;
	return STATUS_OK;
}

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
		switch(cmd) {
			case OP_CALL:  return handle_call(calc, res);
			case OP_RET:   return handle_ret(calc, res);
			case OP_PUSH:  return handle_push(calc, res);
			case OP_POP:   return handle_pop(calc, res);
			case OP_J:     return handle_j(calc, res);
			case OP_JZ:    return handle_jz(calc, res);
			case OP_JNZ:   return handle_jnz(calc, res);
			case OP_DUP:   return handle_dup(calc, res);
			case OP_SWAP:  return handle_swap(calc, res);
			case OP_NOT:   return handle_not(calc, res);
			case OP_BNOT:  return handle_bnot(calc, res);
		default:
			return STATUS_UNKNOWN_OPCODE; //internal error
		}
	}
	if (cmd < MAX_OP) {
		return handle_binary_op(calc, res, cmd);
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

#endif//STACK_MACHINE_SWITCH_IMPL

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

#include "stack_machine.h"

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

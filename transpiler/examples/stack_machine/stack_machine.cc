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

#pragma hls_top
int BatchCalculator::compute(BatchCalculator &calc, int &result) {
	int step = 0;
	int status = STATUS_RAN_OUT_OF_GAS;
    #pragma hls_unroll yes
	for (int gas = 0; gas < MAX_GAS; gas++) {

#ifdef DEBUG
		do {
			printf("%-8d %c:", gas, calc.commands[step]);
			for (int n = 0; n < calc.top; n++) {
				printf(" %-8d", calc.stack[n]);
			}
			printf("\n");
		} while(false);
#endif

		if (step == MAX_CMD) {
			status = STATUS_MAX_COMMANDS;
			break;
		}
		int cmd = calc.commands[step];
		step++;
		if (cmd == OP_CALL) {
			if (step == MAX_CMD) {
				status = STATUS_MAX_COMMANDS;
				break;
			}
			if (calc.call == MAX_CALLS) {
				status = STATUS_MAX_CALLS;
				break;
			}
			int target = calc.commands[step];
			step++;
			calc.callstack[calc.call] = step;
			calc.call++;
			step = target;
		}
		else if (cmd == OP_RET) {
			if (calc.call == 0) {
				if (calc.top == 0) {
					status = STATUS_EMPTY_STACK;
					break;
				}
				calc.top--;
				status = STATUS_OK;
				result = calc.stack[calc.top];
				break;
			}
			else {
				--calc.call;
				step = calc.callstack[calc.call];
			}
		}
		else if (cmd == OP_PUSH) {
			if (step == MAX_CMD) {
				status = STATUS_MAX_COMMANDS;
				break;
			}
			if (calc.top == MAX_OPD) {
				status = STATUS_EXCEEDED_MAX_OPERANDS;
				break;
			}
			int val = calc.commands[step];
			step++;
			calc.stack[calc.top] = val;
			calc.top++;
		}
		else if (cmd == OP_POP) {
			if (calc.top == 0) {
				status = STATUS_EMPTY_STACK;
				break;
			}
			--calc.top;
		}
		else if (cmd == OP_DUP) {
			if (calc.top == 0) {
				status = STATUS_EMPTY_STACK;
				break;
			}
			int top = calc.stack[calc.top-1];
			calc.stack[calc.top] = top;
			calc.top++;
		}
		else if (cmd == OP_J) {
			if (step == MAX_CMD) {
				status = STATUS_MAX_COMMANDS;
				break;
			}
			step = calc.commands[step];
		}
		else if (cmd == OP_JZ) {
			if (step == MAX_CMD) {
				status = STATUS_MAX_COMMANDS;
				break;
			}
			if (calc.top == 0) {
				status = STATUS_EMPTY_STACK;
				break;
			}
			int target = calc.commands[step];
			step++;
			--calc.top;
			int top = calc.stack[calc.top];
			if (!top) {
				step = target;
			}
		}
		else if (cmd == OP_JNZ) {
			if (step == MAX_CMD) {
				status = STATUS_MAX_COMMANDS;
				break;
			}
			if (calc.top == 0) {
				status = STATUS_EMPTY_STACK;
				break;
			}
			int target = calc.commands[step];
			step++;
			--calc.top;
			int top = calc.stack[calc.top];
			if (top) {
				step = target;
			}
		}
		else if (cmd == OP_NOT) {
			if (calc.top < 1) {
				status = STATUS_NOT_ENOUGH_OPERANDS;
				break;
			}
			--calc.top;
			int op1 = calc.stack[calc.top];
			calc.stack[calc.top] = !op1;
			calc.top++;
		}
		else if (cmd == OP_AND) {
			if (calc.top < 2) {
				status = STATUS_NOT_ENOUGH_OPERANDS;
				break;
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
				status = STATUS_NOT_ENOUGH_OPERANDS;
				break;
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
				status = STATUS_NOT_ENOUGH_OPERANDS;
				break;
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
				status = STATUS_NOT_ENOUGH_OPERANDS;
				break;
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
				status = STATUS_NOT_ENOUGH_OPERANDS;
				break;
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
				status = STATUS_NOT_ENOUGH_OPERANDS;
				break;
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
				status = STATUS_NOT_ENOUGH_OPERANDS;
				break;
			}
			--calc.top;
			int op2 = calc.stack[calc.top];
			--calc.top;
			int op1 = calc.stack[calc.top];
			calc.stack[calc.top] = op1 * op2;
			calc.top++;
		}
		else {
			status = STATUS_UNKNOWN_OPCODE;
			break;
		}
	}
	return status;
}

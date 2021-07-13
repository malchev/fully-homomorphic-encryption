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

#include "batch_calculator.h"

#pragma hls_top
int BC::compute(BC &calc, int &result) {
	int step = 0;
	int status = STATUS_RAN_OUT_OF_GAS;
#if 0
    #pragma hls_unroll yes
	for (int gas = 0; gas < MAX_GAS; gas++) {
		if (step == MAX_CMD) {
			status = STATUS_MAX_COMMANDS;
			break;
		}
		int cmd = calc.commands[step];
		step++;
		if (cmd == OP_END) {
			if (calc.top == 0) {
				status = STATUS_NO_FINAL_RESULT;
				break;
			}
			calc.top--;
			status = STATUS_OK;
			result = calc.stack[calc.top];
			break;
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
			int op2 = calc.stack[calc.top];
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
#endif
	return status;
}

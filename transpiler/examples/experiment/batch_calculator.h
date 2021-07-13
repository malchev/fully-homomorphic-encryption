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


#ifndef BATCH_CALCULATOR_H
#define BATCH_CALCULATOR_H

// Result codes
#define STATUS_OK                    0
#define STATUS_MAX_COMMANDS          1
#define STATUS_NO_FINAL_RESULT       2
#define STATUS_EXCEEDED_MAX_OPERANDS 3
#define STATUS_NOT_ENOUGH_OPERANDS   4
#define STATUS_RAN_OUT_OF_GAS        5
#define STATUS_UNKNOWN_OPCODE        6

// Opcodes
#define OP_END  0
#define	OP_ADD  1
#define	OP_SUB  2
#define	OP_MUL  3
#define OP_PUSH 4

// Max number of commands, counting a final END, not counting operands
#define MAX_GAS 50 //999; 
// Max depth of operand stack
#define MAX_OPD 50 //999;
// Maximum number of commands and command-operands (i.e. "push 1" counts as 2 entries)
#define MAX_CMD 50 //999;
	
struct BC {

	BC() : top(0) {}

	int commands[MAX_CMD];
	int stack[MAX_OPD];
	int top;

	static int compute(BC &calc, int &result);
};

#endif//BATCH_CALCULATOR_H

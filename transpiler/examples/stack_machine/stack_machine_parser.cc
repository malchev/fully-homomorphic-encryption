#include <iostream>
#include <string>
#include <sstream>
#include <iterator>
#include <stdexcept>
#include <vector>

#include "stack_machine_parser.h"
#include "stack_machine.h"

bool StackMachineParser(const std::string &prog, StackMachine &calc) {
	StackMachine res;

	std::istringstream iss(prog);
	std::vector<std::string> tokens{std::istream_iterator<std::string>{iss},
		                       std::istream_iterator<std::string>{}};

#define check_cmd_index(v) do {								                        \
	int __v = (v);											      	                \
	if (__v >= MAX_CMD) {							                                \
		std::cerr << "Number of commands exceeds " << MAX_CMD << std::endl;         \
		return false;									                            \
	}																                \
} while(false)

#define parse_argument(val) do {                                                    \
	i++;                                                                            \
	if (i == tokens.size()) {                                                       \
		std::cerr << "'" << tok << "' expects a numerical argument" << std::endl;   \
		return false;                                                               \
	}                                                                               \
	auto &arg = tokens[i];                                                          \
	std::size_t end;                                                                \
	try {                                                                           \
		val = std::stoi(arg, &end, 0);                                              \
		if (end != arg.size()) {                                                    \
			std::cerr << "Trailing non-numerical characters: " << arg << std::endl; \
			return false;                                                           \
		}                                                                           \
	}                                                                               \
	catch (std::invalid_argument &ia) {                                             \
		std::cerr << "Invalid argument: " << arg << std::endl;                      \
		return false;                                                               \
	}                                                                               \
	catch (std::out_of_range &oor) {                                                \
		std::cerr << "Out of range: " << arg << std::endl;                          \
		return false;                                                               \
	}                                                                               \
} while(false)

	int next_cmd_index = 0;
	for (int i = 0; i < tokens.size(); i++) {
		auto &tok = tokens[i];

		if (tok == "push") {
			int val; parse_argument(val);
			check_cmd_index(next_cmd_index + 1);
			calc.commands[next_cmd_index++] = OP_PUSH;
			calc.commands[next_cmd_index++] = val;
		}
		else if (tok == "pop") {
			check_cmd_index(next_cmd_index);
			calc.commands[next_cmd_index++] = OP_POP;
		}
		else if (tok == "jump") {
			int val; parse_argument(val);
			check_cmd_index(next_cmd_index + 1);
			calc.commands[next_cmd_index++] = OP_J;
			calc.commands[next_cmd_index++] = val;
		}
		else if (tok == "jz") {
			int val; parse_argument(val);
			check_cmd_index(next_cmd_index + 1);
			calc.commands[next_cmd_index++] = OP_JZ;
			calc.commands[next_cmd_index++] = val;
		}
		else if (tok == "jnz") {
			int val; parse_argument(val);
			check_cmd_index(next_cmd_index + 1);
			calc.commands[next_cmd_index++] = OP_JNZ;
			calc.commands[next_cmd_index++] = val;
		}
		else if (tok == "dup") {
			check_cmd_index(next_cmd_index);
			calc.commands[next_cmd_index++] = OP_DUP;
		}
		else if (tok == "swap") {
			check_cmd_index(next_cmd_index);
			calc.commands[next_cmd_index++] = OP_SWAP;
		}
		else if (tok == "eq") {
			check_cmd_index(next_cmd_index);
			calc.commands[next_cmd_index++] = OP_EQ;
		}
		else if (tok == "and") {
			check_cmd_index(next_cmd_index);
			calc.commands[next_cmd_index++] = OP_AND;
		}
		else if (tok == "or") {
			check_cmd_index(next_cmd_index);
			calc.commands[next_cmd_index++] = OP_OR;
		}
		else if (tok == "not") {
			check_cmd_index(next_cmd_index);
			calc.commands[next_cmd_index++] = OP_NOT;
		}
		else if (tok == "&") {
			check_cmd_index(next_cmd_index);
			calc.commands[next_cmd_index++] = OP_BAND;
		}
		else if (tok == "|") {
			check_cmd_index(next_cmd_index);
			calc.commands[next_cmd_index++] = OP_BOR;
		}
		else if (tok == "^") {
			check_cmd_index(next_cmd_index);
			calc.commands[next_cmd_index++] = OP_BXOR;
		}
		else if (tok == "~") {
			check_cmd_index(next_cmd_index);
			calc.commands[next_cmd_index++] = OP_BNOT;
		}
		else if (tok == "call") {
			int val; parse_argument(val);
			check_cmd_index(next_cmd_index + 1);
			calc.commands[next_cmd_index++] = OP_CALL;
			calc.commands[next_cmd_index++] = val;
		}
		else if (tok == "ret") {
			check_cmd_index(next_cmd_index);
			calc.commands[next_cmd_index++] = OP_RET;
		}
		else if (tok == "add") {
			check_cmd_index(next_cmd_index);
			calc.commands[next_cmd_index++] = OP_ADD;
		}
		else if (tok == "sub") {
			check_cmd_index(next_cmd_index);
			calc.commands[next_cmd_index++] = OP_SUB;
		}
		else if (tok == "mul") {
			check_cmd_index(next_cmd_index);
			calc.commands[next_cmd_index++] = OP_MUL;
		}
		else {
			std::cerr << "Unknown command '" << tok << "'" << std::endl;   \
			return false;
		}
	}

#undef parse_argument
#undef check_cmd_index

	return true;
}

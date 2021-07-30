#include <iostream>

#include "stack_machine.h"
#include "stack_machine_parser.h"

int main(int argc, char **argv) {
	StackMachine sm;

	// Adapted from https://en.wikipedia.org/wiki/Stack-oriented_programming#Anatomy_of_some_typical_procedures
	const char *const fib7 = \
			"push 7 "\
			"dup dup push 1 eq swap push 0 eq or not "\
			"jz 28 "\
			"dup push 1 sub call 2 swap push 2 sub call 2 add ret";
	const char *prog = fib7;

	if (argc == 2) {
		prog = argv[1];
	}
	else {
		std::cout << "Using following program to compute the 7th fibonacci number:" << std::endl << prog << std::endl;
	}

	if (!StackMachineParser(std::string(prog), sm)) {
		std::cerr << "Invalid program" << std::endl;
		return 1;
	}

	stack_machine_compute(sm, -1);
	int res = 0;
	bool halted = stack_machine_result(sm, res);
	if (halted) {
		std::cout << "result: " << res << std::endl;
	}
	else {
		std::cerr << "error: " << sm.status << std::endl;
	}

	return sm.status;
}

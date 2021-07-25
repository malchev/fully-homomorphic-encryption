#include <iostream>

#include "stack_machine.h"
#include "stack_machine_parser.h"

#ifdef USE_INTERPRETED_TFHE
#include "transpiler/examples/stack_machine/stack_machine_interpreted_tfhe.h"
#include "transpiler/examples/stack_machine/stack_machine_interpreted_tfhe.types.h"
#else
#include "transpiler/examples/stack_machine/stack_machine_tfhe.h"
#include "transpiler/examples/stack_machine/stack_machine_tfhe.types.h"
#endif

constexpr int kMainMinimumLambda = 120;

int main(int argc, char **argv) {
	StackMachine calc;

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

	if (!StackMachineParser(std::string(prog), calc)) {
		std::cerr << "Invalid program" << std::endl;
		return 1;
	}

	

#if 0
	int res = 0;
	int status = BatchCalculator::compute(calc, res);
	if (status == STATUS_HALT) {
		std::cout << res << std::endl;
	}
	else {
		std::cerr << "error " << status << std::endl;
	}

	return status;
#endif
}

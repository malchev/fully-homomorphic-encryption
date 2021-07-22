#include <iostream>

#include "stack_machine.h"
#include "transpiler/examples/stack_machine/stack_machine_bool.h"

int main(int argc, char **argv) {
	StackMachine calc;

	if (argc != 2) {
		std::cerr << "Expecting a program string in argument 1" << std::endl;
		return 1;
	}

	if (!StackMachine::compile(std::string(argv[1]), calc)) {
		std::cerr << "Invalid program" << std::endl;
		return 1;
	}
	
	return 0;
}

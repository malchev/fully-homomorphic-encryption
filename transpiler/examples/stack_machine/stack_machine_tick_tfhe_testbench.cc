#include <iostream>

#include "stack_machine.h"
#include "stack_machine_parser.h"

#include "tfhe/tfhe.h"
#ifdef USE_INTERPRETED_TFHE
#include "transpiler/examples/stack_machine/stack_machine_tick_interpreted_tfhe.h"
#include "transpiler/examples/stack_machine/stack_machine_tick_interpreted_tfhe.types.h"
#else
#include "transpiler/examples/stack_machine/stack_machine_tick_tfhe.h"
#include "transpiler/examples/stack_machine/stack_machine_tick_tfhe.types.h"
#endif

constexpr int kMainMinimumLambda = 120;

int main(int argc, char **argv) {
	StackMachine stack_machine;

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

	if (!StackMachineParser(std::string(prog), stack_machine)) {
		std::cerr << "Invalid program" << std::endl;
		return 1;
	}

	// Generate a keyset.
	TFheGateBootstrappingParameterSet* params =
      new_default_gate_bootstrapping_parameters(kMainMinimumLambda);

	// Generate a "random" key.
	// Note: In real applications, a cryptographically secure seed needs to be
	// used.
	uint32_t seed[] = {314, 1592, 657};
	tfhe_random_generator_setSeed(seed, 3);
	TFheGateBootstrappingSecretKeySet* key =
		new_random_gate_bootstrapping_secret_keyset(params);
	const TFheGateBootstrappingCloudKeySet* cloud_key = &key->cloud;

	FheStackMachine fhe_stack_machine(params);
	fhe_stack_machine.SetEncrypted(stack_machine, key);

	// Round-trip test: decrypt the stack machine and run the calculation:
    std::cout << "Round-trip test: decrypt the stack machine and run the calculation" << std::endl;	
	StackMachine round_trip = fhe_stack_machine.Decrypt(key);

	int res = 0;
	int status = stack_machine_compute(round_trip, -1, res);
	if (status == STATUS_HALT) {
		std::cout << "Result: " << res << std::endl;
	}
	else {
		std::cerr << "error " << status << std::endl;
	}

	return status;

	std::cout << "Starting computation." << std::endl;

	{
		FheInt fhe_status(params);
		FheInt fhe_result(params);
		fhe_stack_machine.SetEncrypted(stack_machine, key);

		XLS_CHECK_OK(stack_machine_tick(fhe_status.get(), fhe_stack_machine.get(), fhe_result.get(), cloud_key));

		int status = fhe_status.Decrypt(key);
		int result = fhe_result.Decrypt(key);

		std::cout << "status: " << status << std::endl;
		std::cout << "result: " << result << std::endl;
	}

	return 0;
}

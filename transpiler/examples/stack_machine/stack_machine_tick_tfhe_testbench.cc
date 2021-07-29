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
#include "xls/common/logging/logging.h"

constexpr int kMainMinimumLambda = 120;

int main(int argc, char **argv) {
	StackMachine next;

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

	if (!StackMachineParser(std::string(prog), next)) {
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
	fhe_stack_machine.SetEncrypted(next, key);

	// Round-trip test: decrypt the stack machine and run the calculation:
	{
		std::cout << "Round-trip test: decrypt the stack machine and run the calculation." << std::endl;	
		StackMachine sm = fhe_stack_machine.Decrypt(key);
		sm = stack_machine_compute(sm, -1);
		int res = 0;
		bool halted = stack_machine_result(sm, res);
		if (halted) {
			std::cout << "result: " << res << std::endl;
		}
		else {
			std::cerr << "error: " << sm.status << std::endl;
		}
	}

	std::cout << "Starting computation." << std::endl;

	for (int gas = 0;; gas++) {
		FheStackMachine fhe_next(params);

		absl::Time start_time = absl::Now();
		double cpu_start_time = clock();
		XLS_CHECK_OK(__stack_machine_tick(fhe_next.get(), fhe_stack_machine.get(), cloud_key));
		double cpu_end_time = clock();
		absl::Time end_time = absl::Now();
		std::cout << "\tComputation done" << std::endl;
		std::cout << "\t\tTotal time: "
			<< absl::ToDoubleSeconds(end_time - start_time) << " secs" << std::endl;
		std::cout << "\t\t  CPU time: "
			<< (cpu_end_time - cpu_start_time) / 1'000'000 << " secs" << std::endl;

		next = fhe_next.Decrypt(key);
		do {
			printf("%-8d %-2d:", gas, next.commands[next.step]);
			for (int n = 0; n < next.top; n++) {
				printf(" %-8d", next.stack[n]);
			}
			printf("\n");
		} while(false);

		fhe_stack_machine.SetEncrypted(next, key);
	}

	return 0;
}

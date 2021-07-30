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

	StackMachine next;

	if (!StackMachineParser(std::string(prog), next)) {
		std::cerr << "Invalid program" << std::endl;
		return 1;
	}
	if (next.status != STATUS_OK) {
		std::cerr << "Newly constructed StackMachine status is not OK: " << next.status << std::endl;
		return next.status;
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
		stack_machine_compute(sm, -1);
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

	for (int gas = 0; next.status == STATUS_OK; gas++) {
		absl::Time start_time = absl::Now();

		std::cout << "\tTick: " << gas << std::endl;
		double cpu_start_time = clock();
		XLS_CHECK_OK(__stack_machine_tick(fhe_stack_machine.get(), cloud_key));
		double cpu_end_time = clock();
		absl::Time end_time = absl::Now();
		std::cout << "\t\tTick " << gas << " done" << std::endl;
		std::cout << "\t\t\tTotal time: "
			<< absl::ToDoubleSeconds(end_time - start_time) << " secs" << std::endl;
		std::cout << "\t\t\tCPU time: "
			<< (cpu_end_time - cpu_start_time) / 1'000'000 << " secs" << std::endl;

		next = fhe_stack_machine.Decrypt(key);

		std::cout << "\t\tStackMachine: " << gas << std::endl;
		do {
			printf("\t\t\t%-8d status: %-2d cmd: %-2d top: %-2d call: %-2d\n", gas, next.status, next.commands[next.step], next.top, next.call);
			printf("\t\t\tprogram:");
			for (int n = 0; n < MAX_CMD; n++) {
				int cmd = next.commands[n];
				printf(" %-8d", cmd);
			}
			printf("\n");
			printf("\t\t\tstack:");
			for (int n = 0; n < next.top; n++) {
				printf(" %-8d", next.stack[n]);
			}
			printf("\n");
			printf("\t\t\tcallstack:");
			for (int n = 0; n < next.call; n++) {
				printf(" %-8d", next.callstack[n]);
			}
			printf("\n");
		} while(false);
	}

	int res = 0;
	bool halted = stack_machine_result(next, res);
	if (halted) {
		std::cout << "result: " << res << std::endl;
	}
	else {
		std::cerr << "error: " << next.status << std::endl;
	}

	return 0;
}

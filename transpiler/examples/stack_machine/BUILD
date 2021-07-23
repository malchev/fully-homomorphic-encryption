load("//transpiler:fhe.bzl", "fhe_cc_library")
load("@bazel_skylib//:bzl_library.bzl", "bzl_library")

licenses(["notice"])

cc_library(
		name = "stack_machine",
		srcs = [ "stack_machine.cc" ],
		hdrs = [ "stack_machine.h" ],
)

fhe_cc_library(
    name = "stack_machine_tfhe",
    src = "stack_machine.cc",
    hdrs = ["stack_machine.h"],
    num_opt_passes = 2,
)

fhe_cc_library(
    name = "stack_machine_interpreted_tfhe",
    src = "stack_machine.cc",
    hdrs = ["stack_machine.h"],
    num_opt_passes = 2,
    transpiler_type = "interpreted_tfhe",
)

fhe_cc_library(
    name = "stack_machine_bool",
    src = "stack_machine.cc",
    hdrs = ["stack_machine.h"],
    num_opt_passes = 2,
    transpiler_type = "bool",
)

cc_library(
		name = "stack_machine_parser",
		srcs = [ "stack_machine_parser.cc" ],
		copts = ["-DSTACK_MACHINE_NOIMPL"],
		hdrs = [ "stack_machine_parser.h", "stack_machine.h" ],
)

cc_binary(
		name = "stack_machine_testbench",
		srcs = [ "stack_machine_testbench.cc" ],
		copts = ["-DSTACK_MACHINE_NOIMPL"],
		deps = [
				":stack_machine",
				":stack_machine_parser",
		],
)

cc_binary(
    name = "stack_machine_bool_testbench",
    srcs = ["stack_machine_bool_testbench.cc"],
    deps = [
        ":stack_machine_bool",
		":stack_machine_parser",
        "//transpiler/data:boolean_data",
        "@com_google_absl//absl/container:fixed_array",
        "@com_google_xls//xls/common/logging",
        "@com_google_xls//xls/common/status:status_macros",
    ],
)

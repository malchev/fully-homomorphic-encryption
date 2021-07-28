load("//transpiler:fhe.bzl", "fhe_cc_library")
load("@bazel_skylib//:bzl_library.bzl", "bzl_library")

licenses(["notice"])

cc_library(
    name = "stack_machine",
    srcs = [ "stack_machine.cc" ],
    hdrs = [ "stack_machine.h", "stack_machine_switch_impl.h" ],
)

fhe_cc_library(
    name = "stack_machine_tick_tfhe",
    src = "stack_machine_tick.cc",
    copts = ["-DEXCLUDE_STACK_MACHINE_IMPLEMENTATION"],
    hdrs = [ "stack_machine_tick.h", "stack_machine.h", "stack_machine_switch_impl.h" ],
    num_opt_passes = 2,
)

fhe_cc_library(
    name = "stack_machine_tick_interpreted_tfhe",
    src = "stack_machine_tick.cc",
    copts = ["-DEXCLUDE_STACK_MACHINE_IMPLEMENTATION"],
    hdrs = [ "stack_machine_tick.h", "stack_machine.h", "stack_machine_switch_impl.h" ],
    num_opt_passes = 2,
    transpiler_type = "interpreted_tfhe",
)

fhe_cc_library(
    name = "stack_machine_tick_bool",
    src = "stack_machine_tick.cc",
    copts = ["-DEXCLUDE_STACK_MACHINE_IMPLEMENTATION"],
    hdrs = ["stack_machine_tick.h", "stack_machine.h", "stack_machine_switch_impl.h" ],
    num_opt_passes = 2,
    transpiler_type = "bool",
)

cc_library(
    name = "stack_machine_parser",
    srcs = [ "stack_machine_parser.cc" ],
    copts = [ "-DEXCLUDE_STACK_MACHINE_IMPLEMENTATION" ],
    hdrs = [ "stack_machine_tick.h", "stack_machine.h", "stack_machine_parser.h", "stack_machine_switch_impl.h" ],
)

cc_binary(
    name = "stack_machine_testbench",
    srcs = [ "stack_machine_testbench.cc" ],
    deps = [
        ":stack_machine",
        ":stack_machine_parser",
    ],
)

cc_binary(
    name = "stack_machine_tick_bool_testbench",
    srcs = ["stack_machine_tick_bool_testbench.cc"],
    deps = [
        ":stack_machine_tick_bool",
        ":stack_machine_parser",
        "//transpiler/data:boolean_data",
        "@com_google_absl//absl/container:fixed_array",
        "@com_google_xls//xls/common/logging",
        "@com_google_xls//xls/common/status:status_macros",
    ],
)

cc_binary(
    name = "stack_machine_tick_tfhe_testbench",
    srcs = ["stack_machine_tick_tfhe_testbench.cc"],
    deps = [
        ":stack_machine_tick_tfhe",
        ":stack_machine_parser",
        "@com_google_absl//absl/container:fixed_array",
        "@com_google_xls//xls/common/logging",
        "@com_google_xls//xls/common/status:status_macros",
    ],
)

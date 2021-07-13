load("//transpiler:fhe.bzl", "fhe_cc_library")
load("@bazel_skylib//:bzl_library.bzl", "bzl_library")

licenses(["notice"])

cc_library(
		name = "batch_calculator",
		srcs = [ "batch_calculator.cc" ],
		hdrs = [ "batch_calculator.h" ],
)

fhe_cc_library(
    name = "batch_calculator_bool",
    src = "batch_calculator.cc",
    hdrs = ["batch_calculator.h"],
    num_opt_passes = 2,
    transpiler_type = "bool",
)

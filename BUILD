package(default_visibility = ["//visibility:public"])

load(
    "@envoy//bazel:envoy_build_system.bzl",
    "envoy_cc_binary",
    "envoy_cc_library",
    "envoy_cc_test",
)

envoy_cc_binary(
    name = "envoy",
    repository = "@envoy",
    deps = [
        "//extensions/transport_sockets/tls:config",
        "//extensions/filters/listener/tls_inspector:config",
        "//extensions/common/crypto:utility_lib",
        "@envoy//source/exe:envoy_main_entry_lib",
    ],
)

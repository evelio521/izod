cc_library(
    name = 'gtest',
    srcs = [
        'src/gtest.cc',
        'src/gtest-filepath.cc',
        'src/gtest-port.cc',
        'src/gtest-printers.cc',
        'src/gtest-test-part.cc',
        'src/gtest-death-test.cc',
    ],
    hdrs = glob(['include/gtest/*.h',
                'include/gtest/internal/*.h',
                'src/gtest-internal-inl.h']),
    includes = ['include',],
    deps = [
        '//base:base',
        '//thirdlibs/gflags:gflags',
    ],
    linkopts = ['-lpthread'],
    defines = ['GTEST_HAS_TR1_TUPLE', 'GTEST_ENV_HAS_STD_TUPLE_'],
    visibility = ["//visibility:public"],
)

cc_library(
    name = 'gtest_main',
    srcs = [
        'src/gtest_main.cc',
    ],
    deps = [
        ':gtest',
        '//thirdlibs/gflags:gflags',
    ],
    visibility = ["//visibility:public"],
)

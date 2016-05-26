package(default_visibility = ["//visibility:public"])

cc_library(
    name = 'gmock',
    srcs = [
        #'src/gmock-all.cc',
        'src/gmock-cardinalities.cc',
        'src/gmock.cc',
        'src/gmock-internal-utils.cc',
        #'src/gmock_main.cc',
        'src/gmock-matchers.cc',
        'src/gmock-spec-builders.cc'
    ],
    deps = [
        '//thirdlibs/gmock/gtest:gtest',
    ],
    hdrs = glob(['include/gmock/*.h', 'include/gmock/internal/*.h']),
    includes = ['.', 'include'],
)

cc_library(
    name = 'gmock_main',
    srcs = [
        'src/gmock_main.cc',
    ],
    deps = [
        ':gmock',
        '//thirdlibs/gmock/gtest:gtest',
    ],
    hdrs = glob(['include/gmock/*.h', 'include/gmock/internal/*.h']),
    includes = ['.', 'include'],
)

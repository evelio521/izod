cc_library(
    name = 'gtest',
    deps = [
        '//thirdlibs/gmock/gtest:gtest',
    ],
    hdrs = glob(['*.h']),
    visibility = ["//visibility:public"],
)

cc_library(
    name = 'gtest_main',
    deps = [
        ':gtest',
        '//thirdlibs/gmock/gtest:gtest_main',
    ],
    hdrs = glob(['*.h']),
    visibility = ["//visibility:public"],
)

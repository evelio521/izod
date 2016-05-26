cc_library(
    name = 'status',
    srcs = ['status.cc'],
    hdrs = glob(['status.h']),
    deps = [
        '//base:base',
     ],
     alwayslink = True,
)
cc_test(
    name = 'status_test',
    srcs = ['status_test.cc'],
    deps = [
        ':status',
        '//thirdlibs/gtest:gtest_main',
     ],
)

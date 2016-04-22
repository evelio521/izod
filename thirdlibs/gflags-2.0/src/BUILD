cc_library(
    name = 'gflags',
    srcs = [
        'gflags.cc',
        'gflags_reporting.cc',
        'gflags_completions.cc',
    ],
    hdrs = glob([
        '*.h',
         "**/*.h"
    ]),
    includes = ['.'],
    copts = ['-fPIC'],
    linkopts = ["-lpthread", '-fPIC'],
    visibility = ["//visibility:public"],
    linkstatic = 1,
)

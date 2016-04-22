package(default_visibility = ["//visibility:public"])

cc_library(
    name = 'util',
    srcs = [
    ],
    deps = [
        '//base:base',
    ],
)

cc_library(
    name = 'event_base_loop',
    srcs = [
        'event_base_loop.cc',
    ],
    deps = [
        '//base:base',
        ':util',
        '//thirdlibs/event:event',
    ],
    hdrs = glob(['event_base_loop.h']),
)
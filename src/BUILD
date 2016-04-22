package(default_visibility = ["//visibility:public"])

cc_library(
    name = 'util',
    srcs = [
        'util.cc',
    ],
    hdrs = glob(['util.h']),
    deps = [
        '//base:base',
        '//thirdlibs/event:event',
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

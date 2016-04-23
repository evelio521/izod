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
cc_library(
    name = 'response',
    srcs = [
        'response.cc',
    ],
    deps = [
        ':util',
        ':request',
        '//base:base',
        '//thirdlibs/event:event',
    ],
    hdrs = glob(['response.h']),
)
cc_library(
    name = 'request',
    srcs = [
        'request.cc',
    ],
    deps = [
        '//base:base',
        ':util',
        '//thirdlibs/event:event',
    ],
    hdrs = glob(['request.h']),
)
cc_library(
    name = 'handlers',
    srcs = [
        'handlers.cc',
    ],
    deps = [
        ':util',
        ':request',
        ':response',
        '//base:base',
        '//thirdlibs/event:event',
    ],
    hdrs = glob(['handlers.h']),
)
cc_library(
    name = 'modules',
    srcs = [
        'modules.cc',
    ],
    deps = [
        ':util',
        ':request',
        ':response',
        ':handlers',
        ':event_base_loop',
        '//base:base',
        '//base:thread_pool',
        '//thirdlibs/event:event',
    ],
    hdrs = glob(['modules.h']),
)
cc_binary(
    name = 'server_main',
    srcs = ['server_main.cc'],
    deps = [':modules',
            ':handlers',
            '//base:base',
            '//thirdlibs/event:event',
            ],
)

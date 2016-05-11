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
    hdrs = glob(['*.h']),
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
    hdrs = glob(['*.h']),
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
    hdrs = glob(['*.h']),
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
    hdrs = glob(['*.h']),
)

cc_library(
    name = 'client',
    srcs = [
        'client.cc',
    ],
    deps = [
        ':event_base_loop',
        '//base:base',
        '//thirdlibs/event:event',
    ],
    hdrs = glob(['*.h']),
)

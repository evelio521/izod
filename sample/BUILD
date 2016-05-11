package(default_visibility = ["//visibility:public"])

cc_library(
    name = 'def',
    hdrs = glob(['defs.h']),
)

cc_library(
    name = 'testhandler',
    srcs = [
        'testhandlers.cc',
    ],
    deps = [
        ':def',
        '//src:util',
        '//src:request',
        '//src:response',
        '//src:handlers',
    ],
    hdrs = glob(['testhandlers.h']),
)

cc_library(
    name = 'testmodule',
    srcs = [
        'testmodules.cc',
    ],
    deps = [
        ':def',
        ':testhandler',
        '//src:request',
        '//src:response',
        '//src:modules',
    ],
    hdrs = glob(['testmodules.h']),
)

cc_binary(
    name = 'server_main',
    srcs = [
        'server_main.cc',
    ],
    deps = [
        ':def',
        ':testhandler',
        ':testmodule',
    ],
)

cc_binary(
    name = 'client_main',
    srcs = [
        'client_main.cc',
    ],
    deps = [
        ':def',
        '//src:client',
    ],
)

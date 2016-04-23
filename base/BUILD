package(default_visibility = ["//visibility:public"])

cc_library(
    name = 'base',
    srcs = [
        'string_piece.cc',
        'callback.cc',
        'string_util.cc',
        'string16.cc',
        'utf_string_conversions.cc',
        'utf_string_conversion_utils.cc',
        'icu_utf.cc',
        'at_exit.cc',
        'mutex.cc',
        'time.cc',
        'atomicops_internals_x86_gcc.cc',
        'platform_thread_posix.cc',
        'thread.cc',
        'g_fmt.cc',
        'dtoa.cc',
    ],
    deps = [
        '//thirdlibs/glog:glog',
        '//thirdlibs/gflags:gflags',
    ],
    hdrs = glob(['*.h']),
)
cc_library(
    name = 'thread_pool',
    srcs = [
        'fixed_size_thread_pool.cc',
    ],
    hdrs = [
        'fixed_size_thread_pool.h',
    ],
    deps = [
        ':base'
    ],
)

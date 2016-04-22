package(default_visibility=['//visibility:public'])

cc_library(name='event',
    deps=[':event_core',
            ':event_extra',
            ],
    hdrs = glob(['*.h', '**/*.h', 'include/event2/*.h',]),
)

cc_library(name='event_core',
    srcs=[
    'event.c',
    'evthread.c',
    'buffer.c',
    'bufferevent.c',
    'bufferevent_sock.c',
    'bufferevent_filter.c',
    'bufferevent_pair.c',
    'listener.c',
    'bufferevent_ratelim.c',
    'evmap.c',
    'log.c',
    'evutil.c',
    'evutil_rand.c',
    'strlcpy.c',
    'select.c',
    'poll.c',
    'epoll.c',
    'signal.c',
    ],
    textual_hdrs = ['arc4random.c'],
    hdrs = glob(['*.h', '**/*.h', 'include/event2/*.h',]),
    includes = ["include/"],
    linkopts = ['-lrt', '-ldl', '-lpthread'],
)

cc_library(name='event_extra',
    srcs=[
    'event_tagging.c',
    'http.c',
    'evdns.c',
    'evrpc.c',
    'evthread_pthread.c',
    ],
    hdrs = glob(['*.h', '**/*.h', 'include/event2/*.h',]),
    deps=[':event_core', ],
)

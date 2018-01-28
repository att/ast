# TODO: Can we remove these tests? It seems like any system we want to support
# should provide all of these items.

if cc.has_header('arpa/inet.h')
    feature_data.set('_hdr_arpa_inet', 1)
endif

if cc.has_header('netinet/in.h')
    feature_data.set('_hdr_netinet_in', 1)
endif

if cc.has_function('socket', prefix: '#include <sys/socket.h>')
    feature_data.set('_sys_socket', 1)
endif

if cc.has_function('getsockname', prefix: '#include sys/socket.h')
    feature_data.set('_lib_getsockname', 1)
endif

if cc.has_function('getsockopt', prefix: '#include sys/socket.h')
    feature_data.set('_lib_getsockopt', 1)
endif

if cc.has_function('inet_ntoa', prefix: '#include arpa/inet.h')
    feature_data.set('_lib_inet_ntoa', 1)
endif

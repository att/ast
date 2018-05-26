# TODO: Can we remove these tests? It seems like any system we want to support
# should provide all of these items.

feature_data.set10('_hdr_arpa_inet', cc.has_header('arpa/inet.h'))
feature_data.set10('_hdr_netinet_in', cc.has_header('netinet/in.h'))

feature_data.set10('_sys_socket', cc.has_function('socket', prefix: '#include <sys/socket.h>'))
feature_data.set10('_lib_getsockname',
    cc.has_function('getsockname', prefix: '#include <sys/socket.h>'))
feature_data.set10('_lib_getsockopt',
    cc.has_function('getsockopt', prefix: '#include <sys/socket.h>'))
feature_data.set10('_lib_inet_ntoa', cc.has_function('inet_ntoa', prefix: '#include <arpa/inet.h>'))

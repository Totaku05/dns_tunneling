import sys, getopt
from ctypes import *
import os

dnstun_client_version = 4.0

def close_files(src, dst_name, dst, libc):
    src.close()
    if(dst_name != None):
        libc.fclose(dst)

def print_usage(name_of_program):
    print('DNS tunneling client version:', dnstun_client_version)
    print('Usage:', name_of_program)
    print('[-h/--help - Show the possible options]')
    print('[-a <address>/--address=<address> - Enter the server address (the default address is '
          'http://localhost:8000/)]')
    print('[-s <file name>/--source_file=<file name> - Enter the source file name (the default name is'
          ' test)]')
    print('[-d <file name>/--destination_file=<file name> - Enter the destination file name (the default'
          ' output is to the console)]')


if __name__ == "__main__":
    src_name = 'test'
    dst_name = None
    server = 'http://localhost:8000/'

    try:
        opts, args = getopt.getopt(sys.argv[1:], "ha:s:d:", ["help", "address=", "source_file=", "destination_file="])
    except getopt.GetoptError:
        print('Wrong arguments.')
        sys.exit(-1)

    for opt, arg in opts:
        if(opt in ('-h', '--help')):
            print_usage(sys.argv[0])
            sys.exit()
        elif(opt in ('-a', '--address')):
            server = arg
        elif(opt in ('-s', '--source_file')):
            src_name = arg
        elif(opt in ('-d', '--destination_file')):
            dst_name = arg

    src = open(src_name, 'r')

    libc = CDLL(None)

    if(dst_name == None):
        dst = c_void_p.in_dll(libc, 'stdout')
    else:
        dst = libc.fopen(dst_name.encode(), 'w'.encode())

    lib_client = cdll.LoadLibrary('./libdnstun_client.so')

    dnstun_client_init = lib_client.dnstun_client_init
    dnstun_client_init.argtypes = (POINTER(c_void_p), c_void_p)
    dnstun_client_init.restypes = c_uint

    dnstun_client_deinit = lib_client.dnstun_client_deinit
    dnstun_client_deinit.argtypes = [c_void_p]
    dnstun_client_deinit.restypes = c_uint

    dnstun_client_send_request = lib_client.dnstun_client_send_request
    dnstun_client_send_request.argtypes = (c_void_p, c_char_p)
    dnstun_client_send_request.restypes = c_uint

    client = c_void_p(0)

    status = dnstun_client_init(byref(client), dst)
    if(status):
        close_files(src, dst_name, dst, libc)

    for line in src:
        req_type, req_name = line.split(', ')
        url = '{}?type={}&name={}'.format(server, req_type, req_name[:-1])
        dnstun_client_send_request(client, url.encode())

    dnstun_client_deinit(client)
    close_files(src, dst_name, dst, libc)

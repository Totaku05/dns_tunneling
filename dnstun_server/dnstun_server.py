from http.server import BaseHTTPRequestHandler, HTTPServer
from urllib.parse import urlparse, parse_qs
import dns.resolver
import sys, getopt

class DNSTunServer(BaseHTTPRequestHandler):
    def __set_headers(self):
        self.send_response(200)
        self.send_header('Content-type', 'text/plain')
        self.end_headers()

    def __process_request(self, request_type, name):
        response = request_type + ', ' + name + ', '
        if(request_type not in ('MX', 'TXT', 'A')):
            print('This request has the wrong type.')
            response += '1'
        else:
            try:
                answers = dns.resolver.query(name, request_type)
                response += '0\n'
                for data in answers:
                    if(request_type == 'A'):
                        print(data.address)
                        response += data.address + '\n'
                    elif(request_type == 'TXT'):
                        print(data.strings[0].decode())
                        response += data.strings[0].decode() + '\n'
                    elif(request_type == 'MX'):
                        print(data.preference, data.exchange)
                        response += str(data.preference) + ' ' + data.exchange.to_text() + '\n'
            except dns.resolver.NoAnswer:
                print('This request was not successful.')
                response += '1'
            except dns.resolver.NXDOMAIN:
                print('This request was not successful.')
                response += '1'
        self.wfile.write(response.encode());

    def do_GET(self):
        self.__set_headers()
        query_components = parse_qs(urlparse(self.path).query)
        request_type = query_components["type"][0]
        name = query_components["name"][0]
        print('New request:', request_type, name)
        self.__process_request(request_type, name)

dnstun_server_version = 1.0

def print_usage(name_of_program):
    print('DNS tunneling server version:', dnstun_server_version)
    print('Usage:', name_of_program)
    print('[-h/--help - Show the possible options]')
    print('[-p <port>/--port=<port> - Enter the port (the default port is 8000)]')

if __name__ == "__main__":
    port = 8000

    try:
        opts, args = getopt.getopt(sys.argv[1:], "hp:", ["help", "port="])
    except getopt.GetoptError:
        print('Wrong arguments.')
        sys.exit(-1)

    for opt, arg in opts:
        if(opt in ('-h', '--help')):
            print_usage(sys.argv[0])
            sys.exit()
        elif(opt in ('-p', '--port')):
            port = int(arg)

    server_address = ('', port)
    httpd = HTTPServer(server_address, DNSTunServer)
    httpd.serve_forever()

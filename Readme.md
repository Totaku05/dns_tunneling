DNSTUN_CLIENT APPLICATION
For the dnstun_client application, you need to install the curl library.
To build the dnstun_client application, you need to go to the dnstun_client folder and execute the make command.
You can use the executable file named as dnstun_client with the following options:
[-h/--help - Show the possible options]
[-s/--server <address> - Enter the server address (the default address is http://localhost:8000/)]
[-src/--source_file <file name> - Enter the source file name (the default name is test)]
[-dst/--destination_file <file name> - Enter the destination file name (the default output is to the console)]

C DNSTUN_SERVER APPLICATION
For the dnstun_server application, you need to install the c-ares library.
To build the dnstun_server application, you need to go to the dnstun_server folder and execute the make command.
You can use the executable file named as dnstun_server with the following options:
[-h/--help - Show the possible options]
[-p/--port <port> - Enter the port (the default port is 8000)]

PYTHON DNSTUN_SERVER APPLICATION
For the dnstun_server application, you need to install the dnspython module.
You can install this module by following these steps:
    1)git clone https://github.com/rthalley/dnspython
    2)cd dnspython
    3)python3 setup.py install
You can find the dnstun_server.py file in the dnstun_server folder.
You can use the python dnstun_server with the following options:
[-h/--help - Show the possible options]
[-p <port>/--port=<port> - Enter the port (the default port is 8000)]

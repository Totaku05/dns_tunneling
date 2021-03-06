C DNSTUN_CLIENT APPLICATION
For the dnstun_client application, you need to install the curl library.
To build the dnstun_client application and the libdnstun_client.so shared library, you need to go to the dnstun_client folder and execute the make command.
You can use the executable file named as dnstun_client with the following options:
[-h/--help - Show the possible options]
[-a/--address <address> - Enter the server address (the default address is http://localhost:8000/)]
[-s/--source_file <file name> - Enter the source file name (the default name is test)]
[-d/--destination_file <file name> - Enter the destination file name (the default output is to the console)]

PYTHON DNSTUN_CLIENT APPLICATION
For the python dnstun_client application, you need to build the libdnstun_client.so shared library.
You can find the dnstun_client.py file in the dnstun_client folder.
You can use the python dnstun_client with the following options:
[-h/--help - Show the possible options]
[-a <address>/--address=<address> - Enter the server address (the default address is http://localhost:8000/)]
[-s <file name>/--source_file=<file name> - Enter the source file name (the default name is test)]
[-d <file name>/--destination_file=<file name> - Enter the destination file name (the default output is to the console)]

C DNSTUN_SERVER APPLICATION
For the dnstun_server application, you need to install the c-ares library.
To build the dnstun_server application, you need to go to the dnstun_server folder and execute the make command.
You can use the executable file named as dnstun_server with the following options:
[-h/--help - Show the possible options]
[-p/--port <port> - Enter the port (the default port is 8000)]
[-l/--level <level> - Enter the level of logging (the default level is info). You can enter the following levels of logging: debug, info, error]
[-n/--number_of_threads <number> - Enter the number of threads (the default number is 10). The number of threads must be greater than 0]
[-m/--max_number_of_items <number> - Enter the max number of items (the default number is 10). The max number of items must be greater than 0]

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

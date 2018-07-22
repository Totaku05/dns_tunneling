LIBS:=-lcurl
SHARED_LIB:=libdnstun_client.so
SRC:=main.c
TARGET:=dnstun_client

all:
	gcc -Wall -fpic dnstun_client.c -shared $(LIBS) -o $(SHARED_LIB)
	gcc -Wl,-rpath=. $(SRC) -o $(TARGET) $(SHARED_LIB)

clean:
	rm -f $(TARGET) $(SHARED_LIB)

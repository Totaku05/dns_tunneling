LIBS:=-lcares
SRC:=main.c mongoose.c dnstun_resolver.c dnstun_server.c
TARGET:=dnstun_server

all:
	gcc $(SRC) $(LIBS) -o $(TARGET)

clean:
	rm -f $(TARGET)

LIBS:=-lcares
SRC:=main.c mongoose.c dnstun_resolver.c dnstun_cache.c dnstun_server.c
TARGET:=dnstun_server

all:
	gcc -pthread -DMG_ENABLE_CALLBACK_USERDATA $(SRC) $(LIBS) -o $(TARGET)

clean:
	rm -f $(TARGET)

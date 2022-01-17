CC=gcc
CFLAGS=-fPIC -g -Wall 
LDFLAGS=-lcurl
SOURCES=sktest.c median.c
OBJECTS=$(SOURCES:.c=.o)
LIB=libsktest.so

%.c.o: %.c
	$(CC) $(CFLAGS) $< -o $@

$(LIB): $(OBJECTS)
	$(CC) -shared $(OBJECTS) $(LDFLAGS) -o $@ 
	$(RM) -rf $(OBJECTS)

all:
	$(LIB)
	echo $(OBJECTS)

clean:
	$(RM) -rf $(BIN) $(OBJECTS) *.dSYM

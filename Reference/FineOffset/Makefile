CC=gcc
CFLAGS=-c -Wall
ifeq ($(DB), 1)
LDFLAGS=-lbcm2835 -lmysqlclient
SOURCES=fo_main.c fo_process.c fo_print.c fo_mysql.c fo_util.c fo_bmp085.c fo_bcm2835.c
else
ifeq ($(DB), 2)
LDFLAGS=-lbcm2835 -lsqlite3
SOURCES=fo_main.c fo_process.c fo_print.c fo_sqlite3.c fo_util.c fo_bmp085.c fo_bcm2835.c
else
LDFLAGS=-lbcm2835
SOURCES=fo_main.c fo_process.c fo_print.c fo_datafile fo_util.c fo_bmp085.c fo_bcm2835.c
endif
endif
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=fopi

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) -lm $(OBJECTS) $(LDFLAGS) -o fopi

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *.o fopi

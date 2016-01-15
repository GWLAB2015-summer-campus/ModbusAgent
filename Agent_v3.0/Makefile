CC = gcc

INC = -I/usr/local/include/modbus
LIBS = -lmodbus

.SUFFIXES : .c .o
.c.o :
	$(CC) -c $< $(LIBS) $(INC)

ALL	= serv cli cli_test

all : $(ALL)

serv : serv.o usrdef_modbus_receive.o
	$(CC) -o $@ $^ $(LIBS) $(INC)

cli : cli.o
	$(CC) -o $@ $^ $(LIBS) $(INC)

cli_test : cli_test.o
	${CC} -o $@ $^ $(LIBS) $(INC)

clean :
	rm -rf *.o $(ALL)

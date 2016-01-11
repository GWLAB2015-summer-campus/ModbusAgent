CC = gcc

INC = -I/usr/local/include/modbus
LIBS = -lmodbus

SERV = serv.c
CLI = cli.c
FUNC = func.c

SERV_OB = ${SERV:.c=.o}
CLI_OB= ${CLI:.c=.o}
FUNC_OB = ${FUNC:.c=.o}

SERV_OUT = serv
CLI_OUT = cli

$(FUNC_OB)	:
	dep	:
		gccmakedep $(INC) $(LIBS)

$(SERV_OUT) : $(SERV_OB)
	$(CC) -o $(SERV_OUT) $(SERV_OB) $(FUNC_OB) $(LIBS) $(INC)


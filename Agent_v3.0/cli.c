#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <modbus.h>

#include "unit-test.h"

enum { FUNC03, FUNC06, FUNC10 };

#define QUERY_SIZE 512 
#define SLAVE_NUM 247

// connect to slave module
int connectToSlave(modbus_t *ctx);

// check string 'code' is Hex code
int isHexCode( char *code );

// Print all data
int PrintAll(modbus_t *ctx, uint16_t *tab_rp_registers, int num);

// generate random query
int RandomQuery(char *query, int FunctionCode);	

int main(int argc, char *argv[])
{
	srand((unsigned)time(NULL));
	int ctxnum = 0;
	int i;
	int sel = 1;

	modbus_t *ctx[SLAVE_NUM];			// modbus structure, save connect data
	char portName[SLAVE_NUM][100];	// for save port name
	int slaveID[SLAVE_NUM];			// for save slave id
	int bitRate[SLAVE_NUM];			// for save bit rate
	char parity[SLAVE_NUM];			// for save parity option

	for ( i=0 ; i<SLAVE_NUM ; i++ )
		ctx[i] = NULL;

	if ( argc != 1 )
	{
		printf("Usage : %s\n", argv[0] );
		exit(1);
	}
/*
	if ( argc == 2 )
		if ( !strcmp( argv[1], "ON" ))
			modbus_set_debug(ctx, TRUE);
*/
	while ( sel != -2 )
	{
		for ( i=0 ; i<ctxnum ; i++ )
			printf("%4d. %s	%d	%d	%c\n", i, portName[i], slaveID[i], bitRate[i], parity[i] );
		printf("  -1. make new connect\n");
		printf("  -2. quit\n");
		printf(" sel	: ");
		scanf("%d", &sel);

		switch ( sel )
		{
			case -1 :
				{
					if ( ctxnum == 246 )
						printf("\nconnection is full!!!\n\n");
					printf("\n\tinput port name : ");
					scanf("%s", portName[ctxnum]);
					printf("\tinput slave ID	: ");
					scanf("%d", &slaveID[ctxnum]);
					printf("\tinput bit rate : ");
					scanf("%d", &bitRate[ctxnum]);
					printf("\tinput parity : ");
					fgetc(stdin);
					parity[ctxnum] = fgetc(stdin);

					ctx[ctxnum] = modbus_new_rtu(portName[ctxnum], bitRate[ctxnum], parity[ctxnum], 8, 1);
					// make rtu connection

					if (ctx[ctxnum] == NULL) {          // error
						fprintf(stderr, "Unable to allocate libmodbus context\n");
						return -1;
					}

					modbus_set_error_recovery(ctx[ctxnum],
							MODBUS_ERROR_RECOVERY_LINK |
							MODBUS_ERROR_RECOVERY_PROTOCOL);

					modbus_set_slave(ctx[ctxnum], slaveID[ctxnum]);       // set slave number

					if (modbus_connect(ctx[ctxnum]) == -1) {
						fprintf(stderr, "Connection failed: %s\n",
								modbus_strerror(errno));
						modbus_free(ctx[ctxnum]);
						return -1;
					}
					ctxnum++;
					printf("\n");
				}
			break;
		case -2 :
			printf("exit client module (master)\n");
			break;
		default :
			if ( ctx[sel] == NULL || sel <-2 || sel>246  )
				printf("\ninput correct number.\n\n");		// error input
			else
				connectToSlave(ctx[sel]);	// cennect to slave
		}
	}

	/* Close the connection */
	for ( i=0 ; i<SLAVE_NUM; i++ )
	{
		if ( ctx[i] != NULL )
		{
			modbus_close(ctx[i]);
			modbus_free(ctx[i]);
		}
	}
	return 0;
}

int connectToSlave(modbus_t *ctx)
{
	uint16_t *tab_rp_registers; // for getting data from server( slave module )
	int i,j;                    // for repeat
	int nb_points;              // number of register
	int rc;                     // register counter

	char query[QUERY_SIZE];     // for getting query

	char sSlaveNum[3];
	char sFuncCode[3];
	char sRegiAdd[5];
	char sRegiNum[5];
	char sData[5];
	char svData[MODBUS_MAX_WRITE_REGISTERS][5];
	// MODBUS_MAX_WRITE_REGISTERS : set value in modbus.h
	char sWriteNum[3];                      // str for read data in query

	uint16_t nSlaveNum;
	uint16_t nFuncCode;
	uint16_t nRegiAdd;
	uint16_t nRegiNum;
	uint16_t nData;
	uint16_t nvData[MODBUS_MAX_WRITE_REGISTERS];
	uint8_t nWriteNum;                      // variable for read data in query

	/* Allocate and initialize the memory to store the registers */
	nb_points = (UT_REGISTERS_NB > UT_INPUT_REGISTERS_NB) ?
		UT_REGISTERS_NB : UT_INPUT_REGISTERS_NB;
	tab_rp_registers = (uint16_t *) malloc(nb_points * sizeof(uint16_t));
	memset(tab_rp_registers, 0, nb_points * sizeof(uint16_t));

	while ( 1 )
	{
		memset(sSlaveNum, 0, sizeof(sSlaveNum) );
		memset(sFuncCode, 0, sizeof(sFuncCode) );
		memset(sRegiAdd, 0, sizeof(sRegiAdd) );
		memset(sRegiNum, 0, sizeof(sRegiNum) );
		memset(sData, 0, sizeof(sData) );
		for ( i=0 ; i< MODBUS_MAX_WRITE_REGISTERS ; i++ )
			memset(svData[i], 0, sizeof(svData[i]) );
		memset(sWriteNum, 0, sizeof(sWriteNum) );
		memset(query, 0, QUERY_SIZE);
		// memory init

		printf("\tinput Query ( \"q\" exit, \"all\" show all data, \"r\" random query )\n");
		printf("\tQUERY : ");
		scanf( "%s", query );
		printf("\n");

		if ( !strcmp( query, "q" ) )    // quit message
			break;  // while break
		else if ( !strcmp( query, "all" ) )
		{
			PrintAll(ctx, tab_rp_registers, UT_REGISTERS_NB);      // print all data
			continue;
		}
		else if ( !strcmp( query, "r") )                // random code
		{
			printf("\t\"r3\" : 0x03 \"r6\" : 0x06   \"r1\" : 0x10\n\n");
			continue;
		}
		else if ( !strcmp( query, "r3") )
			RandomQuery( query, FUNC03 );       // make random code 0x03
		else if ( !strcmp( query, "r6") )
			RandomQuery( query, FUNC06 );       // make random code 0x06
		else if ( !strcmp( query, "r1") )
			RandomQuery( query, FUNC10 );       // make random code 0x10
		else if ( !isHexCode(query) || strlen(query)<12 )   // if incorrect query inputed
		{
			printf("\tplease input correct query\n\n");
			continue;
		}

		memcpy( sSlaveNum, (char *)&(query[0]), 2 );
		memcpy( sFuncCode, (char *)&(query[2]), 2 );
		memcpy( sRegiAdd, (char *)&(query[4]), 4 );
		// get Slave ID, Function code, Register start address from query

		nSlaveNum = strtol( sSlaveNum, NULL, 16 );
		nFuncCode = strtol( sFuncCode, NULL, 16 );
		nRegiAdd = strtol( sRegiAdd, NULL, 16);
		// translate string to uint variable

		switch ( nFuncCode )
		{
			/**/            case 0x03:      // 3    read multiple data in RW register
			/**/            case 0x04:      // 4    read multiple data in RO register
				{
					if ( strlen(query) != 12 )
						// func 0x03, 0x04 must have 16 byte len
						// last 4 byte is CRC
						// 16 - 4 = 12
					{
						printf("\tplease input correct query\n\n");
						break;          // switch break
					}

					memcpy( sRegiNum, (char *)&(query[8]), 4 );

					nRegiNum = strtol( sRegiNum, NULL, 16);

					rc = modbus_read_registers(ctx, nRegiAdd, nRegiNum, tab_rp_registers);
					// func 0x03 in modbus library

					if ( rc < 1 )
					{
						printf("read operation error\n");       // error
						break;                  // switch break
					}
					printf("\t----------------------------------------  Data  ----------------------------------------\n");
					printf("\t\t 0x00 0x01 0x02 0x03 0x04 0x05 0x06 0x07 0x08 0x09 0x0A 0x0B 0x0C 0x0D 0x0E 0x0F\n");
					printf("\t----------------------------------------------------------------------------------------");
					for (i=0 , j=nRegiAdd%0x10; i<nRegiNum+j ; i++ )
					{
						if ( i%16 == 0 )
							printf("\n\t0x%02X\t", nRegiAdd-j+i);
						if ( i >= j && i < j+nRegiNum )
							printf(" %04X", *(tab_rp_registers+i-j));
						else
							printf("     ");
					}
					printf("\n\t----------------------------------------------------------------------------------------\n\n");
				}
				break;  // switch break
			/**/            case 0x06:      // 6    write one data in RW register
				{
					if ( strlen(query) != 12 )
						// func 0x06 must have 16 byte len
						// last 4 byte is CRC
						// 16 - 4 = 12
					{
						printf("\tplease input correct query\n\n");
						break;  // switch break
					}

					memcpy( sData, (char *)&(query[8]), 4 );

					nData = strtol( sData, NULL, 16);

					rc = modbus_write_register(ctx, nRegiAdd, nData);
					// func 0x06 in modbus library

					if ( rc != 1 )
					{
						printf("write operation error\n\n");
						break;          // switch break
					}

					printf("\n");
				}
				break;
			/**/            case 0x10:      // 16   write multiple data in RW register
				{
					memcpy( sRegiNum, (char *)&(query[8]), 4 );
					memcpy( sWriteNum, (char *)&(query[12]), 2 );

					nRegiNum = strtol( sRegiNum, NULL, 16);
					nWriteNum = strtol( sWriteNum, NULL, 16);

					if ( strlen(query) != 14+(nRegiNum*4)
							|| nWriteNum != nRegiNum*2 )
					{
						// func 0x06 must have 16 byte len
						// last 4 byte is CRC
						// 16 - 4 = 12
						printf("\tplease input correct query\n\n");
						break;          // switch break
					}

					for ( i=0 ; i< nRegiNum ; i++ )
					{
						memcpy( svData[i], (char *)&(query[14+(i*4)]), 4 );
						nvData[i] = strtol( svData[i], NULL, 16);
					}
					printf("\n");
					modbus_write_registers(ctx, nRegiAdd, nRegiNum, nvData);
					// func 0x10 in modbus library
				}
				break;  // switch break;
			default :
				printf("incorrect Function code.\n\n");        // function code is not 0x03, 0x06, 0x10
		}       // switch end
	}       // while end

	printf("interface end.\n");

	/* Free the memory */
	free(tab_rp_registers);
}


int isHexCode( char *code )
{
	int len = strlen(code);

	int i;

	for ( i=0 ; i<len ; i++ )
	{
		if ( code[i] <48 ||
				code[i] > 70 ||
				(code[i] >57 && code[i] <65))
		{
			return 0;       // this code is not Hex code.
		}
	}
	return 1;   // this code is Hex code.
}


int PrintAll(modbus_t *ctx, uint16_t *tab_rp_registers, int num)
{
	int i,j;
	int start_add = 0x00;
	int data_num;
	int rc;

	printf("\t----------------------------------------  Data  ----------------------------------------\n");
	printf("\t\t 0x00 0x01 0x02 0x03 0x04 0x05 0x06 0x07 0x08 0x09 0x0A 0x0B 0x0C 0x0D 0x0E 0x0F\n");
	printf("\t----------------------------------------------------------------------------------------");

	// modbus library set maximum number of read data 0x125
	// 
	for ( j=num ; j>0 ; j-=0x70 )
	{
		if ( j >= 0x70 )
			data_num = 0x70;
		else
			data_num = j;

		rc = modbus_read_registers(ctx, start_add,
				data_num, tab_rp_registers);
		if ( rc < 1 )
		{
			printf("read operation error\n");	// error
			break;          // for break
		}

		for (i=0 ; i<data_num ; i++ )		// print readed data
		{
			if ( i%16 == 0 )
				printf("\n\t0x%02X\t", i+start_add);
			printf(" %04X", *(tab_rp_registers+i));
		}
		start_add += 0x70;		// move address
	}
	printf("\n\t----------------------------------------------------------------------------------------\n\n");
	return 1;
}

int RandomQuery(char *query, int FunctionCode)
{
	int i;
	int sel =1;

	char tempAdd[4];
	char tempNum[4];
	char tempData[4];
	char tempByte[2];		// temprary string data

	int add, num, data, byte;

	char tempQuery[QUERY_SIZE];		// temp query
	
	memset(tempQuery, 0, QUERY_SIZE);

	switch ( FunctionCode )
	{
		case FUNC03 :
			{
				sprintf(tempQuery, "%s", "0003");

				add = rand()%256;
				sprintf(tempAdd, "%04X", add );
				strcat(tempQuery, tempAdd);

				num = rand()%MODBUS_MAX_READ_REGISTERS +1;
				// MODBUS_MAX_READ_REGISTERS : set value in modbus.h (125)

				if ( num + add > 0xFF )
					num = 0x100 - add;

				sprintf(tempNum, "%04X", num );
				strcat(tempQuery, tempNum);
				break;
			}
		case FUNC06 :
			{
				sprintf(tempQuery, "%s", "0006");

				add = rand()%256;
				sprintf(tempAdd, "%04X", add );
				strcat(tempQuery, tempAdd);

				data = rand()%0x10000;
				sprintf(tempData, "%04X", data);
				strcat(tempQuery, tempData);
				break;
			}
		case FUNC10 :
			{
				sprintf(tempQuery, "%s", "0010");

				add = rand()%256;
				sprintf(tempAdd, "%04X", add );
				strcat(tempQuery, tempAdd);

				num = rand()%MODBUS_MAX_WRITE_REGISTERS +1;
				// MODBUS_MAX_WRITE_REGISTERS : set value in modbus.h (121)

				if ( num + add > 0xFF )
					num = 0x100 - add;

				sprintf(tempNum, "%04X", num );
				strcat(tempQuery, tempNum);

				byte = 2*num;
				sprintf(tempByte, "%02X", byte );
				strcat(tempQuery, tempByte);

				for ( i=0 ; i<num ; i++ )		 // make random data
				{
					memset( tempData, 0, sizeof(tempData) );
					data = rand()%0x10000;
					sprintf(tempData, "%04X", data);
					strcat(tempQuery, tempData);
				}
				break;
			}
	}
	printf("\nRandom query	: %s\n\n", tempQuery);
	memcpy(query, tempQuery, QUERY_SIZE);
}

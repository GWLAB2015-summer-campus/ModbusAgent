#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <modbus.h>

#include "unit-test.h"

enum { FUNC03, FUNC06, FUNC10 };

#define QUERY_SIZE 512

// check string 'code' is Hex code
int isHexCode( char *code );

// Print all data
int PrintAll(modbus_t *ctx, uint16_t *tab_rp_registers, int num);

// generate random query
int RandomQuery(char *query, int FunctionCode);

int main(int argc, char *argv[])
{
        srand((unsigned)time(NULL));

		int randNum;

        uint16_t *tab_rp_registers;     // for getting data from server( slave module )
        modbus_t *ctx;                          // modbus structure, save connect data
        int i,j;                                        // for repeat
        int nb_points;                          // number of register
        int rc;                                         // register counter

        char query[QUERY_SIZE];         // for getting query

        char sSlaveNum[3];
        char sFuncCode[3];
        char sRegiAdd[5];
        char sRegiNum[5];
        char sData[5];
        char svData[MODBUS_MAX_WRITE_REGISTERS][5];
        // MODBUS_MAX_WRITE_REGISTERS : set value in modbus.h
        char sWriteNum[3];                                              // str for read data in query

        uint16_t nSlaveNum;
        uint16_t nFuncCode;
        uint16_t nRegiAdd;
        uint16_t nRegiNum;
        uint16_t nData;
        uint16_t nvData[MODBUS_MAX_WRITE_REGISTERS];
        uint8_t nWriteNum;                                              // variable for read data in query

        if ( argc != 2 && argc != 3 )
        {
                printf("Usage : ./%s <portName> <option>", argv[0] );
                exit(1);
        }

        ctx = modbus_new_rtu(argv[1], 115200, 'N', 8, 1);               // make rtu connection
        if (ctx == NULL) {                      // error
                fprintf(stderr, "Unable to allocate libmodbus context\n");
                return -1;
        }

        if ( argc == 3 )
                if ( !strcmp( argv[2], "ON" ))
                        modbus_set_debug(ctx, TRUE);

        modbus_set_error_recovery(ctx,
                        MODBUS_ERROR_RECOVERY_LINK |
                        MODBUS_ERROR_RECOVERY_PROTOCOL);

        modbus_set_slave(ctx, SERVER_ID);               // set slave number

        if (modbus_connect(ctx) == -1) {
                fprintf(stderr, "Connection failed: %s\n",
                                modbus_strerror(errno));
                modbus_free(ctx);
                return -1;
        }

        /* Allocate and initialize the memory to store the registers */
        nb_points = (UT_REGISTERS_NB > UT_INPUT_REGISTERS_NB) ?
                UT_REGISTERS_NB : UT_INPUT_REGISTERS_NB;
        tab_rp_registers = (uint16_t *) malloc(nb_points * sizeof(uint16_t));
        memset(tab_rp_registers, 0, nb_points * sizeof(uint16_t));

        while ( 1 )
        {
				sleep(1);

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

				randNum = rand()%3;
				switch ( randNum )
				{
					case 0:
                        RandomQuery( query, FUNC03 );       // make random code 0x03
						break;
					case 1:
                        RandomQuery( query, FUNC06 );       // make random code 0x06
						break;
					case 2:
                        RandomQuery( query, FUNC10 );       // make random code 0x10
						break;
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

        /* Close the connection */
        modbus_close(ctx);
        modbus_free(ctx);

        return 0;
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
                        printf("read operation error\n");       // error
                        break;          // for break
                }

                for (i=0 ; i<data_num ; i++ )           // print readed data
                {
                        if ( i%16 == 0 )
                                printf("\n\t0x%02X\t", i+start_add);
                        printf(" %04X", *(tab_rp_registers+i));
                }
                start_add += 0x70;              // move address
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
        char tempByte[2];               // temprary string data

        int add, num, data, byte;

        char tempQuery[QUERY_SIZE];             // temp query

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

                                for ( i=0 ; i<num ; i++ )                // make random data
                                {
                                        memset( tempData, 0, sizeof(tempData) );
                                        data = rand()%0x10000;
                                        sprintf(tempData, "%04X", data);
                                        strcat(tempQuery, tempData);
                                }
                                break;
                        }
        }
        printf("\nRandom query  : %s\n\n", tempQuery);
        memcpy(query, tempQuery, QUERY_SIZE);
}


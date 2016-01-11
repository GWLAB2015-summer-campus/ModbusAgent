/*
 * Copyright © 2008-2010 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <modbus.h>

#include "unit-test.h"

#include "func.h"

enum {
	TCP,
	TCP_PI,
	RTU
};

#define QUERY_SIZE 512

int main(int argc, char *argv[])
{
	uint8_t *tab_rp_bits;
	uint16_t *tab_rp_registers;
	uint16_t *tab_rp_registers_bad;
	modbus_t *ctx;
	int i;
	uint8_t value;
	int nb_points;
	int rc;
	float real;
	uint32_t ireal;
	struct timeval old_response_timeout;
	struct timeval response_timeout;

	int use_backend = RTU;

	char query[QUERY_SIZE];

	char sSlaveNum[2];
	char sFuncCode[2];
	char sRegiAdd[4];
	char sRegiNum[4];
	char sData[4];
	char svData[50][4];
	char sWriteNum[2];

	short nSlaveNum;
	short nFuncCode;
	short nRegiAdd;
	short nRegiNum;
	short nData;
	short nvData[50];
	char nWriteNum;
/*	
	int nSlaveNum;
	int nFuncCode;
	int nRegiAdd;
	int nRegiNum;
	int nData;
*/
	if ( argc != 2 )
	{
		printf("Usage : ./%s <portName>", argv[0] );
		exit(1);
	}

	ctx = modbus_new_rtu(argv[1], 115200, 'N', 8, 1);
	if (ctx == NULL) {
		fprintf(stderr, "Unable to allocate libmodbus context\n");
		return -1;
	}
	modbus_set_debug(ctx, TRUE);
	modbus_set_error_recovery(ctx,
			MODBUS_ERROR_RECOVERY_LINK |
			MODBUS_ERROR_RECOVERY_PROTOCOL);

	modbus_set_slave(ctx, SERVER_ID);

	if (modbus_connect(ctx) == -1) {
		fprintf(stderr, "Connection failed: %s\n",
				modbus_strerror(errno));
		modbus_free(ctx);
		return -1;
	}

	/* Allocate and initialize the memory to store the bits */
	nb_points = (UT_BITS_NB > UT_INPUT_BITS_NB) ? UT_BITS_NB : UT_INPUT_BITS_NB;
	tab_rp_bits = (uint8_t *) malloc(nb_points * sizeof(uint8_t));
	memset(tab_rp_bits, 0, nb_points * sizeof(uint8_t));

	/* Allocate and initialize the memory to store the registers */
	nb_points = (UT_REGISTERS_NB > UT_INPUT_REGISTERS_NB) ?
		UT_REGISTERS_NB : UT_INPUT_REGISTERS_NB;
	tab_rp_registers = (uint16_t *) malloc(nb_points * sizeof(uint16_t));
	memset(tab_rp_registers, 0, nb_points * sizeof(uint16_t));

	printf("** UNIT TESTING **\n");

	while ( 1 )
	{
		printf("\tinput Query ( \"q\" is exit )\n");
		printf("\tQUERY : ");
		scanf( "%s", query );
		printf("\n");

		if ( !strcmp( query, "q" ) )	// quit message 
			break;
		if ( !isHexCode(query) )	// if incorrect query inputed
		{
			printf("\tplease input correct query\n\n");
			continue;
		}

		strncpy( sSlaveNum, &query[0], 2 );
		strncpy( sFuncCode, &query[2], 2 );

		nSlaveNum = strtol( sSlaveNum, NULL, 16 );
		nFuncCode = strtol( sFuncCode, NULL, 16 );
	
		switch ( nFuncCode )
		{
/**/		case 0x03:	// 3	read multiple data in RW register
/**/		case 0x04:	// 4	read multiple data in RO register
				{
					if ( strlen(query) != 12 )
						// func 0x03, 0x04 must have 16 byte len
						// last 4 byte is CRC
						// 16 - 4 = 12
					{
						printf("\tplease input correct query\n\n");
						break;
					}

					strncpy( sRegiAdd, &query[4], 4 );
					strncpy( sRegiNum, &query[8], 4 );

					nRegiAdd = strtol( sRegiAdd, NULL, 16);
					nRegiNum = strtol( sRegiNum, NULL, 16);
				
					if ( nFuncCode == 0x03 )
						rc = modbus_read_registers(ctx, nRegiAdd,
											nRegiNum, tab_rp_registers);
					else if ( nFuncCode == 0x04 )
						rc = modbus_read_input_registers(ctx, nRegiAdd,
				                               nRegiNum, tab_rp_registers);
					
					if ( rc < 1 )
					{	
						printf("\n");
						break;			// error
					}

					printf("\t----------------  Data  ----------------");
					for (i=0 ; i<nRegiNum ; i++ )
					{
						if ( i%5 == 0 )
							printf("\n");
						printf("\t%X", *(tab_rp_registers+i));
					}
					printf("\n\t----------------------------------------\n\n");
				}
				break;
/**/		case 0x06:	// 6	write one data in RW register
				{
					if ( strlen(query) != 12 )
						// func 0x06 must have 16 byte len
						// last 4 byte is CRC
						// 16 - 4 = 12
					{
					    printf("\tplease input correct query\n\n");
					    break;
					}

					strncpy( sRegiAdd, &query[4], 4 );
					strncpy( sData, &query[8], 4 );

					nRegiAdd = strtol( sRegiAdd, NULL, 16);
					nData = strtol( sData, NULL, 16);

					rc = modbus_write_register(ctx, nRegiAdd, nData);

					if ( rc != 1 )
					{
						printf("write operation error\n\n");
						break;
					}

					printf("\n");
				}
				break;
/**/		case 0x10:	// 16	write multiple data in RW register
				{
					strncpy( sRegiAdd, &query[4], 4 );
					strncpy( sRegiNum, &query[8], 4 );
					strncpy( sWriteNum, &query[12], 2 );

					nRegiAdd = strtol( sRegiAdd, NULL, 16);
					nRegiNum = strtol( sRegiNum, NULL, 16);
					nWriteNum = strtol( sWriteNum, NULL, 16);

					if ( strlen(query) != 14+(nRegiNum*4) 
						|| nWriteNum != nRegiNum*2 )

						// func 0x06 must have 16 byte len
						// last 4 byte is CRC
						// 16 - 4 = 12
					{
						printf("\tplease input correct query\n\n");
						break;
					}

					for ( i=0 ; i< nRegiNum ; i++ )
					{
						strncpy( svData[i], &query[14+(i*4)], 4 );
						nvData[i] = strtol( svData[i], NULL, 16);
					}
					modbus_write_registers(ctx, nRegiAdd, 
													nRegiNum, nvData);
				}
				break;
		}
	}

	printf("interface end.\n");

close:
	/* Free the memory */
	free(tab_rp_bits);
	free(tab_rp_registers);

	/* Close the connection */
	modbus_close(ctx);
	modbus_free(ctx);

	return 0;
}

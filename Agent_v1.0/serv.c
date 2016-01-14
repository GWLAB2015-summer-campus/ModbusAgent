 /* Copyright © 2008-2010 Stéphane Raimbault <stephane.raimbault@gmail.com>
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
#include <time.h>
#include <modbus.h>
#include <arpa/inet.h>

#include "unit-test.h"
#include "usrdef_modbus_receive.h" 

//#define MEASURED	(rand()%100)+1
#define MEASURED	0xEEEE
#define SPECIFIED	0xAAAA
#define TEST		0xBBBB
#define CONTROL		0xCCCC


// initiate data in RW register 
void init_register_data( uint16_t *regi );

int main(int argc, char*argv[])
{
	srand((unsigned)time(NULL));


	int socket;
	modbus_t *ctx;
	modbus_mapping_t *mb_mapping;
	int rc;
	int i;
	uint8_t *query;		// char pointer
	int header_length;

	if (argc != 2 && argc != 3)
	{
		printf ("Usgae : ./%s <portName> <option>\n", argv[0]);
		exit(1);
	}

	ctx = modbus_new_rtu(argv[1], 115200, 'N', 8, 1);

	if ( argc == 3 )
		if ( !strcmp( argv[2], "ON" ))
			modbus_set_debug(ctx, TRUE);

	query = malloc(MODBUS_RTU_MAX_ADU_LENGTH);
	header_length = modbus_get_header_length(ctx);
	modbus_set_slave(ctx, SERVER_ID);

/*	
	mb_mapping = modbus_mapping_new(
		   UT_BITS_ADDRESS + UT_BITS_NB,
		   UT_INPUT_BITS_ADDRESS + UT_INPUT_BITS_NB,
		   UT_REGISTERS_ADDRESS + UT_REGISTERS_NB,
		   UT_INPUT_REGISTERS_ADDRESS + UT_INPUT_REGISTERS_NB);
*/

	mb_mapping = modbus_mapping_new(
			UT_BITS_ADDRESS,
			UT_INPUT_BITS_ADDRESS,
			UT_REGISTERS_ADDRESS,
			UT_INPUT_REGISTERS_ADDRESS);

	if (mb_mapping == NULL) {
		fprintf(stderr, "Failed to allocate the mapping: %s\n",
				modbus_strerror(errno));
		modbus_free(ctx);
		return -1;
	}

	/* Examples from PI_MODBUS_300.pdf.
	   Only the read-only input values are assigned. */

	/** INPUT STATUS **/
	modbus_set_bits_from_bytes(mb_mapping->tab_input_bits,
			UT_INPUT_BITS_ADDRESS, UT_INPUT_BITS_NB,
			UT_INPUT_BITS_TAB);

	/** INPUT REGISTERS **/
	for (i=0; i < UT_INPUT_REGISTERS_NB; i++) {
		mb_mapping->tab_input_registers[i] = UT_INPUT_REGISTERS_TAB[i];
	}

	/** inpu Data **/
	init_register_data( mb_mapping->tab_registers );
		// user define function

	rc = modbus_connect(ctx);	// make modbus rtu connect
	if (rc == -1)
	{
		fprintf(stderr, "Unable to connect %s\n", modbus_strerror(errno));
		modbus_free(ctx);
		return -1;
	}
	for (;;)
	{
		rc = modbus_receive(ctx, query);	// for receive query
		if (rc == -1) {
			/* Connection closed by the client or error */
			break;
		}

		rc = modbus_reply_userdef(ctx, query, rc, mb_mapping, memory_flag);
		// for reply query
		if (rc == -1) {
			/* reply error */
			break;
		}
	}

	printf("Quit the loop: %s\n", modbus_strerror(errno));

	// free memory allocation
	modbus_mapping_free(mb_mapping);
	free(query);
	modbus_free(ctx);

	return 0;
}

void init_register_data( uint16_t *regi )
{
	int i;
	
	// data block for initiating register
	uint16_t tmp_data[] =
	{
		/* 0x00 */	0x5375, 0x6e53,
		/* 0x02 */  0x0001,
		/* 0x03 */  65,
		/* 0x04 */  0x4964, 0x6561, 0x6C20, 0x506F,
					0x7765, 0x7220, 0x436F, 0x6E76,
					0x436F, 0x6E76, 0x6572, 0x7465,
					0x7273, 0x2049, 0x6E63, 0x2E00,	
		/* 0x14 */  0x4942, 0x432D, 0x3330, 0x6B57,
					0x2D34, 0x3830, 0, 0,
					0, 0, 0, 0,
					0, 0, 0, 0,
		/* 0x24 */  0x0000, 0x0000, 0x0000, 0x0000,
					0x0000, 0x0000, 0x0000, 0x0000,
		/* 0x2C */  0x322E, 0x3030, 0x322E, 0x3030,
					0x3300, 0, 0, 0,
		/* 0x34 */  0x2041, 0x3030, 0x3030, 0x3030,
					0x3030, 0x3030, 0x3030, 0x00000,
					0x0000, 0x0000, 0x0000, 0x0000,
					0x0000, 0x0000, 0x0000, 0x0000,
		/* 0x44 */  0x1234,
		/* 0x45 */	101,
		/* 0x46 */	50,
		/* 0x47 */	MEASURED,
		/* 0x48 */	MEASURED,
		/* 0x49 */	MEASURED,
		/* 0x4A */	MEASURED,
		/* 0x4B */	-3,
		/* 0x4C */	MEASURED,
		/* 0x4D */	MEASURED,
		/* 0x4E */	MEASURED,
		/* 0x4F */	MEASURED,

		/* 0x50 */	MEASURED,
		/* 0x51 */  MEASURED,
		/* 0x52 */  0,
		/* 0x53 */  MEASURED,
		/* 0x54 */  0,
		/* 0x55 */  MEASURED,
		/* 0x56 */  0,
		/* 0x57 */  MEASURED,
		/* 0x58 */  0,
		/* 0x59 */  MEASURED,
		/* 0x5A */  0,
		/* 0x5B */  MEASURED,
		/* 0x5C */  0,
		/* 0x5D */  MEASURED, MEASURED,
		/* 0x5F */  MEASURED,

		/* 0x60 */  MEASURED,
		/* 0x61 */  0,
		/* 0x62 */  MEASURED,
		/* 0x63 */  0,
		/* 0x64 */  MEASURED,
		/* 0x65 */  0,
		/* 0x66 */  MEASURED,
		/* 0x67 */  MEASURED,
		/* 0x68 */  MEASURED,
		/* 0x69 */  MEASURED,
		/* 0x6A */  0,
		/* 0x6B */  MEASURED,
		/* 0x6C */  MEASURED,
		/* 0x6D */  MEASURED, MEASURED,
		/* 0x6F */  MEASURED, MEASURED,

		/* 0x71 */  MEASURED, MEASURED,
		/* 0x73 */  MEASURED, MEASURED,
		/* 0x75 */  MEASURED, MEASURED,
		/* 0x77 */  MEASURED, MEASURED,
		/* 0x79 */  0xFA1E,
		/* 0x7A */  133,
		/* 0x7B */  MEASURED,
		/* 0x7C */  MEASURED,
		/* 0x7D */  MEASURED,
		/* 0x7E */  1,
		/* 0x7F */  2,

		/* 0x80 */  SPECIFIED,
		/* 0x81 */  SPECIFIED,
		/* 0x82 */  SPECIFIED,
		/* 0x83 */  SPECIFIED,
		/* 0x84 */  SPECIFIED,
		/* 0x85 */  SPECIFIED,
		/* 0x86 */  SPECIFIED,
		/* 0x87 */  SPECIFIED,
		/* 0x88 */  SPECIFIED,
		/* 0x89 */  SPECIFIED,
		/* 0x8A */  MEASURED,
		/* 0x8B */  SPECIFIED,
		/* 0x8C */  SPECIFIED,
		/* 0x8D */  SPECIFIED,
		/* 0x8E */  MEASURED, MEASURED,

		/* 0x90 */  0,
		/* 0x91 */  SPECIFIED,
		/* 0x92 */  SPECIFIED,
		/* 0x93 */  MEASURED,
		/* 0x94 */  SPECIFIED,
		/* 0x95 */  MEASURED,
		/* 0x96 */  SPECIFIED,
		/* 0x97 */  SPECIFIED,
		/* 0x98 */  SPECIFIED,
		/* 0x99 */  SPECIFIED,
		/* 0x9A */  0, 0, 0, 0, 0, 0,

		/* 0xA0 */  0,
		/* 0xA1 */  SPECIFIED,
		/* 0xA2 */  SPECIFIED,
		/* 0xA3 */  MEASURED,
		/* 0xA4 */  SPECIFIED,
		/* 0xA5 */  MEASURED,
		/* 0xA6 */  SPECIFIED,
		/* 0xA7 */  SPECIFIED,
		/* 0xA8 */  SPECIFIED,
		/* 0xA9 */  SPECIFIED,
		/* 0xAA */  0, 0, 0, 0, 0, 0,

		/* 0xB0 */  0,
		/* 0xB1 */  SPECIFIED,
		/* 0xB2 */  SPECIFIED,
		/* 0xB3 */  MEASURED,
		/* 0xB4 */  SPECIFIED,
		/* 0xB5 */  SPECIFIED,
		/* 0xB6 */  SPECIFIED,
		/* 0xB7 */  SPECIFIED,
		/* 0xB8 */  MEASURED,
		/* 0xB9 */  0, 0, 0, 0, 0, 0, 0,

		/* 0xC0 */  TEST,
		/* 0xC1 */  TEST,
		/* 0xC2 */  TEST,
		/* 0xC3 */  TEST,
		/* 0xC4 */  TEST,
		/* 0xC5 */  TEST,
		/* 0xC6 */  TEST,
		/* 0xC7 */  TEST,
		/* 0xC8 */  TEST,
		/* 0xC9 */  TEST,
		/* 0xCA */  TEST,
		/* 0xCB */  TEST,
		/* 0xCC */  TEST,
		/* 0xCD */  TEST,
		/* 0xCE */  TEST,
		/* 0xCF */  TEST,

		/* 0xD0 */  TEST,
		/* 0xD1 */  TEST,
		/* 0xD2 */  TEST,
		/* 0xD3 */  TEST,
		/* 0xD4 */  TEST,
		/* 0xD5 */  TEST,
		/* 0xD6 */  TEST,
		/* 0xD7 */  TEST,
		/* 0xD8 */  TEST,
		/* 0xD9 */  TEST,
		/* 0xDA */  TEST,
		/* 0xDB */  TEST,
		/* 0xDC */  TEST,
		/* 0xDD */  TEST,
		/* 0xDE */  TEST,
		/* 0xDF */  TEST,

		/* 0xE0 */  MEASURED,
		/* 0xE1 */  MEASURED,
		/* 0xE2 */  MEASURED,
		/* 0xE3 */  SPECIFIED,
		/* 0xE4 */  SPECIFIED,
		/* 0xE5 */  CONTROL,
		/* 0xE6 */  CONTROL,
		/* 0xE7 */  CONTROL,
		/* 0xE8 */  CONTROL,
		/* 0xE9 */  CONTROL,
		/* 0xEA */  CONTROL,
		/* 0xEB */  CONTROL,
		/* 0xEC */  CONTROL,
		/* 0xED */  CONTROL,
		/* 0xEE */  CONTROL,
		/* 0xEF */  CONTROL,

		/* 0xF0 */  TEST,
		/* 0xF1 */  TEST,
		/* 0xF2 */  TEST,
		/* 0xF3 */  TEST,
		/* 0xF4 */  TEST,
		/* 0xF5 */  TEST, TEST, TEST, TEST, TEST, TEST, TEST, TEST,
		/* 0xFD */  TEST,
		/* 0xFE */  0xFFFF,
		/* 0xFF */ 0 
	};

	char Manufacturer[32] = "Gridwiz";
 	char Model[32] = "Gridwiz.Model.12345";
	char Option[16] = "Gridwiz Option";
	char Version[16] = "Gridwiz.1.0.0.";
	char SerialNum[32] = "12345678912345678912345678912345";
	// user define string

	memcpy ( (char*)&(tmp_data[0x04]), Manufacturer, 32 );
	memcpy ( (char*)&(tmp_data[0x14]), Model, 32 );
	memcpy ( (char*)&(tmp_data[0x24]), Option, 16 );
	memcpy ( (char*)&(tmp_data[0x2C]), Version, 16 );
	memcpy ( (char*)&(tmp_data[0x34]), SerialNum, 32 );
	// copy data to register

	
	// little endian to host endian
	for (i=0x04 ; i<0x44 ; i++ ) 
		tmp_data[i] = htons( tmp_data[i] );

	// copy set data to register
	for ( i=0 ; i<UT_REGISTERS_NB ; i++ )
		regi[i] = tmp_data[i];
}

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

enum {
	TCP,
	TCP_PI,
	RTU
};

int main(int argc, char*argv[])
{
	int socket;
	modbus_t *ctx;
	modbus_mapping_t *mb_mapping;
	int rc;
	int i;
	uint8_t *query;		// char pointer
	int header_length;

	if (argc != 2 )
	{
		printf ("Usgae : ./%s <portName>\n", argv[0]);
		exit(1);
	}

	ctx = modbus_new_rtu(argv[1], 115200, 'N', 8, 1);
	modbus_set_slave(ctx, SERVER_ID);
	query = malloc(MODBUS_RTU_MAX_ADU_LENGTH);
	header_length = modbus_get_header_length(ctx);

	modbus_set_debug(ctx, TRUE);
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
		mb_mapping->tab_input_registers[UT_INPUT_REGISTERS_ADDRESS+i] =
			UT_INPUT_REGISTERS_TAB[i];
		printf("%d\t", UT_INPUT_REGISTERS_NB);
	}

	rc = modbus_connect(ctx);
	if (rc == -1)
	{
		fprintf(stderr, "Unable to connect %s\n", modbus_strerror(errno));
		modbus_free(ctx);
		return -1;
	}
	for (;;)
	{
		rc = modbus_receive(ctx, query);
		if (rc == -1) {
			/* Connection closed by the client or error */
			break;
		}

		/* Read holding registers */
/*
		if (query[header_length] == 0x03) {
			if (MODBUS_GET_INT16_FROM_INT8(query, header_length + 3)
					== UT_REGISTERS_NB_SPECIAL) {
				printf("Set an incorrect number of values\n");
				MODBUS_SET_INT16_TO_INT8(query, header_length + 3,
						UT_REGISTERS_NB_SPECIAL - 1);
			}
			else if (MODBUS_GET_INT16_FROM_INT8(query, header_length + 1)
					== UT_REGISTERS_ADDRESS_SPECIAL) {
				printf("Reply to this special register address by an exception\n");
				modbus_reply_exception(ctx, query,
						MODBUS_EXCEPTION_SLAVE_OR_SERVER_BUSY);
				continue;
			}
		}
*/
		rc = modbus_reply(ctx, query, rc, mb_mapping);
		if (rc == -1) {
			break;
		}
	}

	printf("Quit the loop: %s\n", modbus_strerror(errno));

	modbus_mapping_free(mb_mapping);
	free(query);
	modbus_free(ctx);

	return 0;
}

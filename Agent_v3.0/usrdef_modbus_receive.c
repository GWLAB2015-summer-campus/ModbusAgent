#ifndef FUNC_H
#define FUCN_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <time.h>

#include <modbus.h>
#include "modbus-private.h"

/* Internal use */
#define MSG_LENGTH_UNDEFINED -1

/* Max between RTU and TCP max adu length (so TCP) */
#define MAX_MESSAGE_LENGTH 260

int modbus_reply_userdef(modbus_t *ctx, const uint8_t *req,
                 int req_length, modbus_mapping_t *mb_mapping,
				 const char flag[])
{
    int offset = ctx->backend->header_length;
    int slave = req[offset - 1];
    int function = req[offset];
    uint16_t address = (req[offset + 1] << 8) + req[offset + 2];
    uint8_t rsp[MAX_MESSAGE_LENGTH];
    int rsp_length = 0;
    sft_t sft;

    if (ctx->backend->filter_request(ctx, slave) == 1) {
        /* Filtered */
        return 0;
    }

    sft.slave = slave;
    sft.function = function;
    sft.t_id = ctx->backend->prepare_response_tid(req, &req_length);

	switch (function) {
		case _FC_READ_COILS: 
			{
				int nb = (req[offset + 3] << 8) + req[offset + 4];

				if (nb < 1 || MODBUS_MAX_READ_BITS < nb) {
					if (ctx->debug) {
						fprintf(stderr,
								"Illegal nb of values %d in read_bits (max %d)\n",
								nb, MODBUS_MAX_READ_BITS);
					}
					rsp_length = response_exception(
							ctx, &sft,
							MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE, rsp);
				} else if ((address + nb) > mb_mapping->nb_bits) {
					if (ctx->debug) {
						fprintf(stderr, "Illegal data address %0X in read_bits\n",
								address + nb);
					}
					rsp_length = response_exception(
							ctx, &sft,
							MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS, rsp);
				} else {
					rsp_length = ctx->backend->build_response_basis(&sft, rsp);
					rsp[rsp_length++] = (nb / 8) + ((nb % 8) ? 1 : 0);
					rsp_length = response_io_status(address, nb,
							mb_mapping->tab_bits,
							rsp, rsp_length);
				}
			}
			break;
		case _FC_READ_DISCRETE_INPUTS: 
			{
				/* Similar to coil status (but too many arguments to use a
				 * function) */
				int nb = (req[offset + 3] << 8) + req[offset + 4];

				if (nb < 1 || MODBUS_MAX_READ_BITS < nb) {
					if (ctx->debug) {
						fprintf(stderr,
								"Illegal nb of values %d in read_input_bits (max %d)\n",
								nb, MODBUS_MAX_READ_BITS);
					}
					rsp_length = response_exception(
							ctx, &sft,
							MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE, rsp);
				} else if ((address + nb) > mb_mapping->nb_input_bits) {
					if (ctx->debug) {
						fprintf(stderr, "Illegal data address %0X in read_input_bits\n",
								address + nb);
					}
					rsp_length = response_exception(
							ctx, &sft,
							MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS, rsp);
				} else {
					rsp_length = ctx->backend->build_response_basis(&sft, rsp);
					rsp[rsp_length++] = (nb / 8) + ((nb % 8) ? 1 : 0);
					rsp_length = response_io_status(address, nb,
							mb_mapping->tab_input_bits,
							rsp, rsp_length);
				}
			}
			break;
		case _FC_READ_HOLDING_REGISTERS: 
			{
				int nb = (req[offset + 3] << 8) + req[offset + 4];

				if (nb < 1 || MODBUS_MAX_READ_REGISTERS < nb) {
					if (ctx->debug) {
						fprintf(stderr,
								"Illegal nb of values %d in read_holding_registers (max %d)\n",
								nb, MODBUS_MAX_READ_REGISTERS);
					}
					rsp_length = response_exception(
							ctx, &sft,
							MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE, rsp);
				} else if ((address + nb) > mb_mapping->nb_registers) {
					if (ctx->debug) {
						fprintf(stderr, "Illegal data address %0X in read_registers\n",
								address + nb);
					}
					rsp_length = response_exception(
							ctx, &sft,
							MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS, rsp);
				} else {
					int i;

					rsp_length = ctx->backend->build_response_basis(&sft, rsp);
					rsp[rsp_length++] = nb << 1;
					for (i = address; i < address + nb; i++) {
						rsp[rsp_length++] = mb_mapping->tab_registers[i] >> 8;
						rsp[rsp_length++] = mb_mapping->tab_registers[i] & 0xFF;
					}
				}
			}
			break;
		case _FC_READ_INPUT_REGISTERS: 
			{
				/* Similar to holding registers (but too many arguments to use a
				 * function) */
				int nb = (req[offset + 3] << 8) + req[offset + 4];

				if (nb < 1 || MODBUS_MAX_READ_REGISTERS < nb) {
					if (ctx->debug) {
						fprintf(stderr,
								"Illegal number of values %d in read_input_registers (max %d)\n",
								nb, MODBUS_MAX_READ_REGISTERS);
					}
					rsp_length = response_exception(
							ctx, &sft,
							MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE, rsp);
				} else if ((address + nb) > mb_mapping->nb_input_registers) {
					if (ctx->debug) {
						fprintf(stderr, "Illegal data address %0X in read_input_registers\n",
								address + nb);
					}
					rsp_length = response_exception(
							ctx, &sft,
							MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS, rsp);
				} else {
					int i;

					rsp_length = ctx->backend->build_response_basis(&sft, rsp);
					rsp[rsp_length++] = nb << 1;
					for (i = address; i < address + nb; i++) {
						rsp[rsp_length++] = mb_mapping->tab_input_registers[i] >> 8;
						rsp[rsp_length++] = mb_mapping->tab_input_registers[i] & 0xFF;
					}
				}
			}
			break;
		case _FC_WRITE_SINGLE_COIL:
			if (address >= mb_mapping->nb_bits) {
				if (ctx->debug) {
					fprintf(stderr, "Illegal data address %0X in write_bit\n",
							address);
				}
				rsp_length = response_exception(
						ctx, &sft,
						MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS, rsp);
			} else {
				int data = (req[offset + 3] << 8) + req[offset + 4];

				if (data == 0xFF00 || data == 0x0) {
					mb_mapping->tab_bits[address] = (data) ? ON : OFF;
					memcpy(rsp, req, req_length);
					rsp_length = req_length;
				} else {
					if (ctx->debug) {
						fprintf(stderr,
								"Illegal data value %0X in write_bit request at address %0X\n",
								data, address);
					}
					rsp_length = response_exception(
							ctx, &sft,
							MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE, rsp);
				}
			}
			break;
		case _FC_WRITE_SINGLE_REGISTER:	/*	0x06	*/
			if (address >= mb_mapping->nb_registers) {
				if (ctx->debug) {
					fprintf(stderr, "Illegal data address %0X in write_register\n",
							address);
				}
				rsp_length = response_exception(
						ctx, &sft,
						MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS, rsp);
			}
			else
			{
				if ( flag[address] )	// RW flag check
				{
					int data = (req[offset + 3] << 8) + req[offset + 4];

					mb_mapping->tab_registers[address] = data;
				}
				memcpy(rsp, req, req_length);
				rsp_length = req_length;
				
			}
			break;
		case _FC_WRITE_MULTIPLE_COILS: 
			{
				int nb = (req[offset + 3] << 8) + req[offset + 4];

				if (nb < 1 || MODBUS_MAX_WRITE_BITS < nb) {
					if (ctx->debug) {
						fprintf(stderr,
								"Illegal number of values %d in write_bits (max %d)\n",
								nb, MODBUS_MAX_WRITE_BITS);
					}
					rsp_length = response_exception(
							ctx, &sft,
							MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE, rsp);
				} else if ((address + nb) > mb_mapping->nb_bits) {
					if (ctx->debug) {
						fprintf(stderr, "Illegal data address %0X in write_bits\n",
								address + nb);
					}
					rsp_length = response_exception(
							ctx, &sft,
							MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS, rsp);
				} else {
					/* 6 = byte count */
					modbus_set_bits_from_bytes(mb_mapping->tab_bits, address, nb, &req[offset + 6]);

					rsp_length = ctx->backend->build_response_basis(&sft, rsp);
					/* 4 to copy the bit address (2) and the quantity of bits */
					memcpy(rsp + rsp_length, req + rsp_length, 4);
					rsp_length += 4;
				}
			}
			break;
		case _FC_WRITE_MULTIPLE_REGISTERS: 		/*	0x10	*/
			{
				int nb = (req[offset + 3] << 8) + req[offset + 4];

				if (nb < 1 || MODBUS_MAX_WRITE_REGISTERS < nb) {
					if (ctx->debug) {
						fprintf(stderr,
								"Illegal number of values %d in write_registers (max %d)\n",
								nb, MODBUS_MAX_WRITE_REGISTERS);
					}
					rsp_length = response_exception(
							ctx, &sft,
							MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE, rsp);
				} else if ((address + nb) > mb_mapping->nb_registers) {
					if (ctx->debug) {
						fprintf(stderr, "Illegal data address %0X in write_registers\n",
								address + nb);
					}
					rsp_length = response_exception(
							ctx, &sft,
							MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS, rsp);
				}
				else
				{
					int i, j;
					for (i = address, j = 6; i < address + nb; i++, j += 2) 
					{
						/* 6 and 7 = first value */
						if ( flag[i] )	// RW flag check
						{
							mb_mapping->tab_registers[i] =
							(req[offset + j] << 8) + req[offset + j + 1];
						}
					}

					rsp_length = ctx->backend->build_response_basis(&sft, rsp);
					/* 4 to copy the address (2) and the no. of registers */
					memcpy(rsp + rsp_length, req + rsp_length, 4);
					rsp_length += 4;
				}
			}
			break;
		case _FC_REPORT_SLAVE_ID: 
			{
				int str_len;
				int byte_count_pos;

				rsp_length = ctx->backend->build_response_basis(&sft, rsp);
				/* Skip byte count for now */
				byte_count_pos = rsp_length++;
				rsp[rsp_length++] = _REPORT_SLAVE_ID;
				/* Run indicator status to ON */
				rsp[rsp_length++] = 0xFF;
				/* LMB + length of LIBMODBUS_VERSION_STRING */
				str_len = 3 + strlen(LIBMODBUS_VERSION_STRING);
				memcpy(rsp + rsp_length, "LMB" LIBMODBUS_VERSION_STRING, str_len);
				rsp_length += str_len;
				rsp[byte_count_pos] = rsp_length - byte_count_pos - 1;
			}
			break;
		case _FC_READ_EXCEPTION_STATUS:
			if (ctx->debug) {
				fprintf(stderr, "FIXME Not implemented\n");
			}
			errno = ENOPROTOOPT;
			return -1;
			break;

		case _FC_WRITE_AND_READ_REGISTERS: 
			{
				int nb = (req[offset + 3] << 8) + req[offset + 4];
				uint16_t address_write = (req[offset + 5] << 8) + req[offset + 6];
				int nb_write = (req[offset + 7] << 8) + req[offset + 8];
				int nb_write_bytes = req[offset + 9];

				if (nb_write < 1 || MODBUS_MAX_RW_WRITE_REGISTERS < nb_write ||
						nb < 1 || MODBUS_MAX_READ_REGISTERS < nb ||
						nb_write_bytes != nb_write * 2) {
					if (ctx->debug) {
						fprintf(stderr,
								"Illegal nb of values (W%d, R%d) in write_and_read_registers (max W%d, R%d)\n",
								nb_write, nb,
								MODBUS_MAX_RW_WRITE_REGISTERS, MODBUS_MAX_READ_REGISTERS);
					}
					rsp_length = response_exception(
							ctx, &sft,
							MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE, rsp);
				} else if ((address + nb) > mb_mapping->nb_registers ||
						(address_write + nb_write) > mb_mapping->nb_registers) {
					if (ctx->debug) {
						fprintf(stderr,
								"Illegal data read address %0X or write address %0X write_and_read_registers\n",
								address + nb, address_write + nb_write);
					}
					rsp_length = response_exception(ctx, &sft,
							MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS, rsp);
				} else {
					int i, j;
					rsp_length = ctx->backend->build_response_basis(&sft, rsp);
					rsp[rsp_length++] = nb << 1;

					/* Write first.
					   10 and 11 are the offset of the first values to write */
					for (i = address_write, j = 10; i < address_write + nb_write; i++, j += 2) {
						mb_mapping->tab_registers[i] =
							(req[offset + j] << 8) + req[offset + j + 1];
					}

					/* and read the data for the response */
					for (i = address; i < address + nb; i++) {
						rsp[rsp_length++] = mb_mapping->tab_registers[i] >> 8;
						rsp[rsp_length++] = mb_mapping->tab_registers[i] & 0xFF;
					}
				}
			}
			break;

		default:
			rsp_length = response_exception(ctx, &sft,
					MODBUS_EXCEPTION_ILLEGAL_FUNCTION,
					rsp);
			break;
	}

	return send_msg(ctx, rsp, rsp_length);
}

int response_exception(modbus_t *ctx, sft_t *sft,
		int exception_code, uint8_t *rsp)
{
	int rsp_length;

	sft->function = sft->function + 0x80;
	rsp_length = ctx->backend->build_response_basis(sft, rsp);

	/* Positive exception code */
	rsp[rsp_length++] = exception_code;

	return rsp_length;
}

int response_io_status(int address, int nb,
		uint8_t *tab_io_status,
		uint8_t *rsp, int offset)
{
	int shift = 0;
	int byte = 0;
	int i;

	for (i = address; i < address+nb; i++) {
		byte |= tab_io_status[i] << shift;
		if (shift == 7) {
			/* Byte is full */
			rsp[offset++] = byte;
			byte = shift = 0;
		} else {
			shift++;
		}
	}

	if (shift != 0)
		rsp[offset++] = byte;

	return offset;
}

int send_msg(modbus_t *ctx, uint8_t *msg, int msg_length)
{
	int rc;
	int i;

	msg_length = ctx->backend->send_msg_pre(msg, msg_length);

	if (ctx->debug) {
		for (i = 0; i < msg_length; i++)
			printf("[%.2X]", msg[i]);
		printf("\n");
	}

	/* In recovery mode, the write command will be issued until to be
	   successful! Disabled by default. */
	do {
		rc = ctx->backend->send(ctx, msg, msg_length);
		if (rc == -1) {
			_error_print(ctx, NULL);
			if (ctx->error_recovery & MODBUS_ERROR_RECOVERY_LINK) {
				int saved_errno = errno;

				if ((errno == EBADF || errno == ECONNRESET || errno == EPIPE)) {
					modbus_close(ctx);
					modbus_connect(ctx);
				} else {
					_sleep_and_flush(ctx);
				}
				errno = saved_errno;
			}
		}
	} while ((ctx->error_recovery & MODBUS_ERROR_RECOVERY_LINK) &&
			rc == -1);

	if (rc > 0 && rc != msg_length) {
		errno = EMBBADDATA;
		return -1;
	}

	return rc;
}

#endif

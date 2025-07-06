#include "floppy_144.h"
#include "../io.h"
#include "../timer.h"
#include "../print.h"
#include "../pic.h"
#include <stdint.h>
#include <stdbool.h>

enum FloppyCommands {
	FC_READ_TRACK = 2, // generates IRQ6
	FC_SPECIFY = 3, // * set drive parameters
	FC_SENSE_DRIVE_STATUS = 4,
	FC_WRITE_DATA = 5, // * write to the disk
	FC_READ_DATA = 6, // * read from the disk
	FC_RECALIBRATE = 7, // * seek to cylinder 0
	FC_SENSE_INTERRUPT = 8, // * ack IRQ6, get status of last command
	FC_WRITE_DELETED_DATA = 9,
	FC_READ_ID = 10, // generates IRQ6
	FC_READ_DELETED_DATA = 12,
	FC_FORMAT_TRACK = 13, // *
	FC_DUMPREG = 14,
	FC_SEEK = 15, // * seek both heads to cylinder X
	FC_VERSION = 16, // * used during initialization, once
	FC_SCAN_EQUAL = 17,
	FC_PERPENDICULAR_MODE = 18, // * used during initialization, once, maybe
	FC_CONFIGURE = 19, // * set controller parameters
	FC_LOCK = 20, // * protect controller params from a reset
	FC_VERIFY = 22,
	FC_SCAN_LOW_OR_EQUAL = 25,
	FC_SCAN_HIGH_OR_EQUAL = 29
};

enum FloppyRegisters {
	FR_STATUS_REGISTER_A = 0x3F0, // read-only
	FR_STATUS_REGISTER_B = 0x3F1, // read-only
	FR_DIGITAL_OUTPUT_REGISTER = 0x3F2,
	FR_TAPE_DRIVE_REGISTER = 0x3F3,
	FR_MAIN_STATUS_REGISTER = 0x3F4, // read-only
	FR_DATARATE_SELECT_REGISTER = 0x3F4, // write-only
	FR_DATA_FIFO = 0x3F5,
	FR_DIGITAL_INPUT_REGISTER = 0x3F7, // read-only
	FR_CONFIGURATION_CONTROL_REGISTER = 0x3F7 // write-only
};

//DOR = Digital Output Register
#define m_DOR_D_SEL 0 //2 bits
#define m_DOR_RESET 2 //1 bit
#define m_DOR_IRQ 3 //1 bit
#define m_DOR_MOT_A 4 //1 bit
#define m_DOR_MOT_B 5 //1 bit
#define m_DOR_MOT_C 6 //1 bit
#define m_DOR_MOT_D 7 //1 bit

//MSR = Main Status Register
#define m_MSR_ACT_A 0 //1 bit
#define m_MSR_ACT_B 1 //1 bit
#define m_MSR_ACT_C 2 //1 bit
#define m_MSR_ACT_D 3 //1 bit
#define m_MSR_CB 4 //1 bit
#define m_MSR_NDMA 5 //1 bit
#define m_MSR_DIO 6 //1 bit
#define m_MSR_RQM 7 //1 bit

volatile bool received_irq = false;

struct CHS Floppy144_lba_to_chs(uint32_t lba)
{
	return CHS_lba_to_chs(lba, m_FLOPPY_144_SECTORS_PER_TRACK);
}

static void wait_for_floppy_irq(void)
{
	while (!received_irq) {
		__asm__ volatile("hlt");
	}
	received_irq = false;
}

static void send_specify_cmnd(void)
{
	uint8_t srt = 8;
	uint8_t hlt = 5;
	uint8_t hut = 0;
	bool ndma = false;

	uint8_t bytes_to_send[] = {
		FC_SPECIFY,
		(srt << 4) | hut,
		(hlt << 1) | ndma,
	};

	volatile uint8_t msr = IO_in_port_b(FR_MAIN_STATUS_REGISTER);
	if (((msr >> m_MSR_RQM) & 1) != 1 || ((msr >> m_MSR_DIO) & 1) != 0) {
		k_printf("Problem sending specify command to floppy disk.\n");
		return;
	}

	for (unsigned i = 0;
	     i < sizeof(bytes_to_send) / sizeof(bytes_to_send[0]); i++) {
		if (((msr >> m_MSR_DIO) & 1) == 1) {
			k_printf("Problem sending specify command to floppy"
				 " disk. i = %u\n",
				 i);
			return;
		}
		IO_out_port_b(FR_DATA_FIFO, bytes_to_send[i]);
		do {
			msr = IO_in_port_b(FR_MAIN_STATUS_REGISTER);
		} while (((msr >> m_MSR_RQM) & 1) != 1);
	}
}

static void turn_on_floppy_motor(unsigned drive)
{
	//Select the drive for next access, and turn on it's motor
	volatile uint8_t dor = IO_in_port_b(FR_DIGITAL_OUTPUT_REGISTER);
	dor |= (drive << m_DOR_D_SEL);
	dor |= (1 << (m_DOR_MOT_A + drive));
	IO_out_port_b(FR_DIGITAL_OUTPUT_REGISTER, dor);

	//Wait for the floppy to spin up
	Timer_wait(500);

	k_printf("Floppy is up to speed.\n");
}

//Assumes the drive is already spinning
static void calibrate_floppy(unsigned drive)
{
	volatile uint8_t msr = IO_in_port_b(FR_MAIN_STATUS_REGISTER);
	if (((msr >> m_MSR_RQM) & 1) != 1 || ((msr >> m_MSR_DIO) & 1) != 0) {
		k_printf("Problem calibrating floppy disk.\n");
		return;
	}

	volatile uint8_t dor = IO_in_port_b(FR_DIGITAL_OUTPUT_REGISTER);
	k_printf("Calibrating floppy. dor = 0x%02x, msr = 0x%02x\n", dor, msr);

	uint8_t bytes_to_send[] = { FC_RECALIBRATE, drive };
	for (unsigned i = 0;
	     i < sizeof(bytes_to_send) / sizeof(bytes_to_send[0]); i++) {
		if (((msr >> m_MSR_DIO) & 1) == 1) {
			k_printf("Problem calibrating floppy disk. i = %u\n",
				 i);
			return;
		}
		IO_out_port_b(FR_DATA_FIFO, bytes_to_send[i]);
		do {
			msr = IO_in_port_b(FR_MAIN_STATUS_REGISTER);
		} while (((msr >> m_MSR_RQM) & 1) != 1);
	}

	wait_for_floppy_irq();

	msr = IO_in_port_b(FR_MAIN_STATUS_REGISTER);

	k_printf("Calibration done. msr = 0x%02x\n", msr);
}

static void configure_floppy(void)
{
	k_printf("configuring floppy\n");

	volatile uint8_t msr = IO_in_port_b(FR_MAIN_STATUS_REGISTER);
	if (((msr >> m_MSR_RQM) & 1) != 1 || ((msr >> m_MSR_DIO) & 1) != 0) {
		k_printf("Problem configuring floppy disk.\n");
		return;
	}

	//Implied seek on, FIFO on, driving polling mode off, threshold = 8,
	//precomp = 0
	uint8_t second_arg = (1 << 6) | (1 << 5) | (0 << 4) | (8 - 1);
	uint8_t bytes_to_send[4] = { FC_CONFIGURE, 0, second_arg, 0 };
	for (unsigned i = 0; i < 4; i++) {
		if (((msr >> m_MSR_DIO) & 1) == 1) {
			k_printf("Problem configuring floppy disk. i = %u\n",
				 i);
			return;
		}
		IO_out_port_b(FR_DATA_FIFO, bytes_to_send[i]);
		do {
			msr = IO_in_port_b(FR_MAIN_STATUS_REGISTER);
		} while (((msr >> m_MSR_RQM) & 1) != 1);
	}

	if (((msr >> m_MSR_NDMA) & 1) == 1) {
		k_printf("Problem configuring floppy disk.\n");
		return;
	}

	k_printf("done configuring floppy. msr = 0x%02x\n", msr);
}

void Floppy144_reset(void)
{
	k_printf("resetting floppy\n");

	/*
    volatile uint8_t dsr = 0x80;
    IO_out_port_b(FR_DATARATE_SELECT_REGISTER, dsr);

    wait_for_floppy_irq();*/

	configure_floppy();

	//Set the data rate
	volatile uint8_t ccr = 0;
	IO_out_port_b(FR_CONFIGURATION_CONTROL_REGISTER, ccr);

	send_specify_cmnd();

	/*
    volatile uint8_t dor = IO_in_port_b(FR_DIGITAL_OUTPUT_REGISTER);
    dor |= 1 << m_DOR_IRQ;
    IO_out_port_b(FR_DIGITAL_OUTPUT_REGISTER, dor);*/
	volatile uint8_t dor = (0 << m_DOR_D_SEL) | (1 << m_DOR_IRQ) |
			       (1 << m_DOR_RESET);
	IO_out_port_b(FR_DIGITAL_OUTPUT_REGISTER, dor);

	volatile uint8_t msr = IO_in_port_b(FR_MAIN_STATUS_REGISTER);
	k_printf("done resetting floppy. msr = 0x%02x\n", msr);
}

void Floppy144_init(void)
{
	k_printf("initing floppy\n");

	//Send a version command to the floppy controller

	volatile uint8_t msr = IO_in_port_b(FR_MAIN_STATUS_REGISTER);
	if (((msr >> m_MSR_RQM) & 1) != 1 || ((msr >> m_MSR_DIO) & 1) != 0) {
		k_printf("Problem initializing floppy disk.\n");
		return;
	}

	IO_out_port_b(FR_DATA_FIFO, FC_VERSION);
	do {
		msr = IO_in_port_b(FR_MAIN_STATUS_REGISTER);
	} while (((msr >> m_MSR_RQM) & 1) != 1);

	if (((msr >> m_MSR_NDMA) & 1) == 1) {
		k_printf("Problem initializing floppy disk.\n");
		return;
	}

	uint8_t cmd_result = IO_in_port_b(FR_DATA_FIFO);

	if (cmd_result != 0x90) {
		k_printf("The current floppy drive is not supported!\n");
		return;
	}

	configure_floppy();

	Floppy144_reset();

	volatile uint8_t dor = IO_in_port_b(FR_DIGITAL_OUTPUT_REGISTER);
	k_printf("done inititing floppy. dor = 0x%02x\n", dor);
}

static void read_sector(struct CHS location, unsigned drive, void *data_out)
{
	uint8_t bytes_to_send[] = {
		0x80 | 0x40 | FC_READ_DATA,
		(location.head << 2) | drive,
		location.cyl,
		location.head,
		location.sector,
		2,
		(location.sector + 1) >= m_FLOPPY_144_SECTORS_PER_TRACK ?
			m_FLOPPY_144_SECTORS_PER_TRACK :
			location.sector + 1,
		0x1b,
		0xff,
	};

	volatile uint8_t msr = IO_in_port_b(FR_MAIN_STATUS_REGISTER);
	if (((msr >> m_MSR_RQM) & 1) != 1 || ((msr >> m_MSR_DIO) & 1) != 0) {
		k_printf("Problem with reading a sector from floppy disk. 1\n");
		return;
	}

	for (unsigned i = 0;
	     i < sizeof(bytes_to_send) / sizeof(bytes_to_send[0]); i++) {
		if (((msr >> m_MSR_DIO) & 1) == 1) {
			k_printf("Problem with reading a sector from floppy"
				 " disk. i = %u\n",
				 i);
			return;
		}
		IO_out_port_b(FR_DATA_FIFO, bytes_to_send[i]);
		do {
			msr = IO_in_port_b(FR_MAIN_STATUS_REGISTER);
		} while (((msr >> m_MSR_RQM) & 1) != 1);
	}

	if (((msr >> m_MSR_NDMA) & 1) == 1) {
		k_printf("Problem with reading a sector from floppy disk. 2\n");
		return;
	}

	do {
		msr = IO_in_port_b(FR_MAIN_STATUS_REGISTER);
	} while (((msr >> m_MSR_RQM) & 1) == 0);

	//Points to where the next byte should be written.
	uint8_t *data_ptr = data_out;

	while (((msr >> m_MSR_CB) & 1) == 1) {
		while (((msr >> m_MSR_RQM) & 1) == 0) {
			//k_printf("got here\n");
			msr = IO_in_port_b(FR_MAIN_STATUS_REGISTER);
		}

		//k_printf("reading\n");
		if (((msr >> m_MSR_DIO) & 1) != 1 ||
		    ((msr >> m_MSR_CB) & 1) != 1) {
			k_printf(
				"Problem with reading a sector from floppy disk. 3\n");
			return;
		}

		*(data_ptr++) = IO_in_port_b(FR_DATA_FIFO);
		//k_printf("loaded data = %d\n", *(data_ptr - 1));

		//wait_for_floppy_irq();

		msr = IO_in_port_b(FR_MAIN_STATUS_REGISTER);
	}

	k_printf("got here\n");

	if (((msr >> m_MSR_RQM) & 1) != 1 || ((msr >> m_MSR_DIO) & 1) != 0 ||
	    ((msr >> m_MSR_CB) & 1) != 0) {
		k_printf("rqm = %d\n", (msr >> m_MSR_RQM) & 1);
		k_printf("dio = %d\n", (msr >> m_MSR_DIO) & 1);
		k_printf("cb = %d\n", (msr >> m_MSR_CB) & 1);
		k_printf("Problem with reading a sector from floppy disk. 4\n");
		return;
	}
}

void Floppy144_load(struct CHS location, uint32_t n_sectors, unsigned drive,
		    void *output)
{
	turn_on_floppy_motor(drive);
	calibrate_floppy(drive);

	read_sector(location, drive, output);

	k_printf("output = %s\n", (char *)output);
}

void Floppy144_C_irq_handler(void)
{
	//k_printf("Floppy irq!\n");
	received_irq = true;
	PIC_send_eoi(6);
}

__asm__(".globl Floppy144_irq_handler\n"
	"Floppy144_irq_handler:\n"
	"   pusha               \n" /* Save all registers               */
	"   pushw %ds           \n" /* Set up the data segment          */
	"   pushw %es           \n"
	"   pushw %ss           \n" /* Note that ss is always valid     */
	"   pushw %ss           \n"
	"   popw %ds            \n"
	"   popw %es            \n"
	"                       \n"
	"   call Floppy144_C_irq_handler\n"
	"                       \n"
	"   popw %es            \n"
	"   popw %ds            \n" /* Restore registers                */
	"   popa                \n"
	"   iret                \n" /* Exit interrupt                   */
);

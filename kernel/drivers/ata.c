#ifdef m_COMMENT

#include "ata.h"
#include "../io.h"
#include "../timer.h"
#include "../print.h"
#include <stdint.h>

#define m_GET_NTH_BIT(x, n) ((x >> n) & 1)

enum IOPort {

	IO_DATA_REG = 0, //R/W
	IO_ERROR_REG = 1, //R
	IO_FEATURES_REG = 1, //W
	IO_SECT_COUNT_REG = 2, //R/W
	IO_SET_NUM_REG = 3, //R/W
	IO_CYL_LOW_REG = 4, //R/W
	IO_CYL_HIGH_REG = 5, //R/W
	IO_DRIVE_HEAD_REG = 6, //R/W
	IO_STATUS_REG = 7, //R
	IO_COMMAND_REG = 7, //W

};

enum CTRLPort {

	CTRL_ALTERNATE_STATUS_REG = 0, //R
	CTRL_DEVICE_CTRL_REG = 0, //W
	CTRL_DRIVE_ADDRESS_REG = 1, //R

};

enum ErrorRegBit {

	ERR_REG_AMNF, //Address mark not found
	ERR_REG_TKZNF, //Track zero not found
	ERR_REG_ABRT, //Aborted command
	ERR_REG_MCR, //Media change request
	ERR_REG_IDNF, //ID not found
	ERR_REG_MC, //Media changed
	ERR_REG_UNC, //Uncorrectable data error
	ERR_REG_BBK, //Bad block detected

};

enum DriveHeadRegBit {

	//bits 0-3 of the register. In CHS addressing, bits 0-3 of the head. In LBA, bits 24-27 of the block number.
	DRIVE_HEAD_REG_DRV = 4, //Selects the drive number
	DRIVE_HEAD_REG_ALWAYS_SET_0 = 5,
	DRIVE_HEAD_REG_LBA = 6, //If clear, uses CHS, else uses LBA.
	DRIVE_HEAD_REG_ALWAYS_SET_1 = 7,

};

enum StatusRegBit {

	STATUS_REG_ERR, //Indicates an error occured
	STATUS_REG_IDX, //Index. Always set to 0
	STATUS_REG_CORR, //Corrected data. Always set to 0
	STATUS_REG_DRQ, //Set when the drive has PIO data to transfer, or is ready to accept PIO data.
	STATUS_REG_SRV, //Overlapped mode service request
	STATUS_REG_DF, //Drive fault error (does not set ERR)
	STATUS_REG_RDY, //Bit is clear when drive is spun down, or after an error. Set otherwise.
	STATUS_REG_BSY, //Indicates the drive is preparing to send/receive data (wait ofr it to clear). In case of 'hang' (it never clears), do a software reset

};

enum DeviceCtrlRegBit {

	DEVICE_CTRL_REG_ALWAYS_UNSET_0,
	DEVICE_CTRL_REG_NIEN, //Set this to stop the current device from sending interrupts.
	DEVICE_CTRL_REG_SRST, //Set, then clear (after 5us), this to do a "Software Reset" on all ATA drives on a bus, if one is misbehaving.
	DEVICE_CTRL_REG_ALWAYS_UNSET_1,
	DEVICE_CTRL_REG_ALWAYS_UNSET_2,
	DEVICE_CTRL_REG_ALWAYS_UNSET_3,
	DEVICE_CTRL_REG_ALWAYS_UNSET_4,
	DEVICE_CTRL_REG_HOB, //Set this to read back the High Order Byte of the last LBA48 value sent to an IO port

};

enum DeviceAddressRegBit {

	DEVICE_ADDRESS_REG_DS0, //Drive 0 select. Clears when drive 0 selected.
	DEVICE_ADDRESS_REG_DS1, //Drive 1 select. Clears when drive 1 selected.
	//One's compliment representation of the currently selected head.
	DEVICE_ADDRESS_REG_HS0,
	DEVICE_ADDRESS_REG_HS1,
	DEVICE_ADDRESS_REG_HS2,
	DEVICE_ADDRESS_REG_HS3,

	DEVICE_ADDRESS_REG_WTG, //Write gate; goes low while writing to the drive is in progress.
	DEVICE_ADDRESS_REG_UNUSED,

};

struct ATA_Device ATA_device = { 0x1f0, 0x3f6, 0x4, 0x5, 0x6 };

void ATA_soft_reset(struct ATA_Device *ctrl)
{
	volatile uint8_t dev_ctrl_reg =
		IO_in_port_b(ctrl->dev_ctl + CTRL_DEVICE_CTRL_REG);
	dev_ctrl_reg |= (1 << DEVICE_CTRL_REG_SRST);
	IO_out_port_b(ctrl->dev_ctl + CTRL_DEVICE_CTRL_REG, dev_ctrl_reg);
	Timer_wait(2);
	dev_ctrl_reg &= ~(1 << DEVICE_CTRL_REG_SRST);
	IO_out_port_b(ctrl->dev_ctl + CTRL_DEVICE_CTRL_REG, dev_ctrl_reg);
}

void ATA_read_sector(struct ATA_Device *ctrl, void *data)
{
	volatile uint8_t drive_head_reg =
		IO_in_port_b(ctrl->base + IO_DRIVE_HEAD_REG);
	drive_head_reg |= (1 << DRIVE_HEAD_REG_LBA);
	IO_out_port_b(ctrl->base + IO_DRIVE_HEAD_REG, drive_head_reg);
}

#endif

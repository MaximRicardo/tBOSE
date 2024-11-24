#include "floppy_144.h"

enum FloppyRegisters {
    FR_STATUS_REGISTER_A                = 0x3F0, // read-only
    FR_STATUS_REGISTER_B                = 0x3F1, // read-only
    FR_DIGITAL_OUTPUT_REGISTER          = 0x3F2,
    FR_TAPE_DRIVE_REGISTER              = 0x3F3,
    FR_MAIN_STATUS_REGISTER             = 0x3F4, // read-only
    FR_DATARATE_SELECT_REGISTER         = 0x3F4, // write-only
    FR_DATA_FIFO                        = 0x3F5,
    FR_DIGITAL_INPUT_REGISTER           = 0x3F7, // read-only
    FR_CONFIGURATION_CONTROL_REGISTER   = 0x3F7  // write-only
};

enum FloppyCommands {
   FC_READ_TRACK =                 2,	// generates IRQ6
   FC_SPECIFY =                    3,      // * set drive parameters
   FC_SENSE_DRIVE_STATUS =         4,
   FC_WRITE_DATA =                 5,      // * write to the disk
   FC_READ_DATA =                  6,      // * read from the disk
   FC_RECALIBRATE =                7,      // * seek to cylinder 0
   FC_SENSE_INTERRUPT =            8,      // * ack IRQ6, get status of last command
   FC_WRITE_DELETED_DATA =         9,
   FC_READ_ID =                    10,	// generates IRQ6
   FC_READ_DELETED_DATA =          12,
   FC_FORMAT_TRACK =               13,     // *
   FC_DUMPREG =                    14,
   FC_SEEK =                       15,     // * seek both heads to cylinder X
   FC_VERSION =                    16,	// * used during initialization, once
   FC_SCAN_EQUAL =                 17,
   FC_PERPENDICULAR_MODE =         18,	// * used during initialization, once, maybe
   FC_CONFIGURE =                  19,     // * set controller parameters
   FC_LOCK =                       20,     // * protect controller params from a reset
   FC_VERIFY =                     22,
   FC_SCAN_LOW_OR_EQUAL =          25,
   FC_SCAN_HIGH_OR_EQUAL =         29
};

//DOR = Digital Output Register
#define m_DOR_D_SEL         0   //2 bits
#define m_DOR_RESET         2   //1 bit
#define m_DOR_IRQ           3   //1 bit
#define m_DOR_MOT_A         4   //1 bit
#define m_DOR_MOT_B         5   //1 bit
#define m_DOR_MOT_C         6   //1 bit
#define m_DOR_MOT_D         7   //1 bit

//MSR = Main Status Register
#define m_MSR_ACT_A         0   //1 bit
#define m_MSR_ACT_B         1   //1 bit
#define m_MSR_ACT_C         2   //1 bit
#define m_MSR_ACT_D         3   //1 bit
#define m_MSR_CB            4   //1 bit
#define m_MSR_NDMA          5   //1 bit
#define m_MSR_DIO           6   //1 bit
#define m_MSR_RQM           7   //1 bit

struct CHS FLOPPY_144_lba_to_chs(uint32_t lba) {

    return CHS_lba_to_chs(lba, m_FLOPPY_144_SECTORS_PER_TRACK);

}

void FLOPPY_144_reset(void) {



}

void FLOPPY_144_init(void) {



}

void FLOPPY_144_load(struct CHS location, uint32_t n_sectors, void *output) {



}

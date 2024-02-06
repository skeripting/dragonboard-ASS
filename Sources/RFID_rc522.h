//*****************************************************************************
//*****************************    C Source Code    ***************************
//*****************************************************************************
//
// DESIGNER NAME: Bruce Link
//
//     FILE NAME: RFID_rc522.h
//
//-----------------------------------------------------------------------------
//
// DESCRIPTION:
//    This file contains supported functions and command definitions for the
//    Feenove (www.freenove.com) RFID-RC522 module (RFID-MFRC522).
//
//*****************************************************************************

#ifndef _RFID_RC522_MOD_H_
#define _RFID_RC522_MOD_H_

#include "rfid_rc522_regs.h"
 
//  Integer Types
typedef   signed char       sint8;      // signed 8 bit values
typedef unsigned char       uint8;      // unsigned 8 bit values
typedef   signed short int  sint16;     // signed 16 bit values
typedef unsigned short int  uint16;     // unsigned 16 bit values
typedef   signed long  int  sint32;     // signed 32 bit values
typedef unsigned long  int  uint32;     // unsigned 32 bit values
typedef unsigned char       uint8_t;      // unsigned 8 bit values
//  Floating Point Types
typedef float  real32;                  // single precision floating values
typedef double real64;                  // double precision floating values


//  Register Types
typedef volatile uint8*  register8;     //  8-bit register
typedef volatile uint16* register16;    // 16-bit register
typedef volatile uint32* register32;    // 32-bit register

typedef unsigned short bool;            // Boolean

//-----------------------------------------------------------------------------
//                        Define symbolic constants
//-----------------------------------------------------------------------------

// RFID-RC522 (PCD) commands
#define RC522_IDLE_CMD          0x00    // NO action; Cancel the current command
#define RC522_MEMORY            0x01    // 
#define RC522_GEN_RANID_CMD     0x02    // 
#define RC522_CALC_CRC_CMD      0x03    // Calculate CRC
#define RC522_TRANSMIT_CMD      0x04    // Transmit data
#define RC522_NOCHNGE_CMD       0x07    // 
#define RC522_RECEIVE_CMD       0x08    // Receive Data
#define RC522_TRANSCEIVE_CMD    0x0C    // Transmit and receive data,
#define RC522_AUTHENT_CMD       0x0E    // Authentication Key
#define RC522_RESET_CMD         0x0F    // reset

// Mifare_One card command word
#define PICC_REQIDL             0x26    // find the antenna area not in hibernation
#define PICC_REQALL             0x52    // find all the cards antenna area
#define PICC_ANTICOLL           0x93    // prevent conflict (anti-collision)
#define PICC_SElECTTAG          0x93    // select card
#define PICC_AUTHENT1A          0x60    // authentication key A password
#define PICC_AUTHENT1B          0x61    // authentication key B password
#define PICC_READ               0x30    // Read Block
#define PICC_WRITE              0xA0    // write block
#define PICC_DECREMENT          0xC0    // debit
#define PICC_INCREMENT          0xC1    // recharge
#define PICC_RESTORE            0xC2    // transfer block data to the buffer
#define PICC_TRANSFER           0xB0    // save the data in the buffer
#define PICC_HALT               0x50    // Sleep

// Define MCRC522 Status values
#define MI_OK                   (0)
#define MI_NOTAGERR             (1)
#define MI_ERR                  (2)

//Dummy byte
#define MFRC522_DUMMY            0x00
#define RC522_MAX_LEN            16


// Define the types of PICC (Proximity Integrated Circuit Card): 
// Basically the PICC is card or tag using the ISO 14443A interface, eg Mifare 
// or NTAG203
typedef enum
{
  PICC_TYPE_NOT_COMPLETE = 0,
  PICC_TYPE_MIFARE_MINI,
  PICC_TYPE_MIFARE_1K,
  PICC_TYPE_MIFARE_4K,
  PICC_TYPE_MIFARE_UL,
  PICC_TYPE_MIFARE_PLUS,
  PICC_TYPE_TNP3XXX,
  PICC_TYPE_ISO_14443_4,
  PICC_TYPE_ISO_18092,
  PICC_TYPE_UNKNOWN
} PICC_TYPE_t;

//-----------------------------------------------------------------------------
//                      Define Public Functions
//-----------------------------------------------------------------------------
uint8  rc522_init(uint8 Type);
void   rc522_soft_reset(void);
void   rc522_write_reg(uint8 reg, uint8 value);
uint8  rc522_read_reg(uint8 reg);
void   rc522_set_bitmask(uint8 reg, uint8 mask);
void   rc522_clear_bitmask(uint8 reg, uint8 mask);
void   rc522_antenna_on(void);
void   rc522_antenna_off(void);
bool   rc522_select_card(uint8 *card_id);
uint8  rc522_get_firmware_version(void);
sint8  rc522_is_card_present(uint8 req_mode, uint8* tag_type);
bool   rc522_wait_for_card_present(uint16 timeout);
bool   rc522_wait_for_card_removed(uint16 timeout);
void   rc522_send_halt(void);
sint8  rc522_to_card(uint8 command, uint8* data_to_send, uint8 data_length,
                     uint8* receive_data, uint16* backLen);
sint8  MFRC522_Anticoll(uint8* serNum);
sint8* rc522_type_to_string(PICC_TYPE_t type);
sint16 MFRC522_ParseType(uint8 TagSelectRet);
uint8  rc522_select_tag(uint8* serial_num);




//-----------------------------------------------------------------------------
//                 Define Public Global Variables
//-----------------------------------------------------------------------------


#endif /* _RFID_RC522_MOD_H_ */
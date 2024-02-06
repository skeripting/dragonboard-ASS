//*****************************************************************************
//*****************************    C Source Code    ***************************
//*****************************************************************************
//
// DESIGNER NAME: Bruce Link
//
//     FILE NAME: rfid_rc522_regs.h
//
//-----------------------------------------------------------------------------
//
// DESCRIPTION:
//    This file contains register definitions for the Feenove (www.freenove.com)
//    RFID-RC522 module (RFID-MFRC522).
//
//*****************************************************************************

#ifndef _RFID_RC522_REGS_H_
#define _RFID_RC522_REGS_H_

//-----------------------------------------------------------------------------
//                        Define symbolic constants
//-----------------------------------------------------------------------------

//------------------------------------
// MFRC522 Registers Addresses
//------------------------------------
//Page 0 ==> Command and Status
#define PAGE0_RESERVED_1            0x00
#define COMMAND_REG                 0x01
#define COM_IEN_REG                 0x02
#define DIV_IEN_REG                 0x03
#define COM_IRQ_REG                 0x04
#define DIV_IRQ_REG                 0x05
#define ERROR_REG                   0x06
#define STATUS1_REG                 0x07
#define STATUS2_REG                 0x08
#define FIFO_DATA_REG               0x09
#define FIFO_LEVEL_REG              0x0A
#define WATER_LEVEL_REG             0x0B
#define CONTROL_REG                 0x0C
#define BIT_FRAMING_REG             0x0D
#define COLL_REG                    0x0E
#define PAGE0_RESERVED_2            0x0F

//Page 1 ==> Command
#define PAGE1_RESERVED_1            0x10
#define MODE_REG                    0x11
#define TX_MODE_REG                 0x12
#define RX_MODE_REG                 0x13
#define TX_CONTROL_REG              0x14
#define TX_ASK_REG                  0x15
#define TX_SEL_REG                  0x16
#define RX_SEL_REG                  0x17
#define RX_THRESHOLD_REG            0x18
#define DEMOD_REG                   0x19
#define PAGE1_RESERVED_2            0x1A
#define PAGE1_RESERVED_3            0x1B
#define MFTX_REG                    0x1C
#define MFRX_REG                    0x1D
#define PAGE1_RESERVED_4            0x1E
#define SERIAL_SPEED_REG            0x1F

//Page 2 ==> CFG
#define PAGE2_RESERVED_1            0x20
#define CRC_RESULT_REG_M            0x21
#define CRC_RESULT_REG_L            0x22
#define PAGE2_RESERVED_2            0x23
#define MOD_WIDTH_REG               0x24
#define PAGE2_RESERVED_3            0x25
#define RF_CFG_REG                  0x26
#define GSN_REG                     0x27
#define CWGSP_REG                   0x28
#define MODGSP_REG                  0x29
#define T_MODE_REG                  0x2A
#define T_PRESCALER_REG             0x2B
#define T_RELOAD_REG_H              0x2C
#define T_RELOAD_REG_L              0x2D
#define T_COUNTER_VAL_REG_1         0x2E
#define T_COUNTER_VAL_REG_2         0x2F

//Page 3 ==> Test_REGister
#define PAGE3_RESERVED_1            0x30
#define TEST_SEL1_REG               0x31
#define TEST_SEL2_REG               0x32
#define TEST_PIN_EN_REG             0x33
#define TEST_PIN_VALUE_REG          0x34
#define TEST_BUS_REG                0x35
#define AUTO_TEST_REG               0x36
#define VERSION_REG                 0x37
#define ANALOG_TEST_REG             0x38
#define TEST_DAC1_REG               0x39
#define TEST_DAC2_REG               0x3A
#define TEST_ADC_REG                0x3B
#define PAGE3_RESERVED_2            0x3C
#define PAGE3_RESERVED_3            0x3D
#define PAGE3_RESERVED_4            0x3E
#define PAGE3_RESERVED_5            0x3F



//------------------------------------
// MFRC522 Registers Bit Masks
//------------------------------------



#endif /* _RFID_RC522_REGS_H_ */

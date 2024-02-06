//*****************************************************************************
//*****************************    C Source Code    ***************************
//*****************************************************************************
//
// DESIGNER NAME: Bruce Link
//
//     FILE NAME: RFID_rc522.c
//
//-----------------------------------------------------------------------------
//
// DESCRIPTION:
//    This file allows the Dragon12-Light-USB Rev D. board to utilizes the
//    Feenove (www.freenove.com) RFID-RC522 module. The module is also know 
//    as the Proximity Coupling Device pr PCD. It is nothing but the RFID 
//    Reader Module based on the MFRC522 IC. This file provides support 
//    for reading and writing Mifare Cards.
//
//    This code was leveraged from the C code provided by the Feenove RFID
//    Starter Kit for Raspberry Pi (FNK0025). See Github for download
//    https://github.com/Freenove/Freenove_RFID_Starter_Kit_for_Raspberry_Pi
//
//    Note that the RFID-RC522 module is a 3.3v module but the Dragon12-light
//    board does not have a 3.3 VDC power souce. Therefore, an external
//    3.3 VDC must be used to power the module.
//
//    This file assumes the RFID-RC522 module has the following connections
//    to the Dragon12 board.
//
//      ------------------------------------------------------------------
//      - MODULE PIN   |       DRAGON12 CONNECTION                       -
//      ------------------------------------------------------------------
//      - 3.3 VDC      | Connected to externmal 3.3 VDC                  -
//      - RST          | Connected J96-1 PH0 (Uses SW5 as manual reset)  -
//      - GND          | Connected Dagon12 GND pin                       -
//      - IRQ          | Not supported (not used)                        -
//      - MISO         | Connect to J99-3 (SPI0-MOSI)                    -
//      - MOSI         | Connect to J99-4 (SPI0-MISO)                    -
//      - SCK          | Connect to J99-5 (SPI0-SCK)                     -
//      - SDA          | Connect to J99-6 (SS0)                          -
//      ------------------------------------------------------------------
//
//    Serial Peripheral Interface timing requirements for MFRC522 (from
//    NXP Semiconductors MFRC522 data sheet)
//      - The SPI clock must idle low (CPOL=0).
//      - Data bytes on MOSI and MISO lines are sent with the MSB first.
//      - Data on both MOSI and MISO lines must be stable on the rising
//        edge of the clock and can be changed on the falling edge. (CPHA=0)
//      - Data is provided by the MFRC522 on the falling clock edge and is
//        stable during the rising clock edge.
//      - SDA must be low
//      - The MSB of the first byte defines the mode used. To read data
//        from the MFRC522 the MSB is set to logic 1. To write data to
//        the MFRC522 the MSB must be set to logic 0. Bits 6 to 1 define
//        the address and the LSB is set to logic 0.
//
//
//    HCS12 SPI Configuration (SPIxCR1)register
//      - HCS12 is the master (MSTR = 1)
//      - CPHA=0, CPOL=0
//      - LSBFE = 0

//*****************************************************************************

//-----------------------------------------------------------------------------
//                       Required user support files below
//-----------------------------------------------------------------------------
#include "main_asm.h"               // interface to the assembly module
#include "csc202_lab_support.h"     // include CSC202 Support
#include "RFID_rc522.h"             // include CSC202 Support

#define LCD_LINE_2_ADDR 0x40


//-----------------------------------------------------------------------------
//                        Define symbolic constants
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
//                        Define types constants
//-----------------------------------------------------------------------------

char *PICC_TYPE_STRING[] =
  {
    "PICC_TYPE_NOT_COMPLETE",
    "PICC_TYPE_MIFARE_MINI",
    "PICC_TYPE_MIFARE_1K",
    "PICC_TYPE_MIFARE_4K",
    "PICC_TYPE_MIFARE_UL",
    "PICC_TYPE_MIFARE_PLUS",
    "PICC_TYPE_TNP3XXX",
    "PICC_TYPE_ISO_14443_4",
    "PICC_TYPE_ISO_18092",
    "PICC_TYPE_UNKNOWN"
  };



//-----------------------------------------------------------------------------
//                               Public functions
//-----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// NAME: rc522_get_firmware_version
//
// DESCRIPTION:
//    This function returns the firmware version for the module.
//
// INPUT:
//   none
//
// OUTPUT:
//   none
//
// RETURN:
//   an uint8 data value that represents the firmware version of the module.
//----------------------------------------------------------------------------
uint8 rc522_get_firmware_version(void)
{

  return (rc522_read_reg(VERSION_REG));

} /* rc522_get_firmware_version */


//----------------------------------------------------------------------------
// NAME: rc522_init
//
// DESCRIPTION:
//    This function resets the RFID-RC522 module.
//
// INPUT:
//   none
//
// OUTPUT:
//   none
//
// RETURN:
//   none
//----------------------------------------------------------------------------
void rc522_soft_reset(void)
{

  rc522_write_reg(COMMAND_REG, RC522_RESET_CMD);

} /* rc522_reset */


//----------------------------------------------------------------------------
// NAME: rc522_init
//
// DESCRIPTION:
//    This function initializes the RFID-RC522 module. This function support
//    types A and B under ISO/IEC 14443 standard. Both type communicate via
//    radio at 13.56 MHz (RFID HIGH FREQUENCY). The main differences between
//    these two kinds concern modulation methods, coding schemes (Part 2)
//    and protocol initialization procedures (Part 3).
//
// INPUT:
//   card_type - a uint8 that represents the ASIC letters A or B
//
// OUTPUT:
//   none
//
// RETURN:
//   a sint8 data value 0 means Success
//----------------------------------------------------------------------------
uint8 rc522_init(uint8 card_type)
{
  uint8 data;
  uint8 status = MI_OK;

  rc522_soft_reset();

  ms_delay(200);

  rc522_write_reg(T_PRESCALER_REG, 0x3E);

  data = rc522_read_reg(T_PRESCALER_REG);

  // do a quick sanity check to ensure RFID reader is present
  if (data != 0x3E)
  {
    status = MI_ERR;
  } /* if */

  rc522_write_reg(T_MODE_REG, 0x8D);
  rc522_write_reg(T_PRESCALER_REG, 0x3E);
  rc522_write_reg(T_RELOAD_REG_L, 30);
  rc522_write_reg(T_RELOAD_REG_H, 0);
  rc522_write_reg(TX_ASK_REG, 0x40);
  rc522_write_reg(MODE_REG, 0x3D);

  // Card Type A requires a slightly differt configuration
  if (card_type == 'A')
  {
    rc522_clear_bitmask(STATUS2_REG, 0x08);
    rc522_write_reg(MODE_REG, 0x3D);
    rc522_write_reg(RX_SEL_REG, 0x86);
    rc522_write_reg(RF_CFG_REG, 0x7F);
    rc522_write_reg(T_RELOAD_REG_L, 30);
    rc522_write_reg(T_RELOAD_REG_H, 0);
    rc522_write_reg(T_MODE_REG, 0x8D);
    rc522_write_reg(T_PRESCALER_REG, 0x3E);
  } /* if */

  rc522_antenna_on();

  return (status);

} /* rc522_init */


//----------------------------------------------------------------------------
// NAME: rc522_read_reg
//
// DESCRIPTION:
//    This function reads a single register in the MFRC522 chip.
//
// INPUT:
//   reg    - This parameter is a uint8 value that represents the register
//            address to read from
//   value  - This parameter is a uint8 value that represents the data value
//            to read from the register
//
// OUTPUT:
//   none
//
// RETURN:
//   an uint8 data value to read into the register
//----------------------------------------------------------------------------
uint8 rc522_read_reg(uint8 reg)
{
  uint8 data;

  // rc522 requires SS to remain low for entire burst transfer
  SS0_LO();
  (void)send_SPI0((reg << 1 & 0xFE) | 0x80);
  data = send_SPI0(0x00);
  SS0_HI();

  return data;

} /* rc522_read_reg */


//----------------------------------------------------------------------------
// NAME: rc522_write_reg
//
// DESCRIPTION:
//    This function write single register in the MFRC522 chip.
//
// INPUT:
//   reg    - This parameter is a uint8 value that represents the register
//            address write to
//   value  - This parameter is a uint8 value that represents the data value
//            to write to the register
//
// OUTPUT:
//   none
//
// RETURN:
//   none
//----------------------------------------------------------------------------
void rc522_write_reg(uint8 reg, uint8 value)
{

  // rc522 requires SS to remain low for entire burst transfer
  SS0_LO();
  (void)send_SPI0((reg << 1) & 0x7E);
  (void)send_SPI0(value);
  SS0_HI();

} /* rc522_write_reg */


//----------------------------------------------------------------------------
// NAME: rc522_set_bitmask
//
// DESCRIPTION:
//    This function reads a register in the MFRC522 chip and sets the bit(s)
//    marked by the bit mask.
//
// INPUT:
//   reg    - This parameter is a uint8 value that represents the register
//            address to be modified.
//   mask   - This parameter is a uint8 value that represents the bitmask
//            for the bits to set.
//
// OUTPUT:
//   none
//
// RETURN:
//   none
//----------------------------------------------------------------------------
void rc522_set_bitmask(uint8 reg, uint8 mask)
{
  uint8 reg_value = 0;

  reg_value = rc522_read_reg(reg);

  reg_value |= mask;

  rc522_write_reg(reg, reg_value);

} /* rc522_set_bitmask */


//----------------------------------------------------------------------------
// NAME: rc522_clear_bitmask
//
// DESCRIPTION:
//    This function reads a register in the MFRC522 chip and clears the bit(s)
//    marked by the bit mask.
//
// INPUT:
//   reg    - This parameter is a uint8 value that represents the register
//            address to be modified.
//   mask   - This parameter is a uint8 value that represents the bitmask
//            for the bits to clear.
//
// OUTPUT:
//   none
//
// RETURN:
//   none
//----------------------------------------------------------------------------
void rc522_clear_bitmask(uint8 reg, uint8 mask)
{
  uint8 reg_value = 0;

  reg_value = rc522_read_reg(reg);

  reg_value &= ~mask;

  rc522_write_reg(reg, reg_value);

} /* rc522_clear_bitmask */


//----------------------------------------------------------------------------
// NAME: rc522_to_card
//
// DESCRIPTION:
//    This function sends command and data to send to the FIFO on RFID-RC522
//    module.
//
// INPUT:
//    command       - this parameter is a unit8 that defines the RC522
//                    command to execute
//    data_to_send  - this is an array of uint8 data that is written to the
//                    FIFO after the command.
//    data_length   - this parameter is a unit8 that defines the number of
//                    data bytes to write to the FIFO.
//
// OUTPUT:
//    receieve_data - this is the address of an array of uint8 data that
//                    is read back from the FIFO after the command was
//                    executed.
//    bytes_received- this is the address of a unit16 that defines the number
//                    of data bytes read from the FIFO.
//
// RETURN:
//   status of the operation
//----------------------------------------------------------------------------
sint8 rc522_to_card(uint8 command, uint8* data_to_send, uint8 data_length,
                      uint8* receive_data, uint16* bytes_received)
{
  sint8 status  = MI_ERR;
  uint8 irqEn   = 0x00;
  uint8 waitIRq = 0x00;
  uint8 lastBits;
  uint8 ird_status_reg;
  uint8 data_in_fifo;
  uint8 idx;
  uint16 timeout_cntr;

  switch (command)
  {
    case RC522_AUTHENT_CMD:
    {
      irqEn   = 0x12;
      waitIRq = 0x10;
      break;
    } /* case */

    case RC522_TRANSCEIVE_CMD:
    {
      irqEn   = 0x77;
      waitIRq = 0x30;
      break;
    } /* case */

    default:
      break;

  } /* switch */

  rc522_write_reg(COM_IEN_REG, irqEn | 0x80);

  rc522_clear_bitmask(COM_IRQ_REG, 0x80);
  rc522_set_bitmask(FIFO_LEVEL_REG, 0x80);

  rc522_write_reg(COMMAND_REG, RC522_IDLE_CMD);

  // Writing data to the FIFO
  for (idx = 0; idx < data_length; idx++)
  {
    rc522_write_reg(FIFO_DATA_REG, data_to_send[idx]);
  } /* for */

  // Execute the command
  rc522_write_reg(COMMAND_REG, command);

  if (command == RC522_TRANSCEIVE_CMD)
  {
    // Set StartSend=1 to transmission of data
    rc522_set_bitmask(BIT_FRAMING_REG, 0x80);
  } /* if */

  // Waiting to receive data to complete
  timeout_cntr = 2000;

  // CommIrqReg[7..0]
  // Set1 TxIRq RxIRq IdleIRq HiAlerIRq LoAlertIRq ErrIRq TimerIRq
  do
  {
    ms_delay(10);
    ird_status_reg = rc522_read_reg(COM_IRQ_REG);
    timeout_cntr--;
  } while ((timeout_cntr != 0) && !(ird_status_reg & 0x01) && !(ird_status_reg & waitIRq));

  // Tranfser to card done so set StartSend=0
  rc522_clear_bitmask(BIT_FRAMING_REG, 0x80);

  if (timeout_cntr != 0)
  {
    if (!(rc522_read_reg(ERROR_REG) & 0x1B))
    {
      if (ird_status_reg & irqEn & 0x01)
      {
        status = MI_NOTAGERR;
      } /* if */
      else
      {
        status = MI_OK;
      } /* else */

      if (command == RC522_TRANSCEIVE_CMD)
      {
        data_in_fifo = rc522_read_reg(FIFO_LEVEL_REG);
        lastBits = rc522_read_reg(CONTROL_REG) & 0x07;

        if (lastBits)
        {
          *bytes_received = (data_in_fifo - 1) * 8 + lastBits;
        } /* if */
        else
        {
          *bytes_received = data_in_fifo * 8;
        } /* else */

        if (data_in_fifo == 0)
        {
          data_in_fifo = 1;
        } /* if */

        if (data_in_fifo > RC522_MAX_LEN)
        {
          data_in_fifo = RC522_MAX_LEN;
        } /* if */

        // Reading the received data in FIFO
        for (idx = 0; idx < data_in_fifo; idx++)
        {
          receive_data[idx] = rc522_read_reg(FIFO_DATA_REG);
        } /* for */

      }  /* if */
    }  /* if */

    else
    {
      status = MI_ERR;
    }  /* else */
  }  /* if */

  return status;
} /* rc522_to_card */



//----------------------------------------------------------------------------
// NAME: rc522_is_card_present
//
// DESCRIPTION:
//    This function checks if a card is present on the RFID-RC522 module.
//
// INPUT:
//   none
//
// OUTPUT:
//   none
//
// RETURN:
//   none
//----------------------------------------------------------------------------
sint8 rc522_is_card_present(uint8 req_mode, uint8* tag_type)
{
  sint8  status = MI_OK;
  uint16 backBits;      //The received data bits

  rc522_write_reg(BIT_FRAMING_REG, 0x07);

  tag_type[0] = req_mode;

  status = rc522_to_card(RC522_TRANSCEIVE_CMD, tag_type, 1, tag_type, &backBits);

  if (status != MI_OK)
  {
    return(status);
  }

  if (backBits != 0x10)
  {
    return(MI_ERR);
  } /* if */

  return status;

} /* rc522_is_card_present */



//----------------------------------------------------------------------------
// NAME: rc522_wait_for_card_present
//
// DESCRIPTION:
//    This function waits for a card to be present on the RFID-RC522 module.
//    This function checks for the presence of a card on the
//    RFID-RC522 module every 5 mSeconds. It uses a cycle timeout parameter
//    to avoid waiting indefinitely.
//
// INPUT:
//   timeout  - this value is a uint16 loop counter to define how many times
//              the function checks before giving up.//
// OUTPUT:
//   none
//
// RETURN:
//   If a card is detected, it returns true, otherwise, it returns false
//----------------------------------------------------------------------------
bool rc522_wait_for_card_present(uint16 timeout)
{
  uint8 data;
  uint8 status = FALSE;

  while (timeout > 0)
  {
    // sets the StartSend bit to 1, which initiates a data transfer between the RFID reader and the RFID tag.
    rc522_write_reg(BIT_FRAMING_REG, 0x87);
    data = rc522_read_reg(COM_IRQ_REG);

    #define IRQ_REG_IDLE_BITMASK            0x00
    #define IRQ_REG_ERR_BITMASK             0x08

      set_lcd_addr(LCD_LINE_2_ADDR);
      write_int_lcd(data);
    if (((data & 0x10) == 0x10))// && (data & 0x01) == 0x00))
    {
      // Clear CommIrqReg register
      rc522_write_reg(COM_IRQ_REG, 0x7F);
      status  = TRUE;
      timeout = 0;
    } /* if */
    else
    {
      ms_delay(5);
      timeout--;
    } /* else */
  } /* while */

  return status;
} /* rc522_wait_for_card_present */


//----------------------------------------------------------------------------
// NAME: rc522_wait_for_card_removed
//
// DESCRIPTION:
//    This function waits for a card to be removed from the RFID-RC522 module.
//    This function checks for the presence of a card on the RFID-RC522
//    module every 5 mSeconds. It uses a cycle timeout parameter to
//    avoid waiting indefinitely.
//
// INPUT:
//   timeout  - this value is a uint16 loop counter to define how many times
//              the function checks before giving up.
//
// OUTPUT:
//   none
//
// RETURN:
//   If a card is removed, it returns true, otherwise, it returns false.
//----------------------------------------------------------------------------
bool rc522_wait_for_card_removed(uint16 timeout)
{
  uint8 irq_status;
  uint8 removed = FALSE;

  // Clear the card present flag
  rc522_write_reg(BIT_FRAMING_REG, 0x00);

  while (timeout > 0 && !removed)
  {
    // Check the IRQ status register for card removal events
    irq_status = rc522_read_reg(COM_IRQ_REG);

    if ((irq_status & 0x10) && (irq_status & 0x02))
    {
      removed  = TRUE;
    } /* if */
    else
    {
      ms_delay(1);
      timeout--;
    } /* else */

  } /* while */

  return (removed);
} /* rc522_wait_for_card_removed */


//----------------------------------------------------------------------------
// NAME: rc522_antenna_on
//
// DESCRIPTION:
//    This function turns on the antenna of the RFID-RC522 module.
//
// INPUT:
//   none
//
// OUTPUT:
//   none
//
// RETURN:
//   none
//----------------------------------------------------------------------------
void rc522_antenna_on(void)
{
  uint8 reg_value = 0;

  reg_value = rc522_read_reg(TX_CONTROL_REG);
  if (!(reg_value & 0x03))
  {
    rc522_set_bitmask(TX_CONTROL_REG, 0x03);
  } /* if */

} /* rc522_antenna_on */


//----------------------------------------------------------------------------
// NAME: rc522_antenna_off
//
// DESCRIPTION:
//    This function turns off the antenna of the RFID-RC522 module.
//
// INPUT:
//   none
//
// OUTPUT:
//   none
//
// RETURN:
//   none
//----------------------------------------------------------------------------
void rc522_antenna_off(void)
{

  rc522_clear_bitmask(TX_CONTROL_REG, 0x03);

} /* rc522_antenna_off */


//----------------------------------------------------------------------------
// NAME: MFRC522_Anticoll      mfrc522_get_card_serial
//
// DESCRIPTION:
//    This function reads reads the UID of the RFID card on the RFID-RC522
//    module.
//
// INPUT:
//   serial_num - TBD
//
// OUTPUT:
//   none
//
// RETURN:
//   none
//----------------------------------------------------------------------------
sint8 MFRC522_Anticoll(uint8* serial_num)
{
  sint8  status;
  uint8  idx;
  uint8  serial_numCheck = 0;
  uint16 unLen;

  rc522_write_reg(BIT_FRAMING_REG, 0x00);

  serial_num[0] = PICC_ANTICOLL;
  serial_num[1] = 0x20;
  
  status = rc522_to_card(RC522_TRANSCEIVE_CMD, serial_num, 2, serial_num, &unLen);

  if (status == MI_OK)
  {
    // Check card serial number
    for (idx = 0; idx < 4; idx++)
    {
      serial_numCheck ^= serial_num[idx];
    } /* for */

    // check sum with last byte
    if (serial_numCheck != serial_num[idx])
    {
      status = MI_ERR;
    } /* if */
  } /* if */

  return status;

} /* MFRC522_Anticoll */


//----------------------------------------------------------------------------
// NAME: rc522_calculate_CRC
//
// DESCRIPTION:
//    This function initializes the RFID-RC522 module
//
// INPUT:
//   card_type -
//
// OUTPUT:
//   none
//
// RETURN:
//   TBD
//----------------------------------------------------------------------------
void rc522_calculate_CRC(uint8* pIndata, uint8 len, uint8* pOutData)
{
  uint8 i, n;

  rc522_clear_bitmask(DIV_IRQ_REG, 0x04);     //CRCIrq = 0
  rc522_set_bitmask(FIFO_LEVEL_REG, 0x80);  //Clear the FIFO pointer
  //Write_MFRC522(CommandReg, PCD_IDLE);

  //Writing data to the FIFO
  for (i = 0; i < len; i++)
  {
    rc522_write_reg(FIFO_DATA_REG, *(pIndata + i));
  }
  rc522_write_reg(COMMAND_REG, RC522_CALC_CRC_CMD);

  //Wait CRC calculation is complete
  i = 0xFF;
  do
  {
    n = rc522_read_reg(DIV_IRQ_REG);
    i--;
  } while ((i != 0) && !(n & 0x04));      //CRCIrq = 1

  //Read CRC calculation result
  pOutData[0] = rc522_read_reg(CRC_RESULT_REG_L);
  pOutData[1] = rc522_read_reg(CRC_RESULT_REG_M);

} /* rc522_calculate_CRC */


//----------------------------------------------------------------------------
// NAME: rc522_select_tag
//
// DESCRIPTION:
//    This function select card and reads card storage volume
//
// INPUT:
//   serial_num - a uint8 that represents the serial number of the card
//
// OUTPUT:
//   none
//
// RETURN:
//   return MI_OK if success
//----------------------------------------------------------------------------
uint8 rc522_select_tag(uint8* serial_num)
{
  uint8  i;
  sint8  status;
  uint8  size;
  uint16 data_received;
  uint8  buffer[32] = "";

  buffer[0] = PICC_SElECTTAG;
  buffer[1] = 0x70;
  
  for (i = 0; i < 5; i++)
  {
    buffer[i + 2] = *(serial_num + i);
  }
  rc522_calculate_CRC(buffer, 7, &buffer[7]); //Fill [7:8] with 2byte CRC
  status = rc522_to_card(RC522_TRANSCEIVE_CMD, buffer, 9, buffer, &data_received);

  if ((status == MI_OK) && (data_received == 0x18))
  {
    size = buffer[0];
  }
  else
  {
    size = 0;
  }

  return size;
} /* rc522_select_tag */


//----------------------------------------------------------------------------
// NAME: rc522_type_to_string
//
// DESCRIPTION:
//    This function initializes the RFID-RC522 module
//
// INPUT:
//   card_type -
//
// OUTPUT:
//   none
//
// RETURN:
//   TBD
//----------------------------------------------------------------------------
char* rc522_type_to_string(PICC_TYPE_t type)
{

  return PICC_TYPE_STRING[type];

} /* rc522_type_to_string */


//----------------------------------------------------------------------------
// NAME: rc522_init
//
// DESCRIPTION:
//    This function initializes the RFID-RC522 module
//
// INPUT:
//   card_type -
//
// OUTPUT:
//   none
//
// RETURN:
//   TBD
//----------------------------------------------------------------------------
sint16 MFRC522_ParseType(uint8 TagSelectRet)
{
  sint16 return_type = PICC_TYPE_UNKNOWN;
  
  if (TagSelectRet & 0x04)
  {
    // UID not complete
    return_type = PICC_TYPE_NOT_COMPLETE;
  }

  switch (TagSelectRet)
  {
    case 0x09:
      return_type = PICC_TYPE_MIFARE_MINI;
      break;

    case 0x08:
      return_type = PICC_TYPE_MIFARE_1K;
      break;

    case 0x18:
      return_type = PICC_TYPE_MIFARE_4K;
      break;

    case 0x00:
      return_type = PICC_TYPE_MIFARE_UL;
      break;

    case 0x10:
    case 0x11:
      return_type = PICC_TYPE_MIFARE_PLUS;
      break;

    case 0x01:
      return_type = PICC_TYPE_TNP3XXX;
      break;

    default:
      break;
  } /* switch */

  if (TagSelectRet & 0x20)
  {
    return_type = PICC_TYPE_ISO_14443_4;
  } /* if */

  if (TagSelectRet & 0x40) 
  {
    return_type = PICC_TYPE_ISO_18092;
  } /* if */

  return (return_type);
  
} /* MFRC522_ParseType */


//----------------------------------------------------------------------------
// NAME: rc522_send_halt
//
// DESCRIPTION:
//    This function instructs a RFID card (PICC) that is in ACTIVE state to 
//    go to state HALT.
//
// INPUT:
//    none
//
// OUTPUT:
//    none
//
// RETURN:
//    none
//----------------------------------------------------------------------------
void rc522_send_halt(void)
{
  uint8 status;
  uint8 unLen;
  uint8 buff[4]; 

  //ISO14443-3: 6.4.3 HLTA command
  buff[0] = PICC_HALT;
  buff[1] = 0;
  
  rc522_calculate_CRC(buff, 2, &buff[2]);

  status = rc522_to_card(RC522_TRANSCEIVE_CMD, buff, 4, buff, &unLen);
  
} /* rc522_send_halt */




#if(0)
//Source files for reference:
// https://github.com/londonhackspace/mfrc522-energia/blob/master/examples/RC522DumpMifare/RC522DumpMifare.ino
//----------------------------------------------------------------------------
// NAME: MFRC522_Auth
//
// DESCRIPTION:
//    This function verify's the card's password. 
//
// INPUT:
//    authMode  - the a parameter defines the password verify mode
//                 0x60 = verify A passowrd key 
//                 0x61 = verify B passowrd key 
//    block_address - this represnets the block address 
//    sector_key    - this is a address pointer to uint8 that represents the 
//                    password for the block specified by the block address
//    serial_num    - this is a address pointer to the 4 bytes that  
//                    represents the serial number of the card
//
// OUTPUT:
//   none
//
// RETURN:
//   return MI_OK if successed
//----------------------------------------------------------------------------
uint8 MFRC522_Auth(uint8 authMode, uint8 block_address, uint8 *sector_key, uint8 *serial_num)
{
  uint8  status;
  uint16 data_received;
  uint8  i;
  uint8  buff[12]; 

  //Verify command + block address + buffer password + card SN
  buff[0] = authMode;
  buff[1] = block_address;
  
  for (i = 0; i < 6; i++)
  {    
    buff[i+2] = *(sector_key+i);   
  } /* for */
  
  for (i = 0; i < 4; i++)
  {    
    buff[i+8] = *(serial_num+i);   
  } /* for */
  
  status = rc522_to_card(RC522_AUTHENT_CMD, buff, 12, buff, &data_received);

  if ((status != MI_OK) || (!(rc522_read_reg(STATUS2_REG) & 0x08)))
  {   
      status = MI_ERR;   
  } /* if */
  
  return (status);
    
} /* MFRC522_Auth */


//----------------------------------------------------------------------------
// NAME: MFRC522_Read
//
// DESCRIPTION:
//    This function reads and returns the data on the RFID tag at a specific 
//    block address.
//
// INPUT:
//    block_address   - this represents the block address on the card to read
//
// OUTPUT:
//    data_received   - this represents address that points to the block 
//                      data which are read
//
// RETURN:
//   return MI_OK if successed
//----------------------------------------------------------------------------
uint8 MFRC522_Read(uint8 block_address, uint8 *data_received)
{
  uint8  status;
  uint16 unLen;

  data_received[0] = PICC_READ;
  data_received[1] = block_address;
  
  rc522_calculate_CRC(data_received,2, &data_received[2]);
  
  status = rc522_to_card(RC522_TRANSCEIVE_CMD, data_received, 4, data_received, &unLen);

  if ((status != MI_OK) || (unLen != 0x90))
  {
    status = MI_ERR;
  }
  
  return (status);
    
} /* MFRC522_Read */


//----------------------------------------------------------------------------
// NAME: MFRC522_Write
//
// DESCRIPTION:
//    This function writes data to a specific blcok address on the RFID tag.
//
// INPUT:
//    block_address   - this represents the block address on the card to read
//    data_received   - this represents address that points to the block 
//                      16 bytes of data to write
//
// OUTPUT:
//    none
//
// RETURN:
//   return MI_OK if successed
//----------------------------------------------------------------------------
uint8 MFRC522_Write(uint8 block_address, uint8 *writeData)
{
  uint8  status;
  uint16 data_received;
  uint8  idx;
  uint8  buff[18]; 
  
  buff[0] = PICC_WRITE;
  buff[1] = block_address;
  
  rc522_calculate_CRC(buff, 2, &buff[2]);
  
  status = rc522_to_card(RC522_TRANSCEIVE_CMD, buff, 4, buff, &data_received);

  if ((status != MI_OK) || (data_received != 4) || ((buff[0] & 0x0F) != 0x0A))
  {   
    status = MI_ERR;   
  } /* if */
      
  if (status == MI_OK)
  {
    // Write 16 bytes data into FIFO
    for (idx = 0; idx < 16; idx++)
    {    
      buff[idx] = *(writeData + idx);   
    } /* for */
    
    rc522_calculate_CRC(buff, 16, &buff[16]);
    
    status = rc522_to_card(RC522_TRANSCEIVE_CMD, buff, 18, buff, &data_received);
      
  if ((status != MI_OK) || (data_received != 4) || ((buff[0] & 0x0F) != 0x0A))
  {   
    status = MI_ERR;   
  } /* if */
  
  return (status);
    
} /* MFRC522_Write */


#if(0)
void dumpHex(char* buffer, int len)
{
  for(byte i=0; i < len; i++) 
  {
    char text[4];
    if (i % 16 == 0) 
    {
      Serial.print(" "); 
    }
    
    sprintf(text, "%02X \x00", (byte)(*(buffer + i)));
    Serial.print(text);

    if (i % 16 == 15) 
    {
      Serial.println(""); 
    } /* if */
  } /* for */
    //Serial.println(" "); 
}
#endif

#endif
//-----------------------------------------------------------------------------
//                             Private functions
//-----------------------------------------------------------------------------





//*****************************************************************************
//***************************** C Source Code *********************************
//*****************************************************************************
//
// DESIGNER NAME: Kushal & Frank
//
// FILE NAME: security_system.c
//
// DATE: 5/1/2023
//
// DESCRIPTION
// This source code contains a program that uses the Dragon12 board to
// simulate a security system.
//
//*****************************************************************************

#include <hidef.h>      /* common defines and macros */
#include <mc9s12dg256.h>     /* derivative information */
#pragma LINK_INFO DERIVATIVE "mc9s12dg256b"

#include "main_asm.h" /* interface to the assembly module */
#include "rfid_rc522.h"

// General constants
#define TRUE 1
#define FALSE 0

// LED constants
#define RGB_LED_WHITE 0x70
#define RGB_LED_RED 0x10
#define RGB_LED_GREEN 0x40
#define RGB_LED_BLUE 0x20
#define RGB_LED_YELLOW RGB_LED_RED | RGB_LED_GREEN
#define ALL_ON 0xFF

// LCD constants
#define LINES_PER_LCD 2
#define CHARACTERS_PER_LCD_LINE 16
#define TOTAL_CHARACTERS_PER_LCD = (LINES_PER_LCD * CHARACTERS_PER_LCD_LINE)
#define LCD_LINE_1_ADDR 0x00
#define LCD_LINE_2_ADDR 0x40
#define LCD_START_FIRST 0x00
#define LCD_LINE_2 0x40
#define FIRST_LINE_END 0x0F
#define SECOND_LINE_END 0x4F
#define BLANK_LINE_LCD "                "
#define PORT_H_INTERRUPT_VECTOR 25

// LCD Scroll constants
#define LCD_SCROLL_RATE 3 // Characters per increment

// Sensor constants
#define LIGHT_SENSOR_CHANNEL 4
#define LCD_SCROLL_DELAY_TIME 25
#define TEMP_CHANNEL 5

// SCI constants
#define SERIAL_COMMUNICATION_BAUD_RATE 9600
#define ENTER_KEY '\r'
#define NULL_STRING '\0'
#define BACKSPACE 0x7F

// Interrupt constants
#define RTI_VECTOR 7
#define ULTRASONIC_BITMASK 0x04
#define ULTRASONIC_VECTOR 10
#define SPEAKER_VECTOR 13

// Security System constants
#define ERROR   0
#define SECURITY_SYSTEM_HEADER "Security System v. 1.0.0"
#define NO_AUTHENTICATION 0
#define AUTHENTICATED_USER 1
#define AUTHENTICATED_ADMINISTRATOR 2
#define DIVIDER "=============================================\n\r"
#define ALARM_PITCH_1 957
#define ALARM_PITCH_2 1074
#define GOOD_BEEP_PITCH 957
#define GOOD_BEEP_DURATION 250
#define NEUTRAL_BEEP_PITCH 1434
#define NEUTRAL_BEEP_DURATION 150
#define ERROR_BEEP_DURATION 500
#define ERROR_BEEP_PITCH 2867

#define SYSTEM_STATUS_GOOD 2
#define SYSTEM_STATUS_OK 1
#define SYSTEM_STATUS_BAD 0

#define ADMINISTRATOR_UID_SEGMENT_1 0xBB
#define ADMINISTRATOR_UID_SEGMENT_2 0x85
#define ADMINISTRATOR_UID_SEGMENT_3 0x53
#define ADMINISTRATOR_UID_SEGMENT_4 0xB3
#define USER_UID_SEGMENT_1 0xAD
#define USER_UID_SEGMENT_2 0x2D
#define USER_UID_SEGMENT_3 0xFF
#define USER_UID_SEGMENT_4 0x30
#define ADMINISTRATOR_PIN_CHAR_1 1
#define ADMINISTRATOR_PIN_CHAR_2 2
#define ADMINISTRATOR_PIN_CHAR_3 3
#define ADMINISTRATOR_PIN_CHAR_4 4
#define MAX_PIN_TRIES 4

// Other constants
#define NEW_LINE "\n\r"
#define LCD_START_FIRST 0x00
#define READ_LIGHT_COMMAND "readlight"
#define READ_TEMP_COMMAND "readtemp"
#define SCAN_ENVIRONMENT_COMMAND "scan"
#define FLASH_LED_COMMAND "flash_led"
#define ALARM_ON_COMMAND "alarm_on"
#define DISABLE_ALARM_COMMAND "alarm_off"
#define LOW_ALERTNESS_COMMAND "alert_low"
#define MED_ALERTNESS_COMMAND "alert_med"
#define HIGH_ALERTNESS_COMMAND "alert_hig"
#define READ_MOTION_COMMAND "readmotion"
#define SENSOR_STATUS_GOOD 1
#define SENSOR_STATUS_OK 2
#define SENSOR_STATUS_BAD 3

#define ULTRASONIC_DELAY 15


// Ultrasonic constants
#define BUS_CLOCK 24E6
#define TIMER_PRESCALER 0x02
#define MS_PER_SECOND 1000
#define COUNTS_PER_SECOND (BUS_CLOCK / (1 << TIMER_PRESCALER))
#define COUNTS_PER_MS (uint16) (COUNTS_PER_SECOND/MS_PER_SECOND)
#define CHANNEL4_BITMASK (1 << 4)
#define SPEED_OF_SOUND 340 // 340 m/s (approx.)

#define SW2_BITMASK 0x08
#define SW5_BITMASK 0x01


// Global values
uint8 g_lightDetected = 0;
int g_alarm_on = TRUE;
unsigned short ticks, ticks0; // RTI interrupt counts
uint8 g_pitch;
uint8 gstatus_level = SYSTEM_STATUS_GOOD;
uint16 g_total_count = 0; // for isObjectNearby()
uint8 g_measurement_ready = FALSE; // for isObjectNearby()
uint16 g_light_threshold = 150;
uint16 g_temp_threshold = 90; // 90 F
uint16 g_motion_threshold = 200;
uint16 g_distance = 0;
uint8 g_user_level = NO_AUTHENTICATION;

// Function headers
void scroll_across_lcd_once(char message[]); // Scroll string across LCD once
int getLightLevel(void);                         // Returns current light level
uint8 getTempLevel(void);                        // Returns current temperature in Fahrenheit
void change_status_level(uint8 new_status);  // Changes system's status level
void scanEnvironment(void);                      // Scans environment for environmental hazards
void change_rgb_led_value(uint8 new_value);  // Changes color of RGB LED                                            
void beginAlarm(void);                           // Activates the alarm
void stopAlarm(void);                            // Disables the alarm
void print_console(sint8 buffer[70]);        // Prints string to the PUTTY console
void clear_lcd_line_2(void);                     // Clears LCD line 2
void successful_beep(void);                      // Beeps a tone indicating something happened successfully
void neutral_beep(void);                         // Beeps a tone indicating something happened
void error_beep(void);                           // Beeps a tone indicating an error occurred

// HELPER METHODS //

// -----------------------------------------------------------------------------
// DESCRIPTION
//   This function gets a string from the SCI and checks if it's equal to.
//   another string.
//
// INPUT PARAMETERS:
//   buffer_1        - The string from the SCI.
//   buffer_1_length - That string's length.
//   buffer_2        - The string we're comparing to.
//   buffer_2_length - That string's length
//
// RETURN:
//   equals_flag    - Whether or not they are equal.
//
// -----------------------------------------------------------------------------
int str_equals(char buffer_1[], int buffer_1_length, char buffer_2[], int buffer_2_length) {
  // Get string from SCI and check if it equals
  // a provided string,
  uint8 i;
  int equals_flag = TRUE;
  if (buffer_1_length != buffer_2_length) {
    return FALSE;
  }
  for (i = 0; i < buffer_1_length; i++)
  {
     if (buffer_1[i] != buffer_2[i]) {
         equals_flag = FALSE;
     }
  }
  return equals_flag;
}



// CONSOLE METHODS //
// -----------------------------------------------------------------------------
// DESCRIPTION
//   This function authenticates a user using a keycard and a PIN.
//
// -----------------------------------------------------------------------------
void authenticate(void)
{
      char password_buffer[30];
      char successful_authentication;
      uint8 admin_uid_segment_1 = ADMINISTRATOR_UID_SEGMENT_1;
  int i;
  int current_administrator_try = 1;      
      // RFID logic here
      uint8 _status = MI_OK;
      uint8 rc522_version = 0;
      uint8 card_tag_type;

      //Recognized card IDs
      uint8 card_id[5] = { 0x00,
     
      };

      uint8 tagType[16] = { 0x00,
     
      };
     
      uint8 current_pin;
      uint8 current_pin_idx = 0;
      uint8 pin_sequence[4];
     
      // Every keycard has a unique UID which
      // it transmits to the RFID sensor.
      // We will compare the transmitted UIDs
      // to these.
      uint8 correct_admin_uid[4] = {
        ADMINISTRATOR_UID_SEGMENT_1,
        ADMINISTRATOR_UID_SEGMENT_2,
        ADMINISTRATOR_UID_SEGMENT_3,
        ADMINISTRATOR_UID_SEGMENT_4    
  };

  uint8 correct_user_uid[4] = {
      USER_UID_SEGMENT_1,
      USER_UID_SEGMENT_2,
      USER_UID_SEGMENT_3,
      USER_UID_SEGMENT_4
  };

  uint8 correct_admin_pin[4] = {
      ADMINISTRATOR_PIN_CHAR_1,
      ADMINISTRATOR_PIN_CHAR_2,
      ADMINISTRATOR_PIN_CHAR_3,
      ADMINISTRATOR_PIN_CHAR_4
  };
      // Turn on SCI/terminal
      SCI1_init(SERIAL_COMMUNICATION_BAUD_RATE);
      SPI0_init();
      SS0_HI();

      print_console("Authenticating..\n\r");

      _status = rc522_init('B');
      successful_authentication = NO_AUTHENTICATION;

      if (_status == MI_OK)    // RFID is working
      {
            clear_lcd();
            set_lcd_addr(LCD_LINE_1_ADDR);
            type_lcd("Scan card");
            print_console("Checking for a present card..\n\r");
            while (successful_authentication == NO_AUTHENTICATION)
            {
                  ms_delay(MS_PER_SECOND);
                  _status = rc522_is_card_present(PICC_REQIDL, tagType);
                  if (_status == MI_OK)
                  {
                        print_console("RFID Card found\n\r");
                        _status = MFRC522_Anticoll(card_id);

                        if (_status == MI_OK)
                        {
                              // Print the card's UIDs
                              print_console("Card UID:");
                              alt_printf(" %02X ", card_id[0]);
                              alt_printf(" %02X ", card_id[1]);
                              alt_printf(" %02X ", card_id[2]);
                              alt_printf(" %02X ", card_id[3]);
          print_console("\n\r");
         
                              // Is user an admin or normal user?
                              if (card_id[0] == ADMINISTRATOR_UID_SEGMENT_1 &&
                                    card_id[1] == ADMINISTRATOR_UID_SEGMENT_2 &&
                                    card_id[2] == ADMINISTRATOR_UID_SEGMENT_3 &&
                                    card_id[3] == ADMINISTRATOR_UID_SEGMENT_4)
                              {
                                    print_console("Detected: Administrator\n\r");
                                    successful_authentication = AUTHENTICATED_ADMINISTRATOR;
                              } else if (card_id[0] == USER_UID_SEGMENT_1 &&
                                    card_id[1] == USER_UID_SEGMENT_2 &&
                                    card_id[2] == USER_UID_SEGMENT_3 &&
                                    card_id[3] == USER_UID_SEGMENT_4)
                              {
                                    print_console("Detected: User\n\r");
                                    successful_authentication = AUTHENTICATED_USER;
                              }

                             
                              // Notify user that card was detected. and print out their user level.
                              print_console("\n\r");
                              card_tag_type = rc522_select_tag(card_id);

                             
                              print_console("Card Selected, Type: ");
                              print_console(rc522_type_to_string(MFRC522_ParseType(card_tag_type)));

                              print_console("\n\r");
                              print_console("**********************************\n\r");
                              print_console("***    Remove RFID Card       ***\n\r");
                              print_console("**********************************\n\r");
                              print_console("\n\r");
                        } /* End if */
                  } /* End if */
            } /* End while */
      } /* End if */
     
      else // RFID not detected
      {
            print_console("Error.. RFID NOT WORKING\n\r");
            beginAlarm();
      }
     
  // The user has gotten past the point of scanning
      successful_beep();
      g_user_level = successful_authentication; // Set to admin or user before
      if (successful_authentication == AUTHENTICATED_ADMINISTRATOR) {
        print_console("Please enter your password using the keypad.");
        clear_lcd();
        set_lcd_addr(LCD_LINE_1_ADDR);
        type_lcd("Enter password");
     
        keypad_enable();
        // Give the user 3 chances to enter a valid 4-digit PIN
        while (current_administrator_try < MAX_PIN_TRIES - 1) {
          int n_successful_consecutive_sequence = 0;
          set_lcd_addr(LCD_LINE_2_ADDR);
          if (current_pin_idx > 3) {
             current_pin_idx = 0;
          }
          current_pin = getkey();
          wait_keyup();
          neutral_beep();
          pin_sequence[current_pin_idx] = current_pin;
          current_pin_idx++;
          // Check for a valid pin & display current pin
          clear_lcd_line_2();
          set_lcd_addr(LCD_LINE_2_ADDR);
         
          // Write entered pin to LCD and check if it's a valid PIN
          for (i = 0; i < current_pin_idx; i++) {
            uint8 current_pin_char = pin_sequence[i];
            if (current_pin_char) {
               data8(hex2asc(current_pin_char));
            }
            if (pin_sequence[i] == correct_admin_pin[i]) {
               n_successful_consecutive_sequence++;
            } else {
              n_successful_consecutive_sequence = 0;
            }
          }
         
          if (n_successful_consecutive_sequence == MAX_PIN_TRIES) {
             successful_beep();
             clear_lcd();
             successful_authentication == AUTHENTICATED_ADMINISTRATOR;
             break;
          } else if (current_pin_idx == 4) {
             error_beep();
             ms_delay(500);
             clear_lcd();
             current_administrator_try++;
             set_lcd_addr(LCD_LINE_1_ADDR);
             type_lcd("Attempt 2");
          }
           
        }
        if (successful_authentication != AUTHENTICATED_ADMINISTRATOR) {
          beginAlarm();
        }
        // Reached the 3rd try. Alarm on.
      }
}

// -----------------------------------------------------------------------------
// DESCRIPTION
//   This function prints a string to the SCI.
//
// INPUT PARAMETERS:
//   buffer - The string we want to print.
//
// -----------------------------------------------------------------------------
void print_console(sint8 buffer[70])      // Print text to console
{
      uint8 idx = 0;

      // while not at the end of the string
      while (buffer[idx] != '\0')
      {
            outchar1(buffer[idx++]);
      } /*while */

} /*print_console */

// -----------------------------------------------------------------------------
// DESCRIPTION
//   This function clears line 1 of the LCD.
//
// -----------------------------------------------------------------------------
void clear_lcd_line_1(void) {
  set_lcd_addr(LCD_LINE_1_ADDR);
  type_lcd(BLANK_LINE_LCD);
}

// -----------------------------------------------------------------------------
// DESCRIPTION
//   This function clears line 2 of the LCD.
//
// -----------------------------------------------------------------------------
void clear_lcd_line_2(void) {
  set_lcd_addr(LCD_LINE_2_ADDR);
  type_lcd(BLANK_LINE_LCD);
}

// -----------------------------------------------------------------------------
// DESCRIPTION
//   This function prints the commands available to a normal user on the SCI.
//
// -----------------------------------------------------------------------------
void printUserCommands(void)
{
  print_console(DIVIDER);
  print_console("Commands:\n\r");
  print_console("readlight  - Display information about the current light level in the area.\n\r");
  print_console("readtemp   - Display information about the current temperature in the area.\n\r");
  print_console("readmotion - Display information about the current motion level in the area.\n\r");
  print_console("scan       - Scan the environment for hazards.\n\r");  
}

// -----------------------------------------------------------------------------
// DESCRIPTION
//   This function sets the current alertness level.
//
// INPUT PARAMETERS:
//   alertness_level - The new alertness level.
//
// -----------------------------------------------------------------------------

void set_alertness(uint8 alertness_level)
{
  switch (alertness_level)
  { // If none chosen, don't edit anything
    // as this shouldn't happen!
    case 1: // Low alertness
      g_light_threshold = 150;
      g_temp_threshold  = 90;
      g_motion_threshold = 200;
      break;
    case 2: // Med alertness
      g_light_threshold = 100;
      g_temp_threshold  = 90;
      g_motion_threshold = 150;
      break;
    case 3: // High alertness
      g_light_threshold = 50;
      g_temp_threshold  = 90;
      g_motion_threshold = 80;
      break;
  }
}

// -----------------------------------------------------------------------------
// DESCRIPTION
//   This function prints the commands available to the user
//   depending on their authentication level, on the SCI.
//
// -----------------------------------------------------------------------------
void display_commands() {
  // SCI variables
  char buffer[30];
  char character;  
  int buffer_size = 0;
  int enter_pressed = 0;
  int i = 0;
 
  // Sensor variables
  int lightLevel;
  int tempLevel;
  int motionLevel;
  int motionStatus;
  int lightStatus;
  int tempStatus;  
 
  print_console("\n\r");
 
  if (g_user_level == AUTHENTICATED_USER) {
    printUserCommands();
  }
 
  else if (g_user_level == AUTHENTICATED_ADMINISTRATOR) {
      printUserCommands();
      print_console("alarm_on   - Activate the alarm system.\n\r");
      print_console("alarm_off  - Disable the alarm system.\n\r");
      print_console("flash_led  - Flash the LEDs.\n\r");
      print_console("alert_low  - Set the alertness level low\n\r");
      print_console("alert_med  - Set the alertness level medium\n\r");
      print_console("alert_hig  - Set the alertness level high\n\r");
  }
 
  print_console("Please enter the command that you'd like to execute: \n\r");
 
  while (!enter_pressed) {
    character = inchar1(); // Take characters from putty
    outchar1(character); // Immediately echo characters back into putty
    if (character == ENTER_KEY) {
       enter_pressed = TRUE;
   
       // See what command the user has entered
       if (str_equals(buffer, buffer_size, READ_LIGHT_COMMAND, 9)) { // If user wants to read light val
          print_console("\n\rReading light..");
          lightLevel = getLightLevel();
         
          lightStatus = getLightStatus(lightLevel);
         
          print_console("Light Level: "); // Display light level
          alt_printf("%d", lightLevel);
          print_console("\n\r");
       
          if (lightStatus == SENSOR_STATUS_BAD) { // Indicates intruder (via flashlight)
             print_console(".. HIGH LIGHT LEVEL - NOTIFY ADMINISTRATOR");
          } else if (lightStatus == SENSOR_STATUS_OK) // Suspicious light levels
          {
             print_console(".. SUSPICIOUS LIGHT LEVEL - CONSIDER NOTIFYING ADMINSITRATOR");
          } else
          {
            print_console(".. SAFE LEVEL");
          }
             
       } /* End if */
       
       // TODO: ADD READ MOTION COMMAND
       else if (str_equals(buffer, buffer_size, READ_TEMP_COMMAND, 8)) { // If user wants to read temp val
          print_console("\n\rReading temperature...");
         
          tempLevel = getTempLevel();
          tempStatus = getTempStatus(tempLevel);
          print_console("Temperature: ");
          alt_printf("%d", tempLevel);
          print_console("\n\r");
         
          if (tempStatus == SENSOR_STATUS_BAD) { // Indicates intruder (via flashlight)
             print_console(".. HIGH TEMP - NOTIFY ADMINISTRATOR");
          } else if (tempStatus == SENSOR_STATUS_OK) // Suspicious light levels
          {
             print_console(".. REACHING HIGH TEMP - CONSIDER NOTIFYING ADMINISTRATOR");
          } else
          {
            print_console(".. SAFE LEVEL");
          }
       }
       else if (str_equals(buffer, buffer_size, READ_MOTION_COMMAND, 10))
       {
          print_console("\n\rReading motion level...");
         
          motionLevel = getMotionLevel();
          motionStatus = getMotionStatus(motionLevel);
          print_console("Motion level: ");
          alt_printf("%d", motionLevel);
          print_console("\n\r");
         
          if (motionStatus == SENSOR_STATUS_BAD) { // Indicates intruder (via flashlight)
             print_console(".. HIGH MOTION - NOTIFY ADMINISTRATOR");
          } else if (motionStatus == SENSOR_STATUS_OK) // Suspicious light levels
          {
             print_console(".. REACHING MOTION - CONSIDER NOTIFYING ADMINISTRATOR");
          } else
          {
            print_console(".. SAFE LEVEL");
          }
 
       }
       
           // If user wants to scan environment
       else if (str_equals(buffer, buffer_size, SCAN_ENVIRONMENT_COMMAND, 4)) {
          scanEnvironment();
       }
         // If user wants to turn on alarms
         else if (str_equals(buffer, buffer_size, ALARM_ON_COMMAND, 8) && (g_user_level == AUTHENTICATED_ADMINISTRATOR)) {
            beginAlarm();
         }
       
         // If user wants to flash LEDs
         else if (str_equals(buffer, buffer_size, FLASH_LED_COMMAND, 9) && (g_user_level == AUTHENTICATED_ADMINISTRATOR)) {
            led_enable();
              for (i = 0; i < 5; i++) {
                        leds_on(ALL_ON);
                        ms_delay(100);
                        leds_off();
                        ms_delay(100);
              } /* End for loop */
         } /* End else if */
       
         // If user wants to disable alarms
         else if (str_equals(buffer, buffer_size, DISABLE_ALARM_COMMAND, 9) && (g_user_level == AUTHENTICATED_ADMINISTRATOR)) {
               stopAlarm();
         }
       
         // If user wants to set alertness level low
         else if (str_equals(buffer, buffer_size, LOW_ALERTNESS_COMMAND, 9)  && (g_user_level == AUTHENTICATED_ADMINISTRATOR)) {
               set_alertness(1);
           print_console("\n\rNEW ALERTNESS LEVEL: LOW");
         }
       
         // If user wants to set alertness level medium
         else if (str_equals(buffer, buffer_size, MED_ALERTNESS_COMMAND, 9) && (g_user_level == AUTHENTICATED_ADMINISTRATOR)) {
               set_alertness(2);
         print_console("\n\rNEW ALERTNESS LEVEL: MED");
         }
       
         // If user wants to alertness level high
         else if (str_equals(buffer, buffer_size, HIGH_ALERTNESS_COMMAND, 9) && (g_user_level == AUTHENTICATED_ADMINISTRATOR)) {
               set_alertness(3);
         print_console("\n\rNEW ALERTNESS LEVEL: HIGH");
         }
       else {
          print_console("Error: Invalid command!");
       }
       
    } else { // Character wasn't enter key
      if (character == BACKSPACE) {
         buffer[buffer_size] = NULL_STRING;
         buffer_size--;
      } else {
         buffer[buffer_size++] = character;
      }
    } /* else */
  } /* while */
  print_console("\n\r");
} /* display_commands() */

// -----------------------------------------------------------------------------
// DESCRIPTION
//   This function scans the environment for hazards
//   and displays information about them to the user.                                            
// -----------------------------------------------------------------------------
void scanEnvironment(void) {
 
  int lightLevel;
  int tempLevel;
  int motionLevel;
  int lightStatus;
  int tempStatus;
  int motionStatus;
  int objectLevel;
  int objectStatus;
 
  print_console("Scanning environment..\n\r");
  lightLevel = getLightLevel();
  clear_lcd();
  lightStatus = getLightStatus(lightLevel);
  print_console("Light Level: ");
  alt_printf("%d", lightLevel);
 
  if (lightStatus == 3) { // Indicates intruder (via flashlight)
     print_console(".. TOO MUCH LIGHT - DANGEROUS LEVEL");
  }
  else if (lightStatus == 2) { // There may be an intruder
       print_console(".. SUSPICIOUS LIGHT LEVELS");
  }
  else {
     print_console(".. SAFE LEVEL  ");
  }                                
 
  print_console("\n\r");
 
  tempLevel = getTempLevel();
  tempStatus = getTempStatus(tempLevel);
  print_console("Temperature:");
  alt_printf("%d", tempLevel);
  if (lightStatus == 3) { // Indicates environmental temperature risk
     print_console(".. HIGH TEMPERATURE - DANGEROUS LEVEL");
  }
  else if (lightStatus == 2) {
       print_console(".. REACHING HIGH TEMPS");
  }
  else {
     print_console(".. SAFE LEVEL");
  }
 
  print_console("\n\r");
 
  motionLevel = getMotionLevel();
  motionStatus = getMotionStatus(motionLevel);
  print_console("MOTION:");
  alt_printf("%d", motionLevel);
  clear_lcd();
 
  if (motionStatus == 3) { // Indicates environmental motion risk
     print_console(".. HIGH MOTION - DANGEROUS LEVEL");
  }
  else if (motionStatus == 2) {
       print_console(".. SUSPICIOUS MOTION LEVELS");
  }
  else {
     print_console(".. SAFE LEVEL");
  }
 
  print_console("\n\r");
  objectLevel = g_distance;
  objectStatus = isObjectNearby();
  print_console("DISTANCE FROM OBJECT:");
  alt_printf("%d",objectLevel);
 
  if (objectStatus == SENSOR_STATUS_BAD) { // Indicates environmental motion risk
     print_console(".. OBJECT NEARBY - NOTIFY ADMINISTRATOR");
  }
  else if (motionStatus == SENSOR_STATUS_OK) {
       print_console(".. OBJECT MAY BE NEARBY - CONSIDER NOTIFYING ADMINISTRATOR");
  }
  else {
     print_console(".. SAFE LEVEL");
  }
 
  // Update status based on environmental hazard levels
  // This is done rather than updating in the previous blocks of code
  // to ensure a lower status doesn't override a higher status.
  // (e.g. lightStatus = 3, tempStatus == 2, so status is OK)
  if ((lightStatus == SENSOR_STATUS_BAD) || (tempStatus == SENSOR_STATUS_BAD) || (motionStatus == SENSOR_STATUS_BAD) || (objectStatus == SENSOR_STATUS_BAD))
  {
        change_status_level(SYSTEM_STATUS_BAD);
  } else if ((lightStatus == 2) || (tempStatus == SENSOR_STATUS_OK) || (motionStatus == SENSOR_STATUS_OK) || (objectStatus == SENSOR_STATUS_OK))
  {
    change_status_level(SYSTEM_STATUS_OK);
  } else
  {
    change_status_level(SYSTEM_STATUS_GOOD);
  }
 
  print_console("\n\r");  
}

// -----------------------------------------------------------------------------
// DESCRIPTION
//   This function displays the initial messages shown to
//   the user on the SCI.
//
// -----------------------------------------------------------------------------
void display_initial_console_message(void) {
  // Display header
  alt_clear();
  print_console(DIVIDER);
  print_console(SECURITY_SYSTEM_HEADER);
  print_console(NEW_LINE);
  print_console(DIVIDER);
 
  // Security levels:
  //  Authenticated User - [DESCRIBE PERMISSIONS]
  //  Authenticated Administrator - [DESCRIBE PERMISSIONS]
  if (g_user_level == AUTHENTICATED_USER) {
     // Display text on console
     print_console("Welcome user, to ");
     print_console(SECURITY_SYSTEM_HEADER);
     print_console(NEW_LINE);
     print_console("You are currently logged in as a User.\n\r");
     
     print_console("What would you like to do today?\n\r");
     
     // Display text on LCD
     set_lcd_addr(LCD_LINE_1_ADDR);
     type_lcd("Welcome, user.");
     set_lcd_addr(LCD_LINE_2_ADDR);
     scroll_across_lcd_once("All systems are operational.");
     clear_lcd_line_2();
     
  } else if (g_user_level == AUTHENTICATED_ADMINISTRATOR) {
     // Display text on console
     print_console("Welcome Administrator, to ");
     print_console(SECURITY_SYSTEM_HEADER);
     print_console(NEW_LINE);
     print_console("You are currently logged in as an Administrator.");
     print_console("What would you like to do today?\n\r");
  } else {
     print_console("Please scan your keycard to log in.\n\r");
  }
}


// ALARM METHODS //
// -----------------------------------------------------------------------------
// DESCRIPTION
//   This function activates the system's alarm.
//
// -----------------------------------------------------------------------------
void beginAlarm(void) {
  // Turn alarm + LEDs on
  g_alarm_on = TRUE;
  sound_init();
  sound_on();
  led_enable();
  change_rgb_led_value(RGB_LED_RED);
  while (g_alarm_on == TRUE) {
    // Play noise + flash lights
    g_pitch = ALARM_PITCH_1;
    leds_on(ALL_ON);
    ms_delay(100);
    g_pitch = ALARM_PITCH_2;
    leds_off();
    ms_delay(100);
  }
}

// -----------------------------------------------------------------------------
// DESCRIPTION
//   This function deactivates the system's alarm.
//
// -----------------------------------------------------------------------------
void stopAlarm(void) {
  g_alarm_on = FALSE;
  sound_off();
  asm { // sound_off() disables interrupts,
        // so clear them
    cli
  }
}

// -----------------------------------------------------------------------------
// DESCRIPTION
//   This function changes the RGB LED's color.
//
// INPUT PARAMETERS:
//   new_value - The color you wish to change it to.
//
// -----------------------------------------------------------------------------
void change_rgb_led_value(uint8 new_value) {
   DDRP = 0x70; // Set RGB to outputs  
   DDRM = ~0x04; // Enable the RGB LED
   PTM = ~0x04;
   PTP = new_value;
}

// -----------------------------------------------------------------------------
// DESCRIPTION
//   This function sets the system's current status level.
//
// INPUT PARAMETERS:
//   new_status - The new status level.
//
// -----------------------------------------------------------------------------
void change_status_level(uint8 new_status) {
    if (new_status <= SYSTEM_STATUS_GOOD) {
      // Update _status + change RGBs
      gstatus_level = new_status;
      if (new_status == SYSTEM_STATUS_BAD) {
        beginAlarm();
        change_rgb_led_value(RGB_LED_RED);
        clear_lcd();
        set_lcd_addr(LCD_LINE_1_ADDR);
        type_lcd("WARNING");
        scroll_across_lcd_once("SECURITY CONCERN!");
       
      } else if (new_status == SYSTEM_STATUS_OK) {
        change_rgb_led_value(RGB_LED_YELLOW);
     
      } else if (new_status == SYSTEM_STATUS_GOOD) {
        stopAlarm();
        change_rgb_led_value(RGB_LED_GREEN);
      }
    }
}

// -----------------------------------------------------------------------------
// DESCRIPTION
//   This function is an ISR for the switches. It detects switch presses.
//
// -----------------------------------------------------------------------------
void interrupt PORT_H_INTERRUPT_VECTOR switch_handler() {
  int clear_bits = 0;
  int switchValue = PIFH;
 
  // If SW2 pressed
  if ((switchValue & SW2_BITMASK) == SW2_BITMASK && g_user_level == AUTHENTICATED_ADMINISTRATOR) {
    stopAlarm();
    clear_bits |= SW2_BITMASK;
  }

  PIFH = clear_bits;
}

// -----------------------------------------------------------------------------
// DESCRIPTION
//   This function is an ISR for the speaker. It plays a beep noise.
//
// -----------------------------------------------------------------------------
void interrupt SPEAKER_VECTOR handler()
{
  tone(g_pitch);
}

// -----------------------------------------------------------------------------
// DESCRIPTION
//   This function plays a beep on the speaker indicating a successful action
//   has occurred.,
//
// -----------------------------------------------------------------------------
void successful_beep()
{
      g_pitch = GOOD_BEEP_PITCH;
      sound_init();
      sound_on();
      led_enable();
      change_rgb_led_value(RGB_LED_GREEN);
      ms_delay(GOOD_BEEP_DURATION);
      led_off(0xFF);
      sound_off();
      asm
      {
            // sound_off() disables interrupts,
            // so clear them
            cli
      }
}

// -----------------------------------------------------------------------------
// DESCRIPTION
//   This function plays a beep on the speaker indicating an action has occurred,
//
// -----------------------------------------------------------------------------
void neutral_beep() {
  g_pitch = NEUTRAL_BEEP_PITCH;
      sound_init();
      sound_on();
      led_enable();
      change_rgb_led_value(RGB_LED_GREEN);
      ms_delay(NEUTRAL_BEEP_DURATION);
      led_off(0xFF);
      sound_off();
      asm
      {
            // sound_off() disables interrupts,
            // so clear them
            cli
      }
}

// -----------------------------------------------------------------------------
// DESCRIPTION
//   This function plays a beep on the speaker indicating an
//   error has occurred.
//
// -----------------------------------------------------------------------------
void error_beep() {
  g_pitch = ERROR_BEEP_PITCH;
      sound_init();
      sound_on();
      led_enable();
      change_rgb_led_value(RGB_LED_RED);
      ms_delay(ERROR_BEEP_DURATION);
      led_off(0xFF);
      sound_off();
      asm
      {
            // sound_off() disables interrupts,
            // so clear them
            cli
      }
}


// SENSOR METHODS //

// -----------------------------------------------------------------------------
// DESCRIPTION
//   This function returns the current light level.
//
// RETURN
//   lightLevel - The current light level.
// -----------------------------------------------------------------------------
int getLightLevel(void) {
  return ad0conv(LIGHT_SENSOR_CHANNEL);
}

// -----------------------------------------------------------------------------
// DESCRIPTION
//   This function returns the current temperature.
//
// RETURN
//   tempLevel - The current temperature.
// -----------------------------------------------------------------------------
uint8 getTempLevel(void) {
  uint8 temp;
 
  temp = ad0conv(TEMP_CHANNEL); // Get temperature from AD0 Channel 5
  temp = temp >> 1; // Divide by 2 to convert to Celsius
  temp = (temp * 9/5) + 32; // Convert from C to F
     
  //highTempDetected = (temp >= HIGH_TEMP);
  return temp;
}

// -----------------------------------------------------------------------------
// DESCRIPTION
//   This function returns the motion level using an accelerometer.
//
// RETURN
//   tempLevel - The current motion level.
// -----------------------------------------------------------------------------
int getMotionLevel(void) {
  // Uses X, Y, and Z components of acceleration
  // and if there's high acceleration, high motion detected
 
  int accX, accY, accZ, accTotal = 0;
 
  // Get components from channels 0, 1, and 2 of AD1
  accX = ad1conv(0);
  accY = ad1conv(1);
  accZ = ad1conv(2);

  accTotal = accX + accY + accZ;
  return accTotal;
}

// -----------------------------------------------------------------------------
// DESCRIPTION
//   This function returns the current motion sensor status (good, ok, bad)
//
// RETURN
//   motionStatus - The current motion sensor status.
// -----------------------------------------------------------------------------
int getMotionStatus(int motionLevel)
{
   uint8 motionBuffer = 10;
   
   if (motionLevel < (g_motion_threshold - motionBuffer)) {
        return SENSOR_STATUS_GOOD; // Good motion
   }
   else if ((motionLevel > (g_motion_threshold - motionBuffer)) && (motionLevel < g_motion_threshold)) {
        return SENSOR_STATUS_OK; // Med motion
   }
   else {
        return SENSOR_STATUS_BAD; // Bad motion
   }
}

// -----------------------------------------------------------------------------
// DESCRIPTION
//   This function returns the current temp sensor status (good, ok, bad)
//
// RETURN
//   tempStatus - The current temp sensor status.
// -----------------------------------------------------------------------------
int getTempStatus(int temp) {
   uint8 tempBuffer = 10;
   
   if (temp < (g_temp_threshold - tempBuffer)) {
        return SENSOR_STATUS_GOOD; // Good temp
   }
   else if ((temp > (g_temp_threshold - tempBuffer)) && (temp < g_temp_threshold)) {
        return SENSOR_STATUS_OK; // Med temp
   }
   else {
        return SENSOR_STATUS_BAD; // Bad temp
   }
}

// -----------------------------------------------------------------------------
// DESCRIPTION
//   This function returns the current light sensor status (good, ok, bad)
//
// RETURN
//   lightStatus - The current light sensor status.
// -----------------------------------------------------------------------------
int getLightStatus(int lightValue) {
   uint8 lightBuffer = 10;
   
   if (lightValue < (g_light_threshold - lightBuffer)) {
        return SENSOR_STATUS_GOOD; // Good light
   }
   else if ((lightValue > (g_light_threshold - lightBuffer)) && (lightValue < g_light_threshold)) {
        return SENSOR_STATUS_OK; // Med light
   }
   else {
        return SENSOR_STATUS_BAD; // Bad light
   }
}

// -----------------------------------------------------------------------------
// DESCRIPTION
//   This function is an ISR for the ultrasonic sensor. It's used
//   to calculate distance from the microcontroller.
//
// -----------------------------------------------------------------------------
void interrupt ULTRASONIC_VECTOR echo_handler() { // Ultrasonic sensor ISR
  static uint16 start_tcnt = 0;
 
  if ((PTT & ULTRASONIC_BITMASK) == ULTRASONIC_BITMASK) // if echo is high (rising edge)
  {
    start_tcnt = TC2;
  }
  else // if else is low (falling edge)
  {
    // calculate pulse width + save to global
    g_total_count = TC2 - start_tcnt;
    g_measurement_ready = TRUE;
   
  }
 
  // turn on channel 1 flag
  TFLG1 = TFLG1 | 0x04;
}

// -----------------------------------------------------------------------------
// DESCRIPTION
//   This function detects if an object is near the microcontroller
//   using the ultrasonic sensor.
//
// -----------------------------------------------------------------------------
int isObjectNearby(void) { // Uses ultrasonic sensor
  return (g_distance); // From ultrasonic ISR (echo_handler)
}


// LCD METHODS //

// -----------------------------------------------------------------------------
// DESCRIPTION
//   This function splits a string into chunks usable by the LCD.
//
// INPUT PARAMETERS:
//   string - The string the function is going to be splitting.
//   start_lcd_addr - The starting point of the string on the LCD.
//   max_lcd_addr - The end of the string on the LCD.
//
// -----------------------------------------------------------------------------
void lcd_string_parser(char string[], char start_lcd_addr, char max_lcd_addr)
{
  // Split a long string up so that it
  // displays on the LCD, and display the
  // smaller subsection
  int index = 0;
  int max_index = max_lcd_addr - start_lcd_addr;
  while (string[index] != '\0' && index <= max_index)
  {
    data8(string[index]);
    index++;
  }
}

// -----------------------------------------------------------------------------
// DESCRIPTION
//   This function scrolls a message across the LCD once.
//
// INPUT PARAMETERS:
//   message - The string the function is going to be splitting.
//
// -----------------------------------------------------------------------------
void scroll_across_lcd_once(char message[]) {
  unsigned char msg_idx = 0;
  unsigned int i = 0;  
  char lcd_char_address;
  int current_lcd_addr = LCD_LINE_2_ADDR;
  set_lcd_addr(LCD_LINE_2_ADDR);
 
  // Write portion of long string to LCD
  for (current_lcd_addr = LCD_LINE_2_ADDR + CHARACTERS_PER_LCD_LINE; current_lcd_addr >= LCD_LINE_2_ADDR; current_lcd_addr-=LCD_SCROLL_RATE) {
    ms_delay(LCD_SCROLL_DELAY_TIME);
    clear_lcd_line_2();
    ms_delay(LCD_SCROLL_DELAY_TIME);
    set_lcd_addr(current_lcd_addr);
    lcd_string_parser(message, current_lcd_addr, SECOND_LINE_END);
  }
 
  // Since we are scrolling across the LCD LCD_SCROLL_RATE at a time,
  // we have to check if the msg_idx is bound to be the null
  // terminator, so that we scroll properly.  
 
 
  while (message[msg_idx] != '\0') {
    int null_terminator_bound = 3;
    for (i = 1; i < (LCD_SCROLL_RATE + 1); i++) {
       if (message[msg_idx + i] == '\0') { // A null character is bound
          null_terminator_bound = i;
          break;
       }
    }
    ms_delay(LCD_SCROLL_DELAY_TIME);
    clear_lcd_line_2();
    ms_delay(LCD_SCROLL_DELAY_TIME);
    set_lcd_addr(LCD_LINE_2_ADDR);
    msg_idx += null_terminator_bound;
    lcd_string_parser(&message[msg_idx], current_lcd_addr, SECOND_LINE_END);
  }
}

void main(void) {
  uint8 DONE = FALSE;

  // Ultrasonic stuff
  uint16 seconds;                              
 
  // Initialize peripherals
  PLL_init();
  lcd_init();
  ad1_enable();
  ad0_enable();

  SCI1_init(SERIAL_COMMUNICATION_BAUD_RATE);
  alt_clear();
  change_status_level(SYSTEM_STATUS_GOOD);
  led_enable();
 
  authenticate();
  display_initial_console_message();

  _asm CLI // Clear interrupts so we can use the ultrasonic timer ISR
  TIE != CHANNEL4_BITMASK;
 
  // Set up switch ISR

  // Below are some magic numbers, but it'd be redundant to define 
  // them for the timer simply to get rid of them.  
  
  PIFH = 0xFF;
  PPSH = 0x00;
  PIEH = SW2_BITMASK | SW5_BITMASK;
 
  TIE  = 0x04;
  TIOS  = 0x10; // Config. channel 4 (TRIG) as OC
  TSCR2 = 0x04; // set time for 1.5 mhz
  TSCR1 = 0x80; // start timer (tcnt) running
 
  // Timer control variables
  TCTL1 = 0;
  TCTL2 = 0;
  TCTL3 = 0;
  TCTL4 = 0x30;

    // Send first trigger
  TCTL1 = 0x03; // channel 4 goes high when tc4 & tcnt match
  CFORC = 0x10; // force a tc4 & tcnt match
  TCTL1 = 0x02;  // channel 4 goes low when tc4 + tcnt match
  TC4 = TCNT + ULTRASONIC_DELAY; // set tc4 15 counts ahead of tcnt
  while (TRUE) {
    if (g_measurement_ready) {
      g_measurement_ready = FALSE;
 
      g_distance = (uint16)((uint32)g_total_count * (uint32)SPEED_OF_SOUND* (uint32)(MS_PER_SECOND)/2 * (float)(1/COUNTS_PER_SECOND));
   
      // Send first trigger
      TCTL1 = 0x03; // channel 4 goes high when tc4 & tcnt match
      CFORC = 0x10; // force a tc4 & tcnt match
      TCTL1 = 0x02;  // channel 4 goes low when tc4 + tcnt match
      TC4 = TCNT + 15; // set tc4 15 counts ahead of tcnt  
    }
   
    display_commands();
  }  
}
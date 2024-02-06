# 1 - Summary
## 1.1 - Overview
An automated security system helps to maintain the security and integrity of a company and allows for temporary security suspension and variable modifications at a distance. This system allows a company to live without worries regarding security concerns, knowing that the automated system will take care of it. Additionally, they don’t have to invest in employees to watch the cameras every day. 
This Automated Security System keeps track of the overall welfare of the environment, using the following components.

## 1.2 - Components
1.	Dragon12-Light Rev D development board: The primary component of the project, responsible for processing and managing the collected data from connected sensors.
2.	Light level sensor (included with the Dragon12 board): To monitor the light levels in designated areas and detect any unexpected changes.
3.	Ultra sonic sensor: To detect movement within a specified range and identify potential intruders.
4.	Temperature sensor (included with the Dragon12 board): To monitor temperature changes in critical areas and detect potential fire hazards or unauthorized access.
5.	Speaker (included with the Dragon12 board): To sound alarms for security reasons. 
6.	RGB LED (included with the Dragon12 Board): To provide a visual signal for the current status condition. 
a.	Green is used to indicate a safe environment. 
b.	Yellow is used to indicate the presence of a potential security risk. 
c.	Red is used to indicate the presence of a certain security risk.
7.	LEDs (included with the Dragon12 board): To provide visual signals for alarms.
8.	1602 LCD Module (included with the Dragon12 Board): To display the information about the currently scanned user and other security-related data. 
9.	RFID reader and RFID tags: To authenticate users of different administration levels (User, Administrator) for accessing the system.
a.	HW-131
b.	RFID-RC522 
10.	Accelerometer (included with the Dragon12 Board): To detect tampering with the security system. 
11.	Computer: Used as a reliable power source for the Dragon12-Light Rev D development board and connected sensors and is also used with a computer terminal. 
12.	9V Battery – Used to provide power to the HW-131.
13.	Breadboard and jumper wires: For connecting and prototyping the sensors with the development board.
14.	FTDI cable: To interface the microcontroller with a computer terminal for displaying data and debugging.
a.	Connected to the computer via USB. 

## 1.3 - Additional Features
This Automated Security System also provides multi-factor authentication for administrators. While others may rely on solely a Keycard or pin, this Automated Security System utilizes both keycard and pin for administrative login. If a third-party were able to access the keycard, they’d also need to type the correct pin after the keycard has been scanned. If an incorrect pin is typed within 2 attempts, the automatic alarm system will sound. 
Additionally, the RGB LEDs keep track of the overall status level in the area, to give operators time to prepare for suspicious activity. The Automated Security System has preset thresholds that define the levels of security risk (maximum light level to be considered moderately risky, maximum temperature level to be considered heavy risk, etc.) Administrators can change these thresholds upon logging in. 

# 2 - Future Work Needed
## 2.1 - Infrared Sensor
In addition to the existing components, one area of future work for the Automated Security System is the incorporation of an infrared sensor. The infrared sensor would further enhance the system's capability to detect and respond to potential security threats. The infrared sensor can be used to detect heat signatures and motion in the designated areas. By analyzing the heat patterns, the system can identify human presence even in low-light or dark environments, making it more effective in detecting intruders or unauthorized access attempts during nighttime or in areas with limited visibility. Integrating an infrared sensor into the system would provide an additional layer of security, complementing the existing sensors such as the light level sensor, ultrasonic sensor, and temperature sensor. It would enable the system to detect and respond to potential security risks that may go unnoticed by other sensors alone. The data collected from the infrared sensor can be processed and analyzed by the Dragon12-Light Rev D development board, like other sensors. To implement the infrared sensor, proper hardware integration and software programming would be required. The system would need to be calibrated to accurately interpret the data from the sensor and adjust the security thresholds accordingly. Additionally, the user interface and controls may need to be updated to accommodate the new sensor and provide administrators with the ability to customize its settings.
## 2.2 - Ultrasonic Sensor
Another area of work is the addition of a higher range ultrasonic sensor, which would allow for longer range detection capabilities. Currently, the Automated Security System utilizes an ultrasonic sensor to detect movement within a specified range. However, by incorporating a higher range ultrasonic sensor, the system would be able to extend its reach and detect potential intruders or unauthorized activities at greater distances. The higher range ultrasonic sensor would expand the system's coverage area, making it particularly useful for securing larger spaces or outdoor environments. It would provide an increased field of detection, enabling the system to monitor a broader range and identify potential threats even before they approach the immediate vicinity of the protected area.

## 2.3 - Biometric Authentication
To further enhance the security measures, the Automated Security System can explore the implementation of additional layers of authentication. While the current system utilizes both a keycard and a pin for administrative login, it could benefit from incorporating additional factors such as biometric authentication: fingerprint or facial recognition. By introducing biometric authentication, the system would require administrators to provide their unique biometric data in addition to the keycard and pin. This additional layer of authentication adds an extra level of security, as it ensures that only authorized individuals with approved biometric features can access the system. It significantly reduces the chances of unauthorized access, even in the event of keycard theft or pin compromise. If an administrator forgets the pin, they would be prompted with security questions and biometric authentication to allow them to reset the pin.

## 2.4 - Improvements to Current Features
Instead of the Automated Security System’s command-driven menu, it would be more beneficial to the Administrators and Users to have a number-driven menu, where operators can simply type in numbers as input. By utilizing this, they wouldn’t have to spend time unnecessarily typing full-length words, especially in emergencies. Additionally, it may have been more streamlined if the Automated Security System relied on the LCD and keypad more than the PuTTY terminal. This approach would’ve represented a more compact and typical control panel interface. Finally, by creating separate software to control the whitelist of the card serial IDs instead of hard coding it would allow for a more robust and user-friendly approach, as authentication would not rely on a specific set of keycards. Likewise, using encrypted RFID cards would protect the system from skimming, ensuring greater security. 

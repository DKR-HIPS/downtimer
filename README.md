# DownTimer
<b>DownTimer - Arduino-controlled timer switch with LCD and RTC</b>

This project is about an Arduino-controlled timer switch to control the usage of an electrical device. It has a daily time contingent which is reset every morning. The one-button operation allows to start/stop the device, and the time already used is remembered also in case it would be unplugged. This is achieved by using the EEPROM which is present on the DS3231-type realtime clock module. Information about the current state is displayed on the LC display.

The specific use case for which this project was designed, is to control the network-connected time on a gaming PC. When the daily contigent is used up, it switches off the network connection by cutting off the power from a tiny Ethernet switch. Everything is assembled together in a compact housing, so that the Ethernet and pwer cables are not easily unplugged to circumvent the down-time. <b>Note:</b> this thing is meant as a help for self-control of network time spent on the gaming PC, not as a stringent method to block it. The default time contingent is 240 minutes but this can be easily configured in the code. There are some additional features: a "reserve time" mode where an extra 15 minutes can be activated, but these will be deduced from the next-day contingent. The "chance time" feature is an option to spare more than 60 minutes of the daily allowed time; when this done two days in a row, it will allow the additional time on the third day, plus a small random number of minutes as reward.

<img src=DownTimer_case.jpg width=60%>

## How to build it

<b>Hardware setup:</b> I used an Arduino Pro Mini for this project, but of course other boards such as the UNO will do also. The wiring diagram is shown below. Wire the LCD and RTC modules with +5V and GND. The I2C data wires from LCD and RTC module are connected to pins A4 (SDA) and A5 (SCL). In addition, pin 8 controls the relais via transistor T1. The start/stop button connects pin 7 to GND. Two jumpers are optionally placed between Arduino pins 3 and 6 to GND. Another option is a speaker (8R with 100R serial resistor) for audio signals. The electric circuit of the Arduino and the switched device are kept strictly separate. This is on purpose to eliminate any interference via the power lines (it also allows to switch a voltage other than 5V if needed by the device).

<br>
<img src=DownTimer_wiring.png width=80%>

<b>Operation:</b> Ater uploading the program to the board using the Arduino IDE, hold down the button for more than two seconds to enter the date/time setup. Becaude the DS3231 RTC module is battery backed-up it keeps the time up-to-date even without external power. Note that jumper on pin 6 can be set, to disable access to the date/time etup during power-up. This is to reduce the risk for self-cheating in the usage scenario described above. The jumper on pin 3 disables the code reading/writing the EEPROM on the DS3231 which can be useful when experimenting with the program.



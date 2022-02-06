# Down-Timer
<b>Down-Timer - Arduino-controlled timer switch with LCD and RTC</b>

This project is about an Arduino-controlled timer switch to control the usage of an electrical device. It has a daily time contingent which is reset every morning. The one-button operation allows to start/stop the device, and the time already used is remembered also in case it would be unplugged. This is achieved by using the EEPROM which is present on the DS3231-type realtime clock module. Information about the current state is displayed on the LC display.

The specific use case for which this project was designed, is to control the network-connected time on a gaming PC. When the daily contigent is used up, it switches off the network connection by cutting off the power from a tiny Ethernet switch. Everything is assembled together in a compact housing, so that the Ethernet and power cables would not be too easily unplugged to circumvent the down-time. <b>Note:</b> this thing is meant as a help for self-control of network time spent on the gaming PC, not as a stringent method to block it. The default time contingent is 240 minutes but this can be easily configured in the code. There are some additional features: a "reserve time" mode where an extra 15 minutes can be activated, but these will be deduced from the next-day contingent. The "chance time" feature is an option to spare more than 60 minutes of the daily allowed time; when this was done two days in a row, it will allow the additional time on the third day, plus a small random number of minutes as reward.

<img src=DownTimer_case.jpg width=60%>

## How to build it

<b>Hardware setup:</b> I used an Arduino Pro Mini for this project, but of course other boards such as the UNO will do also. The wiring diagram is shown below. Wire the LCD and RTC modules with +5V and GND. The I2C data wires from LCD and RTC module are connected to pins A4 (SDA) and A5 (SCL). In addition, pin 8 controls the relais state via transistor T1. Dont' forget thy tpical protection diode anti-parallel to the relay coil. The start/stop button connects pin 7 to GND. Two jumpers are optionally placed between Arduino pins 3 and 6 to GND. Another option is a small speaker (I used 8&#8486; with 100&#8486; serial resistor) for audio signals. The electric circuit of the Arduino and the switched device are kept strictly separate. This is on purpose to eliminate any interference via the power lines (it also allows to switch a voltage other than 5V if needed by the device). The 220 µF capacitor was included to improve stability of the 5V power when the relais switches.

<br>
<img src=DownTimer_wiring.png width=80%>

<b>Operation:</b> After uploading the program to the board using the Arduino IDE, hold down the button for more than two seconds to enter the date/time setup during first power-up. Because the DS3231 RTC module is battery backed-up it maintains the current time even without external power. Note that the jumper on pin 6 can be set, to disable access to the date/time setup during power-up. This is to reduce the risk for self-cheating in the usage scenario described above. The jumper on pin 3 prevents the code from reading/writing the EEPROM on the DS3231 which can be useful when experimenting with the program.

During operation the LC display will toggle between showing the current date/time and the remaining credit or runtime, plus the ON/OFF state. Pressing the button shortly will switch on/off the LCD backlight. Holding it for a second will start the countdown or pause it when it was already running. When the countdown time (default: 240 minutes) was fully used it will change the device status to BLOCKED. The button can then be used to request the reserve time (default: 15 minutes), this will also work while the last minute of the countdown is running. If reserve time is used, it will be deduced form the time contingent of the following day.
When the countdown is stopped with more than 60 minutes left, AND this is done on two consecutive days, it will add the saved time plus a small bonus on the third day (this is meant to be some kind of reward, as an incentive not to use the full available time every day).
The time contingent is reset in the morning (default: at 6:00 h). The LCD backlight switch on at 8:00 h and off at 22:00 h automatically, this can be configured in the code. The program produces a few audio signals when the speaker is connected, but this is kept to a minimum. For example, it will beep 5 minutes before the "chance time" (60 minutes left) and when the time contingent is almost used up.

## Known issues

It could be tempting to power the Down-Timer and the switched device with the same 5V power supply when applicable, but by experience I recommend against it. In particular the tiny Ethernet switch which was connected to the relay in this project proved to be a rather "dirty" electrical device and caused unpredictable interference with the Arduino. So better keep the circuits separate.

The DS3231 modules sometimes come with onboard circuitry to charge the backup battery. If not using a rechargeable battery this must be deactivated, e.g. by destroying a tiny diode next to the battery holder as explained elsewhere. Note that this project uses the EEPROM (typically an AT24C32 chip) which is present on many DS3231-based RTC modules. If using a module without the EEPROM you would have to disable that functionality (jumper on Arduino pin 3), but then it cannot remember the countdown time if power was removed.


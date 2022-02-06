# DownTimer
<b>DownTimer - Arduino-controlled timer switch with LCD and RTC</b>

This project is about an Arduino-controlled timer switch to control the usage of an electrical device. It has a daily time contingent which is reset every morning. The one-button operation allows to start/stop the device, and the time already used is remembered also in case it would be unplugged. This is achieved by using the EEPROM which is present on the DS3231-type realtime clock module. Information about the current state is displayed on the LCD.

The specific use case for which this project was designed, is to control the network-connected time on a gaming PC. When the daily contigent is used up, it switches off the network connection by cutting off the power from a tiny Ethernet switch. Everything is assembled together in a compact housing, so that the Ethernet and pwer cables are not easily unplugged to circumvent the down-time. <b>Note:</b> this thing is meant as a help for self-control of network time spent on the gaming PC, not as a stringent method to block it. The default time contingent is 240 minutes but this can be easily configured in the code. There are some additional features: a "reserve time" mode where an extra 15 minutes can be activated, but these will be deduced from the next-day contingent. The "chance time" feature is an option to spare more than 60 minutes of the daily allowed time; when this done two days in a row, it will allow the additional time on the third day, plus a small random number of minutes as reward.

<img src=DownTimer_case.jpg width=60%>

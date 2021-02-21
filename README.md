# AlarmClock_arduino

Meant to be run on an Arduino Mega + Learning Shield, the alarm clock has 3 modes(corespounding to the 3 buttons):
- Pressing BTN1 results in showing the alarm time on the SSD instead of the current time(pressing it again will switch back);
- Pressing BTN2 will mute the alarm buzzer(pressing it again will unmute);
- Pressing BTN3 will enter setupMode where the user is able to set the currentTime(only the first time the program enters the setupMode) and the alarmTime(every time other than the first, where the current time is set). In order to set the times, BTN1 and BTN2 are used in order to increase/decrease the minutes and seconds and BTN3 is used in order to submit the time to the Arduino board;

*due to the limitations of the board and shield the alarm clock cannot show times as hours:minutes:seconds and shows it as minutes:seconds(since the shield only has 4 segments for the SSD). 

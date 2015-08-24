# UltrasonicSensor
UltrasonicSensor for presence detector of object in small room

![Image of Shema project UltrasonisSensor](https://github.com/lexxai/UltrasonisSensor/blob/master/shema/shema3.png)

Release versions:
https://github.com/lexxai/UltrasonisSensor/releases

Release v1.1.0:

Used `Sleep()` instead of `__delay` in place where nou use measure of distance.
For WakeUp from sleep state used WatchDog timer.
When not measured distance calculation time is approx. for safety timers.
~18ms of WatchDog base time of RC acceptable for this task.
If want not use sleep function may redefine variable to `#define UseWatchDogForDelay 0` in `user.h`

PS. HEX file need flash to PIC with detached UltraSound Module, because ECHO pin shared with VPP.

If need for debug know value of distance returned form UltraSound Module, connect RX pin of USB RS232 TTL module to GP0. And uncommet `#define DEBUG_UART` at `system.h`. Speed 9600 8N1. RealTerm application will help to show 16 bit value.


https://translate.googleusercontent.com/translate_c?depth=1&hl=uk&ie=UTF8

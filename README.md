# UltrasonisSensor
UltrasonisSensor for presence detector of room

 

![Image of Shema project UltrasonisSensor](https://github.com/lexxai/UltrasonisSensor/blob/master/shema/shema.png)

Shema with charger: https://github.com/lexxai/UltrasonisSensor/releases/download/v1.1.0/shema-charger.png

Release versions:
https://github.com/lexxai/UltrasonisSensor/releases

Release v1.1.0:
Used Sleep() instaed of __delay in place where nou use measure of distance.
For WakeUp from sleep state used WatchDog timer.
When not measured distance calculation time is approx. for safety timers.
~18ms of WatchDog bace time of RC acceptable for this task.
If want not use sleep function may redefine variable to #define UseWatchDogForDelay 0 in user.h

PS. HEX file need flash to PIC with detached UltraSound Module, because ECHO pin shared with VPP.

If need for debug know value of distance returned form UltraSound Module, connect RX pin of USB RS232 TTL module to GP0. And uncommet #define DEBUG_UART at system.h. Speed 9600 8N1. RealTerm application will help to show 16 bit value.

# UltrasonicSensor
UltrasonicSensor для визнаяення наявності об'єктів у малому приміщенні

![Image of Shema project UltrasonisSensor](https://github.com/lexxai/UltrasonisSensor/blob/master/shema/shema.png)

Release versions:
https://github.com/lexxai/UltrasonisSensor/releases

Release v1.1.0:

Використанно механізм сну `Sleep()` замість звичайної затримки  `__delay` там де не потрібно вимірювати відстань до об'єкту.
Для просинання зі "сну" викристанно WatchDog таймер. Так як у режимі не вимірювання відстанні не потрібнна велика точність часу,
~18ms стандартної затримки WatchDog основаної на RC прийняттно для данної задачі.
Якщо не портібно використовуати мехнаізм "сну" у программі требе визанчити визнаячееня для компілятора `#define UseWatchDogForDelay 0` у `user.h`.

PS. Прошивку HEX файлу портібно записувати до контролерра PIC з відключенним модулем UltraSound, томущо ECHO pin сумісно використовується з VPP.

Для діагностики якщо Вам потрібно знати відстань у см (uint16), що повертає  UltraSound модуль, то можна використовувати USB RS232 TTL модуль під'єднавши його RX до GP0 контролерра. А також розкоментувати визначення для компілятора `#define DEBUG_UART` в `system.h`. Швидкість 9600 8N1. За допомогою програми [RealTerm](http://realterm.sourceforge.net) можна ефективно переглядати результати 16 бітних значень.

[Translate to English](https://translate.google.com/translate?depth=1&hl=uk&ie=UTF8&prev=_t&rurl=translate.google.com.ua&sl=uk&tl=en&u=https://github.com/lexxai/UltrasonicSensor)

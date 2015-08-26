# UltrasonicSensor
UltrasonicSensor для визнаяення наявності об'єктів у малому приміщенні

![Image of Shema project UltrasonisSensor](https://github.com/lexxai/UltrasonisSensor/blob/master/shema/shema.png)

Release versions:
https://github.com/lexxai/UltrasonisSensor/releases

Release v1.1.0:

У даній версії там де не потрібно вимірювати відстань до об'єкту використано механізм сну - `Sleep()` замість звичайної затримки - `__delay`.  Для виходу  зі "сну" використано таймер - WatchDog. Так як у режимі не вимірювання відстані не потрібна велика точність часу, то стандартної затримки WatchDog основаної на RC (~18ms) є прийнятною для даної задачі. Якщо не потрібно використовувати механізм "сну" у програмі треба визначити визначення  для компілятора  `#define UseWatchDogForDelay 0` у `user.h`.

PS. Прошивку HEX файлу потрібно записувати до контролера PIC від’єднавши модуль UltraSound, тому що вхід ECHO сумісно використовується з VPP.

Для діагностики, якщо Вам потрібно знати відстань у см (uint16), що повертає UltraSound модуль, то можна використовувати зовнішній модуль USB RS232 TTL, під'єднавши його вхід RX RS232 до виходу контролера GP0. А також роз коментувати визначення для компілятора  `#define DEBUG_UART` в `system.h`. Швидкість з’єднання 9600 8N1. За допомогою програми [RealTerm](http://realterm.sourceforge.net) можна ефективно переглядати результати 16 бітних значень.

[Translate to English](https://translate.google.com/translate?depth=1&hl=uk&ie=UTF8&prev=_t&rurl=translate.google.com.ua&sl=uk&tl=en&u=https://github.com/lexxai/UltrasonicSensor)

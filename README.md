## [Blink с прерываниями от mik32-amur/platform_mik32_amur](https://gitflic.ru/project/mik32-amur/platform_mik32_amur)

### Изменения

Имеются следующие исправления относительно оригинальной версии распространяемой через репозиторий:

1. Добавлен Makefile, вся сборка выполняется командой ```make``` и не зависит.

2. Оставлена поддержка платы NUKE MIK32 V0.3 - выполнена конфигурация соответствующих пинов GPIO.

3. Прошивка осуществляется командой ```make upload```. 

4. Добавлен launch.json для запуска отладки из vscode
# ArduinoPHPi2C
PHP and Arduino tools for I2C EEPROM 24C02, 24C04, 24C08, 24C16, 24C32, 24C64, 24C65, 24C128, 24C256, 24C512, 24C1024.

This is a simple tool to reand and write the i2c EEPROM via Arduino and PHP.
Requires:
* PHP5-cli.
* Arduino board.
* Linux system.

Upload to your Arduino boadr the program at onArduino folder.
Use by terminal the commands in onPHP folder.

./initi2c /dev/ttyUSB0
./rdi2c ...
./wri2c ...

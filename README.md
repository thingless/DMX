You also need https://github.com/lowlander/dmx_usb_module.git as a Linux driver.
Don't forget to `rmmod ftdi_sio`
Build with `gcc -g -o driver -lpthread driver.c`
Example program:
BEGIN
10 0 50 0 0
10 0 0 50 0
10 0 0 0 50
10 0 0 50 0
10 0 50 0 0
10 0 25 25 25
10 0 0 0 0
END

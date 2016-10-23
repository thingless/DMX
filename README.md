Build
-----
Build and install DMX kernel drivers with 'make install_driver'

Build with 'make'

See 'examples' directory for animations.
Run random examples with 'make visualizer'

Example Hardware
----------------
* DMX LED driver (24 channels)
* DMX to USB board

Note: for our 5-pin DMX cable, we use 3 pins:

* D+ = black
* D- = white
* GND = silver/bare wire

Language format
---------------
2 numbers plus channel values (0-255)
```
T 0 A B C D E F G H
T 0 A B C D E F G H
T 0 A B C D E F G H
...
END
```
```
T = time in "frames" to send this command
0 = literal 0, the command to send to DMX, must be at start of every line
A, B, C... = channel letters, replace these with brightness values from 0-255
```


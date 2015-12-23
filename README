Introduction
============
This is a small library for small devices, like ATMega32, etc...
Most public header files can be found in the 'src/' folder.
This library uses 'cmake' build system, the corresponding
configuration files can be found in sub-directories of the
'targets/' folder.

Build instructions
==================
Here is how we can build for emulator:
$ mkdir build
$ cd build
$ cmake ../targets/pc/
$ make

For AVR, configuration can be done line this:
$ mkdir build
$ cd build
$ cmake ../targets/avr/
$ make

Flashing devices:
$ sudo avrdude -p m32 -c jtagmkI -P /dev/ttyUSB0 -n
More info can be found in the README file for a particular target.

Console: escape sequences support
=================================
Console supports the following escape sequences:
 1) "\033[[value-1[;value-2[...;value-n]]]m" - Set text/background colors. Supported values:
    - [0] - reset all attributes to default values;
    - [30..37] - text font color;
    - [40..47] - background color;
 2) "\033[<line>;<column>H" - Set cursor position to line/column position;
 3) "\033[<line>;<column>f" - Set cursor position to line/column position;
 4) "\033[2J" - Clear screen, reset cursor positions to the default position (0, 0);
 5) "\033[<lines>A" - Move cursor up <lines> number of lines;
 6) "\033[<lines>B" - Move cursor down <lines> number of lines;
 7) "\033[<cols>C" - Move cursor forward <cols> number of columns;
 8) "\033[<cols>D" - Move cursor backward <cols> number of columns;
 9) "\033[s" - Save the current cursor position;
10) "\033[u" - Restore the current cursor position;
11) "\033[K" - Erase line: clear the current line starting the current cursor position;

Here is the link where some escape sequesnces are listed: http://ascii-table.com/ansi-escape-sequences.php

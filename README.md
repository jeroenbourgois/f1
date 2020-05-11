# F1

Produces a full F1 startgrid sequence.

Developed for the LSST Data Management System.
This product includes software developed by the LSST Project
(https://www.lsst.org).
See the COPYRIGHT file at the top-level directory of this distribution
for details of code ownership.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

All code is available on [Github](https://github.com/jeroenbourgois/f1)

Circuit:
- 8 ohm speaker on digital pin 2
- 5 LEDs connected to digital pins 9 -> 13,
  each LED grounded with a 220Ω resistor
- 'OK' pushbutton on analog pin A3 (used as digital pin) and +5V
- 'CANCEL' pushbutton on analog pin A4 (used as digital pin) and +5V
- two reed switches to measure passing cars, analog pins A1 and A2 and +5v
- LCD:
   - data pins D4 -> D7 wired to digital pins 7 -> 4
   - E pin wired to digital pin 8
   - RS pin wired to digital pin 3
   - VCC+ and
   - a potentionmeter with it's middle leg connected to the LCD 4

A detailed Fritzing sketch is available in the repository.

On the LCD we have the following screens:

START -> [Y] -> COUNTDOWN -> RACE --> [Y] -> QUIT? -> [Y] -> START
                                  |                -> [N] -> RACE
                                  --> [N] -> n/a

Created:       Apr 1, 2020
Last Modified: May 11, 2020
By Jeroen Bourgois.

This code is in the public domain.

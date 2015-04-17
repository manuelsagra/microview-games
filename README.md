# MicroView Games

These are some simple games for the [MicroView](http://learn.microview.io/) device, a little Arduino Uno compatible board with a built-in 64x48 pixel OLED display.

## MicroBird

It's a mix between Flapy Bird and that old Copter game. I think the code is self explanatory, and you only have to put a push button between a data pin (A0 / pin 7 by default) and GND (pin 8) to play.

[![MicroBird Video](http://img.youtube.com/vi/HBmVei5h2kM/0.jpg)](http://www.youtube.com/watch?v=HBmVei5h2kM)

The other button shown on the video is just to reset the MicroView.

## MicroPong

As the name suggests, it's a Pong game. It's for two players only, and I've used the original Atari CX30 Paddle Controllers. The wiring is really simple, as each controller only has a push button and a 1MΩ potentiometer.

[![MicroPong Video](http://img.youtube.com/vi/hz0Fo0Yh_2E/0.jpg)](http://www.youtube.com/watch?v=hz0Fo0Yh_2E)

This is how I wired the controllers and the MicroView:

| Signal    | DB-9 Connector Pin | MicroView Pin |
|-----------|--------------------|---------------|
| GND       | 8                  | 8             |
| Vcc       | 7                  | 15            |
| P1 Button | 3                  | 7             |
| P2 Button | 4                  | 6             |
| P1 Paddle | 9                  | 5             |
| P2 Paddle | 5                  | 4             |

As the potentiometer is not wired internally to GND, you have to put two 1MΩ resistors between pins 4 & 5 from the MicroView and pin 8.

## MicroBreakOut

Using some code from the previous games I made a BreakOut clone. Do you know a system without one?

[![MicroBreakOut Video](http://img.youtube.com/vi/holrhtSdw-4/0.jpg)](http://www.youtube.com/watch?v=holrhtSdw-4)

The wiring is the same as before, but only Player 1 inputs are used.

# Handheld Rocket Game 🚀

A small handheld Arduino game where you pilot a rocket through space and shoot school subjects disguised as meteors.

The game uses simple shooter mechanics, physical buttons, a TFT display, and a buzzer for sound effects. Your goal is simple: survive, shoot the subjects you don't like, and get the highest score you can. :)

## Main Parts

| Quantity | Part |
|---:|---|
| 1x | Arduino Nano |
| 1x | 1.8" 128×160 TFT Display (ST7735) |
| 1x | Buzzer |
| 3x | Push Buttons |
| Several | Jumper Wires |
| 1x | Breadboard / Perfboard |

> Other displays can also be used, but you may need to change the resolution, pin configuration, and display library in the code.

## Pinout

The exact pinout may vary depending on the display you use.

For my build, the connections are:

| Component | Arduino Pin |
|---|---|
| GND | GND |
| VDD / BLK | 5V |
| SCL | D13 |
| SDA | D11 |
| RST | D9 |
| DC | D8 |
| CS | D10 |
| Button UP | D2 |
| Button DOWN | D3 |
| Button FIRE | D4 |
| Buzzer | D7 |

## Controls

| Button | Action |
|---|---|
| UP | Move the rocket up |
| DOWN | Move the rocket down |
| FIRE | Shoot |

## Display

The project was designed around a **128×160 ST7735 TFT display**.

If you use a different TFT, LCD, or OLED display, you will probably need to modify:

- Display library
- Screen resolution
- Pin definitions
- Drawing coordinates in the code

## Build

The circuit can first be assembled on a breadboard for testing before being moved into a more permanent handheld enclosure.

## Wiring Schematic

The following schematic shows the wiring between the Arduino Nano, ST7735 display, control buttons, and buzzer.

<img width="830" height="879" alt="Screenshot 2026-10-01 230515" src="https://github.com/user-attachments/assets/d74b3abc-c221-44f5-bf0a-ebf2f803d59f" />

## Project Photo

<img
  src="https://github.com/user-attachments/assets/d9e642f4-5f73-4d05-a670-ac59f2fe43c4"
  alt="Handheld Rocket Game"
  width="600"
/>

---

Make sure to watch the video in YouTube! https://www.youtube.com/watch?v=62NKlN6i7YI<br><br><br>
Hope you enjoy the project! 🚀


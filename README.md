# avr-study-projects
Some projects on AVR for study purpose

## Prerequisite
- avrdude v6.1
- avr-gcc v4.8.1

## Getting started
```bash
# go into any project folder
cd ./any-project-folder

# compile the project
make all

# flash the chip, ensure the avr settings in Makefile
make flash

# clean the project
make clean
```

## Projects

### Push Button
- Simple push button project

### Digital Clock
- avr digital clock based on interrupt
- keypad integration
- compile with [avr-datetime](https://github.com/peteroid/avr-datetime)

### Simple Music Player
- simple music player based on interrupt, PWM
- volumn, tempo control
- LCD character customization

### Voltage Meter
- voltmeter based on the built-in ADC
- range from 0v to 5v
- don't test over 5v
- show the current, average, maximum and minimum

### Wireless Speedometer (self-propose)
- interfaced the bluetooth module with onboard usart
- maganetic sensor
- great for mobile app integration
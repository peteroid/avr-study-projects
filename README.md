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
- simple music player based on interrupt generating PWM
- volumn, tempo control
- LCD character customization

### Voltage Meter (todo)
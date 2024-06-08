STM32F103C8 (bluepill) with libopencm3 controlling a ssd1306 display (128x64). 
The program simulates Conway's game of life. with "Acorn" pattern 


## Install/compile
You can use just make without bear.
```bash
$ sudo apt-get install stlink-tools bear gcc-arm-none-eabi
$ git clone --recurse-submodules https://github.com/bonmas14/stm32-ssd1306-GOL.git
$ cd stm32-ssd*/libopencm3
$ make
$ cd ../ssd*
$ bear -- make
$ st-flash --reset write ssd1306_GOL.bin 0x8000000
```

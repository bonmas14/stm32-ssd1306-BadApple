STM32F103C8 (bluepill) with libopencm3 controlling a ssd1306 display (128x64) to show some BadApple. 

## Warning 
This works with fake stm32f103c8 from aliexpress, they are 128kb instead of 64. But this will work with stm32f103cB

## Install/compile
You can use just make without bear.
```bash
$ sudo apt-get install stlink-tools bear gcc-arm-none-eabi
$ git clone --recurse-submodules https://github.com/bonmas14/stm32-ssd1306-BadApple.git
$ cd stm32-ssd*/libopencm3
$ make
$ cd ../ssd*
$ bear -- make
$ st-flash --reset write badapple.bin 0x8000000
```

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/i2c.h>

#include "gol.h"

#include <stdint.h>

#define ssd1306addr 0x3c
#define ssd1306_width 128
#define ssd1306_height 64
#define ssd1306_buffer_size (ssd1306_width * 8)

static uint8_t ssd1306_buffer[ssd1306_buffer_size];

static void clock_init(void) {
    rcc_clock_setup_in_hse_8mhz_out_72mhz();
}

static void gpio_init(void) {
    rcc_periph_clock_enable(RCC_GPIOC);

	gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_2_MHZ, 
            GPIO_CNF_OUTPUT_PUSHPULL, 
            GPIO13);
}

static void wait(uint32_t mul)
{
    for (uint32_t i = 0; i < (1000000 * mul); i++) {
        __asm__("nop");
    }
}

static void i2c_init(void) {
    rcc_periph_clock_enable(RCC_GPIOB);
    rcc_periph_clock_enable(RCC_I2C1);
    rcc_periph_clock_enable(RCC_AFIO);

    gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ,
            GPIO_CNF_OUTPUT_ALTFN_OPENDRAIN, 
            GPIO_I2C1_RE_SCL | GPIO_I2C1_RE_SDA);
    
    gpio_primary_remap(0, AFIO_MAPR_I2C1_REMAP);

    i2c_peripheral_disable(I2C1);

    i2c_set_speed(I2C1, i2c_speed_fm_400k, rcc_apb1_frequency / 1000000);

    i2c_peripheral_enable(I2C1);
}

enum ssd1306_ctlbyte {
    LongCMD = 0b00000000,
    LongDAT = 0b01000000,
    ShortCMD = 0b10000000,
    ShortDAT = 0b11000000,
};

static void ssd1306_send_page(size_t page)
{
    while ((I2C_SR2(I2C1) & I2C_SR2_BUSY)) { }

    i2c_send_start(I2C1);

    while ( !( (I2C_SR1(I2C1) & I2C_SR1_SB)
            && (I2C_SR2(I2C1) & I2C_SR2_MSL)
            && (I2C_SR2(I2C1) & I2C_SR2_BUSY) ));

    i2c_send_7bit_address(I2C1, ssd1306addr, I2C_WRITE);
    while (!(I2C_SR1(I2C1) & I2C_SR1_ADDR));
    (void)I2C_SR2(I2C1);

    i2c_send_data(I2C1, LongDAT);

    for (size_t i = 0; i < ssd1306_width; i++) {
        i2c_send_data(I2C1, ssd1306_buffer[i + page * ssd1306_width]);
        while (!(I2C_SR1(I2C1) & (I2C_SR1_BTF)));
    }

    i2c_send_stop(I2C1);

}

static void ssd1306_update(void) {

    i2c_transfer7(I2C1, ssd1306addr, (uint8_t[]){ LongCMD, 0xD3, 0x00 }, 3, NULL, 0);
    for (uint8_t page = 0; page < 8; page++) {
        i2c_transfer7(I2C1, ssd1306addr, (uint8_t[]){ ShortCMD, 0xB0 + page }, 2, NULL, 0);
        i2c_transfer7(I2C1, ssd1306addr, (uint8_t[]){ ShortCMD, 0x00 }, 2, NULL, 0);
        i2c_transfer7(I2C1, ssd1306addr, (uint8_t[]){ ShortCMD, 0x10 }, 2, NULL, 0);
        ssd1306_send_page(page);
    }
}

static void ssd1306_clear(void) {
    for (size_t i = 0; i < ssd1306_buffer_size; i++) {
        ssd1306_buffer[i] = 0;
    
    }
}

static void ssd1306_init(void) {
    i2c_transfer7(I2C1, ssd1306addr, (uint8_t[]){ ShortCMD, 0xAE }, 2, NULL, 0);
    i2c_transfer7(I2C1, ssd1306addr, (uint8_t[]){ LongCMD, 0x20, 0x02 }, 3, NULL, 0);
    i2c_transfer7(I2C1, ssd1306addr, (uint8_t[]){ ShortCMD, 0xB0 }, 2, NULL, 0);

    i2c_transfer7(I2C1, ssd1306addr, (uint8_t[]){ LongCMD, 0xA8, 0x3F }, 3, NULL, 0);
    i2c_transfer7(I2C1, ssd1306addr, (uint8_t[]){ LongCMD, 0xD3, 0x00 }, 3, NULL, 0);
    i2c_transfer7(I2C1, ssd1306addr, (uint8_t[]){ ShortCMD, 0x40, ShortCMD, 0xA0, ShortCMD, 0xC0 }, 8, NULL, 0);
    i2c_transfer7(I2C1, ssd1306addr, (uint8_t[]){ LongCMD, 0xDA, 0x12 }, 3, NULL, 0);
    i2c_transfer7(I2C1, ssd1306addr, (uint8_t[]){ LongCMD, 0x81, 0x7F }, 3, NULL, 0);
    i2c_transfer7(I2C1, ssd1306addr, (uint8_t[]){ ShortCMD, 0xA4, ShortCMD, 0xA6 }, 4, NULL, 0);
    i2c_transfer7(I2C1, ssd1306addr, (uint8_t[]){ LongCMD, 0xD5, 0x80 }, 3, NULL, 0);

    i2c_transfer7(I2C1, ssd1306addr, (uint8_t[]){ LongCMD, 0xDB, 0x20 }, 3, NULL, 0);
    i2c_transfer7(I2C1, ssd1306addr, (uint8_t[]){ LongCMD, 0xD9, 0x22 }, 3, NULL, 0);
    i2c_transfer7(I2C1, ssd1306addr, (uint8_t[]){ LongCMD, 0x8D, 0x14 }, 3, NULL, 0);
    i2c_transfer7(I2C1, ssd1306addr, (uint8_t[]){ ShortCMD, 0xAF }, 2, NULL, 0);

    ssd1306_update();
}

static void ssd1306_compact_write_gol(GOL_t* state) {
    size_t array_size = state->width * state->height;
    uint8_t* map = state->map;
    
    ssd1306_clear();

    for (size_t i = 0; i < array_size; i++) {
        size_t x = i % state->width;
        size_t y = i / state->width;

        uint8_t bit = map[x + y * state->width] << (y % 8);
        uint8_t page = y / 8;

        ssd1306_buffer[x + page * ssd1306_width] |= bit;
    }
}

int main(void) {
    clock_init();
    gpio_init();
    i2c_init();

    gpio_set(GPIOC, GPIO13);
    wait(10);
    ssd1306_init();
    gpio_clear(GPIOC, GPIO13);
    ssd1306_update();

    GOL_t* state = gol_init(ssd1306_width, ssd1306_height);

    state->map[1 + ssd1306_width / 2 + ssd1306_width * (ssd1306_height / 2 + 1)] = 1;
    state->map[3 + ssd1306_width / 2 + ssd1306_width * (ssd1306_height / 2 + 2)] = 1;
    state->map[0 + ssd1306_width / 2 + ssd1306_width * (ssd1306_height / 2 + 3)] = 1;
    state->map[1 + ssd1306_width / 2 + ssd1306_width * (ssd1306_height / 2 + 3)] = 1;
    state->map[4 + ssd1306_width / 2 + ssd1306_width * (ssd1306_height / 2 + 3)] = 1;
    state->map[5 + ssd1306_width / 2 + ssd1306_width * (ssd1306_height / 2 + 3)] = 1;
    state->map[6 + ssd1306_width / 2 + ssd1306_width * (ssd1306_height / 2 + 3)] = 1;

	while(1) {
        ssd1306_compact_write_gol(state);
        ssd1306_update();
        gol_update(state);
        gpio_toggle(GPIOC, GPIO13);
	}
}

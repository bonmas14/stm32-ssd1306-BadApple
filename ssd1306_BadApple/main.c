#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/i2c.h>
#include <libopencm3/cm3/nvic.h>

#include "apple.h"

#include <stdint.h>

#define ssd1306addr 0x3c
#define ssd1306_width 128
#define ssd1306_height 64
#define ssd1306_buffer_size (ssd1306_width * 8)

static uint8_t ssd1306_buffer[ssd1306_buffer_size];

static void clock_init(void) {
    rcc_clock_setup_pll(&rcc_hse_configs[RCC_CLOCK_HSE8_72MHZ]);
    
    rcc_periph_clock_enable(RCC_TIM2);

    nvic_enable_irq(NVIC_TIM2_IRQ);
    nvic_set_priority(NVIC_TIM2_IRQ, 1);
}

static void gpio_init(void) {
    rcc_periph_clock_enable(RCC_GPIOC);

	gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_2_MHZ, 
            GPIO_CNF_OUTPUT_PUSHPULL, 
            GPIO13);
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

static size_t ssd1306_write_rle(size_t start) {
    ssd1306_clear();

    size_t counter = 0;

    size_t pixel_width = ssd1306_width / APPLE_WIDTH;

    bool color = 0;

    size_t i = 0;

    while (i < ssd1306_buffer_size) {
        counter = frames[start++];

        while (counter > 0) {
            for (size_t pix = 0; pix < pixel_width; pix++) {
                ssd1306_buffer[i] = color ? 0xFF : 0x00;
                i++;
            }

            counter--;
        }

        color = !color;
    }

    return start;
}

size_t offset = 0;
size_t frame = 1;

int main(void) {
    clock_init();
    gpio_init();
    i2c_init();

    TIM2_CNT = 1;
    TIM2_PSC = 2400; // 2400 == 30000 fps

    TIM2_ARR = 1000; // 30000 / 1000 = 30 fps
    TIM2_DIER |= TIM_DIER_UIE;


    ssd1306_init();
    ssd1306_update();

    TIM2_CR1 |= TIM_CR1_CEN;

	while(1) {
	}
}

void tim2_isr(void) {
    gpio_toggle(GPIOC, GPIO13);

    if (frame >= 6572) {
        offset = 0;
    }

    offset = ssd1306_write_rle(offset);
    ssd1306_update();
    frame++;

    TIM2_SR &= ~TIM_SR_UIF;
}

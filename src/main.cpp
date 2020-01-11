#include <gpio.h>
#include <spi.h>

using hal::sys_tick;
using hal::sys_clock;
using namespace hal::gpio;
using namespace hal::spi;

typedef output_t<PB8> led;
typedef output_t<PB4> cs1;
typedef output_t<PB5> ldac1;
typedef spi_t<1, PA5, PA7> spi;

void loop();

int main()
{
    spi::setup<mode_0, msb_first, fpclk_256, low_speed>();
    cs1::setup();
    ldac1::setup();
    led::setup();
    cs1::set();
    ldac1::set();
    for (;;)
        loop();
}


static void write_mcp4822(uint16_t x)
{
    static constexpr uint8_t ch_a = 0;
    //static constexpr uint8_t ch_b = 0x80;
    static constexpr uint8_t gain = 0x20;
    static constexpr uint8_t enable = 0x10;

    cs1::clear();
    sys_clock::delay_us(1);
    spi::write8(ch_a | gain | enable | ((x >> 8) & 0x0fff));
    spi::write8(x);
    spi::wait_idle();
    sys_clock::delay_us(1);
    cs1::set();
    sys_clock::delay_us(1);
    ldac1::clear();
    sys_clock::delay_us(1);
    ldac1::set();
}

void loop()
{
    static uint16_t i = 0;

    led::toggle();
    write_mcp4822(7);
    i++;
    sys_tick::delay_ms(1);
}


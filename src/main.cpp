#include "mcp48x2.h"

using hal::sys_tick;
using hal::sys_clock;
using namespace hal::gpio;
using namespace hal::spi;
using namespace mcp48x2;

typedef output_t<PB8> led;
typedef spi_t<1, PA5, PA7> spi;
typedef mcp48x2_t<spi, PB4, PB5> dac1;
typedef mcp48x2_t<spi, PA11, PA8> dac2;

void loop();

int main()
{
    spi::setup<mode_0, msb_first, fpclk_64, low_speed>();
    dac1::setup();
    dac2::setup();
    for (;;)
        loop();
}

void loop()
{
    static uint16_t i = 0;

    led::toggle();
    dac1::write<A>(1000);
    dac1::write<B>(2000);
    dac2::write<A>(3000);
    dac2::write<B>(4000);
    i++;
    sys_tick::delay_ms(1);
}


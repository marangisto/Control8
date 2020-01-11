#include <stdlib.h>
#include "mcp48x2.h"

using hal::sys_tick;
using hal::sys_clock;
using namespace hal::gpio;
using namespace hal::spi;
using namespace mcp48x2;

typedef output_t<PB8> led;
typedef output_t<PA6> trg1;
typedef output_t<PA4> trg2;
typedef output_t<PA1> trg3;
typedef output_t<PA0> trg4;
typedef spi_t<1, PA5, PA7> spi;
typedef mcp48x2_t<spi, PB4, PB5> dac1;
typedef mcp48x2_t<spi, PA11, PA8> dac2;

void loop();

int main()
{
    spi::setup<mode_0, msb_first, fpclk_64, low_speed>();
    dac1::setup();
    dac2::setup();
    trg1::setup();
    trg2::setup();
    trg3::setup();
    trg4::setup();
    for (;;)
        loop();
}

void loop()
{
    static uint16_t i = 0;

    led::toggle();
    dac1::write<A>(rand());
    dac1::write<B>(rand());
    dac2::write<A>(4095-i);
    dac2::write<B>(rand());
    i++;
    if (i > 4095)
        i = 0;
    trg1::write(i & (1 << 9));
    trg2::write(i & (1 << 8));
    trg3::write(i & (1 << 7));
    trg4::write(i & (1 << 6));
    sys_tick::delay_ms(1);
}


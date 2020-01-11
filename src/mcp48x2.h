#pragma once

#include <gpio.h>
#include <spi.h>

namespace mcp48x2
{

enum channel_t { A, B };

template<channel_t> struct channel_traits {};
template<> struct channel_traits<A> { static constexpr uint8_t ch = 0x00; };
template<> struct channel_traits<B> { static constexpr uint8_t ch = 0x80; };

template<typename SPI, hal::gpio::gpio_pin_t CS, hal::gpio::gpio_pin_t LDAC>
struct mcp48x2_t
{
    typedef hal::gpio::output_t<CS> cs;
    typedef hal::gpio::output_t<LDAC> ldac;

    static void setup()
    {
        cs::setup();
        cs::set();
        ldac::setup();
        ldac::set();
    }

    template<channel_t CH>
    static void write(uint16_t x)
    {
        static constexpr uint8_t gain = 0x20;
        static constexpr uint8_t enable = 0x10;
        static constexpr uint8_t ctrl = channel_traits<CH>::ch | gain | enable;

        cs::clear();
        SPI::write8(ctrl | (x >> 8));
        SPI::write8(x);
        SPI::wait_idle();
        cs::set();
        ldac::clear();
        ldac::set();
    }
};

} // namecpace 48x2


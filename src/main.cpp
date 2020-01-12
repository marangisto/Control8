//#include <cstring>
#include <stdlib.h>
#include <usart.h>
#include <redirect.h>
#include "mcp48x2.h"

using hal::sys_tick;
using hal::sys_clock;
using namespace hal::usart;
using namespace hal::gpio;
using namespace hal::spi;
using namespace mcp48x2;

typedef output_t<PB8> led;
typedef output_t<PA6> trg1;
typedef output_t<PA4> trg2;
typedef output_t<PA1> trg3;
typedef output_t<PA0> trg4;
typedef spi_t<1, PA5, PA7> spi;
typedef mcp48x2_t<spi, PB4, PA12> dac1;
typedef mcp48x2_t<spi, PA11, PA8> dac2;
typedef usart_t<2, PA2, PA3> serial;

template<> void handler<interrupt::USART2>()
{
    led::toggle();
    serial::isr();
}

static uint16_t midi2dac(uint8_t ch, uint8_t note)
{
    constexpr float a[] = { -408.9450464, -409.3488986, -410.1605246, -408.7555259 };
    constexpr float b[] = { 2612.798547,  2600.621013,  2612.654322,  2606.178379 };
    constexpr float inv12 = 1.0f / 12.0f;
    float cv = inv12 * (static_cast<float>(note) - 69.0f);
    uint16_t x = static_cast<uint16_t>(a[ch] * cv + b[ch]);

    //printf("%d, %d\n", note, x);
    return x;
}

enum midi_tag_t
    { note_off      = 0x80
    , note_on       = 0x90
    , calibrate     = 0xf4      // reserved & undefined in midi
    };

struct midi_message_t
{
    midi_tag_t  tag;
    uint8_t     channel;
    uint16_t    data1;      // should stricly be 8-bit but allow more for extended use
    uint8_t     data2;
};

static bool parse_midi_message(const char *s, midi_message_t& m)
{
    int cmd, d1, d2, n = sscanf(s, "%x %d %d", &cmd, &d1, &d2);
    //printf("n = %d, cmd = %d, d1 = %d, d2 = %d\n", n, cmd, d1, d2);

    if (n == 0)
        return false;

    m.channel = cmd & 0x0f;
    m.data1 = d1;
    m.data2 = d2;

    switch (m.tag = static_cast<midi_tag_t>(cmd & 0xf0))
    {
        case note_off: return n == 3;
        case note_on: return n == 3;
        default:
            switch (m.tag = static_cast<midi_tag_t>(cmd))
            {
            case calibrate: m.channel = d2; return n == 3;
            default: return false;
            }
    }
}

static void show_midi_message(const midi_message_t& m)
{
    switch (m.tag)
    {
        case note_off: printf("note-off[%d]: %d %d\n", m.channel, m.data1, m.data2); break;
        case note_on: printf("note-on[%d]: %d %d\n", m.channel, m.data1, m.data2); break;
        case calibrate: printf("calibrate[%d]: %d\n", m.channel, m.data1); break;
        default: printf("unrecognized message tag: %d\n", m.tag);
    }
}

static void interpret_midi_message(const midi_message_t& m)
{
    switch (m.tag)
    {
    case note_off:
        switch (m.channel)
        {
        case 1: trg1::set(); break;
        case 2: trg2::set(); break;
        case 3: trg3::set(); break;
        case 4: trg4::set(); break;
        default: ;
        }
        break;
    case note_on:
        switch (m.channel)
        {
        case 1: dac1::write<A>(midi2dac(0, m.data1)); trg1::clear(); break;
        case 2: dac1::write<B>(midi2dac(1, m.data1)); trg2::clear(); break;
        case 3: dac2::write<A>(midi2dac(2, m.data1)); trg3::clear(); break;
        case 4: dac2::write<B>(midi2dac(3, m.data1)); trg4::clear(); break;
        default: ;
        }
        break;
    case calibrate:
        switch (m.channel)
        {
        case 1: dac1::write<A>(m.data1); trg1::toggle(); break;
        case 2: dac1::write<B>(m.data1); trg2::toggle(); break;
        case 3: dac2::write<A>(m.data1); trg3::toggle(); break;
        case 4: dac2::write<B>(m.data1); trg4::toggle(); break;
        default: ;
        }
        break;
    default: ;
    }
}

static char buf[1024];

int main()
{
    serial::setup<115200>();
    hal::nvic<interrupt::USART2>::enable();
    interrupt::enable();

    stdio_t::bind<serial>();
    printf("Welcome to the STM32G431!\n");

    spi::setup<mode_0, msb_first, fpclk_64, low_speed>();
    dac1::setup();
    dac2::setup();
    trg1::setup();
    trg1::set();
    trg2::setup();
    trg2::set();
    trg3::setup();
    trg3::set();
    trg4::setup();
    trg4::set();

    dac1::write<A>(midi2dac(0, 69));
    dac1::write<B>(midi2dac(1, 69));
    dac2::write<A>(midi2dac(2, 69));
    dac2::write<B>(midi2dac(3, 69));

    for (;;)
    {
        midi_message_t m;

        //printf("> \n");
        if (fgets(buf, sizeof(buf), stdin) && parse_midi_message(buf, m))
        {
            show_midi_message(m);
            interpret_midi_message(m);
            led::toggle();
        }
    }
}


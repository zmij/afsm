/*
 * ansi_colors.cpp
 *
 *  Created on: Jun 2, 2016
 *      Author: zmij
 */

#include <pushkin/ansi_colors.hpp>
#include <iostream>

namespace psst {

namespace {

int const multiply_de_bruijn_bit_position[32] =
{
  0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8,
  31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
};

int
lowest_bit_set(unsigned int v)
{
    return multiply_de_bruijn_bit_position[((unsigned int)((v & -v) * 0x077CB531U)) >> 27];
}

char ESC = '\033';

}  /* namespace  */

int
operator & (ansi_color a, ansi_color b)
{
    return static_cast<int>(a) & static_cast<int>(b);
}

::std::ostream&
operator << (::std::ostream& os, ansi_color val)
{
    ::std::ostream::sentry s(os);
    if (s) {
        if (val == ansi_color::clear) {
            os << ESC << "[0m";
        } else {
            if (val & ansi_color::bright) {
                os << ESC << "[1m";
            } else if (val & ansi_color::dim) {
                os << ESC << "[2m";
            }
            if (val & ansi_color::underline) {
                os << ESC << "[4m";
            }
            char fg = '3';
            if (val & ansi_color::backgound) {
                fg = '4';
            }
            // clear attribute bits
            val = static_cast<ansi_color>(val & ansi_color::colors);
            if (val != ansi_color::clear) {
                int color_pos = lowest_bit_set(static_cast<int>(val)) -
                        lowest_bit_set(static_cast<int>(ansi_color::black));
                os << ESC << '[' << fg << (char)('0' + color_pos) << 'm';
            }
        }
    }
    return os;
}


}  /* namespace psst */

/*
 * ansi_colors.hpp
 *
 *  Created on: Jun 2, 2016
 *      Author: zmij
 */

#ifndef PUSHKIN_ANSI_COLORS_HPP_
#define PUSHKIN_ANSI_COLORS_HPP_

#include <iosfwd>

namespace psst {

enum class ansi_color {
    // Clear the color
    clear           = 0x0000,
    // Color attributes
    bright          = 0x0001,
    dim             = 0x0002,
    underline       = 0x0004,

    foreground      = 0x0008,
    backgound       = 0x0010,
    // Normal colors
    black           = 0x0020,
    red             = 0x0040,
    green           = 0x0080,
    yellow          = 0x0100,
    blue            = 0x0200,
    magenta         = 0x0400,
    cyan            = 0x0800,
    white           = 0x1000,
    // Color mask
    colors          = black | red | green | yellow | blue | magenta | cyan | white,
    // Attributes mask
    attributes      = ~colors
};

::std::ostream&
operator << (::std::ostream& os, ansi_color val);

inline ansi_color
operator | (ansi_color a, ansi_color b)
{
    return static_cast<ansi_color>( static_cast<int>(a) | static_cast<int>(b) );
}

}  /* namespace psst */

#endif /* PUSHKIN_ANSI_COLORS_HPP_ */

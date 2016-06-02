/*
 * ansi_colors_example.cpp
 *
 *  Created on: Jun 2, 2016
 *      Author: zmij
 */

#include <iostream>
#include <iomanip>
#include <sstream>
#include <array>
#include <tuple>

#include <pushkin/ansi_colors.hpp>

namespace {

using psst::ansi_color;

int const field_width = 20;

::std::array<::std::string, 8> const color_names {{
    "Black",
    "Red",
    "Green",
    "Yellow",
    "Blue",
    "Magenta",
    "Cyan",
    "White"
}};

struct color_data {
    ansi_color  color;
    ansi_color  dim_complement;
    ansi_color  bright_complement;
};
::std::array< color_data, 8 > const colors {{
    /*  Color               Dim complement      Bright complement*/
    { ansi_color::black,    ansi_color::white,  ansi_color::white },
    { ansi_color::red,      ansi_color::white,  ansi_color::black },
    { ansi_color::green,    ansi_color::white,  ansi_color::black },
    { ansi_color::yellow,   ansi_color::white,  ansi_color::black },
    { ansi_color::blue,     ansi_color::white,  ansi_color::white },
    { ansi_color::magenta,  ansi_color::white,  ansi_color::black },
    { ansi_color::cyan,     ansi_color::white,  ansi_color::black },
    { ansi_color::white,    ansi_color::black,  ansi_color::black },
}};

::std::string
centered(::std::string const& str, int width)
{
    auto lmargin = (width - str.size()) / 2;
    auto rmargin = width - str.size() - lmargin;

    ::std::ostringstream os;
    os << ::std::setw(lmargin) << " " << str << ::std::setw(rmargin) << " ";
    return os.str();
}

void
output_color(::std::string const& name, color_data const& data)
{
    ::std::cout
            << "|"
            << (ansi_color::foreground | data.dim_complement)
            << (ansi_color::backgound | ansi_color::dim | data.color )
            << centered(name, field_width) << ansi_color::clear
            << "|"
            << (ansi_color::foreground | data.bright_complement)
            << (ansi_color::backgound  | ansi_color::bright | data.color)
            << centered(name, field_width) << ansi_color::clear
            << "|"
            << "\n"
    ;
}

}  /* namespace  */

int
main(int argc, char* argv[])
try {
    using psst::ansi_color;

    ::std::cout
            << "|" << centered("Dim", field_width)
            << "|" << centered("Bright", field_width)
            << "|\n";

    for (int i = 0; i < 8; ++i) {
        output_color(color_names[i], colors[i]);
    }

    return 0;
} catch (::std::exception const& e) {
    ::std::cerr << "Exception: " << e.what() << "\n";
    return 1;
} catch (...) {
    ::std::cerr << "Unexpected exception\n";
    return 2;
}

/*
 * vending.cpp
 *
 *  Created on: Nov 15, 2016
 *      Author: zmij
 */

#include <iostream>
#include "vending_machine.hpp"

using vending_machine = vending::vending_machine;

int
main(int, char*[])
try {
    vending_machine vm;
    return 0;
} catch (::std::exception const& e) {
    ::std::cerr << "Exception: " << e.what() << "\n";
    return 1;
} catch (...) {
    ::std::cerr << "Unexpected exception\n";
    return 2;
}

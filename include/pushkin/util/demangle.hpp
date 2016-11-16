/*
 * demangle.hpp
 *
 *  Created on: Jun 23, 2016
 *      Author: zmij
 */

#ifndef PUSHKIN_UTIL_DEMANGLE_HPP_
#define PUSHKIN_UTIL_DEMANGLE_HPP_

#include <cxxabi.h>
#include <string>
#include <iosfwd>

namespace psst {
namespace util {

/**
 * Type name demangle function
 * Usage:
 * @code
 * ::std::cout << demangle< ::std::iostream >() << "\n"
 * @endcode
 * @return Demangled type name
 */
template < typename T >
::std::string
demangle()
{
    int status {0};
    char* ret = abi::__cxa_demangle( typeid(T).name(), nullptr, nullptr, &status );
    ::std::string res{ret};
    if(ret) free(ret);
    return res;
}

/**
 * Type name demangle function, io manip interface. Doesn't create a string.
 * Usage:
 * @code
 * ::std::cout << demangle< ::std::iostream > << "\n"
 * @endcode
 * @param os
 */
template < typename T >
void
demangle(::std::iostream& os)
{
    ::std::ostream::sentry s(os);
    if (s) {
        int status {0};
        char* ret = abi::__cxa_demangle( typeid(T).name(), nullptr, nullptr, &status );
        if (ret) {
            os << ret;
            free(ret);
        }
    }
    return os;
}

}  /* namespace util */
}  /* namespace psst */


#endif /* PUSHKIN_UTIL_DEMANGLE_HPP_ */

/**
 * @file sanitizers.hpp
 * @brief Contains functions that can be used to sanitize the input read from a file into a variable managed by the Settings class.
 *
 * @date 30.03.2025
 * @author Jakob Wandel
 * @version 1.0
 **/

#include <utils/types/range.hpp>

#pragma once

namespace util {

/**
 * @brief Checks if var is inside min max
 *
 * @tparam The variable Type
 * @param var The variable reference which is clamped within min, max
 * @param min The minimal accepted value
 * @param max The maximal accepted value
 **/
template <class T>
void saneMinMax(T& var, util::Range<T> range) {
  var = range.clamp(var);
}
}  // namespace util

#pragma once

namespace util {
template <class T>
void saneMinMax(T& var, T min, T max) {
  if (var > max) {
    var = max;
  } else if (var < min) {
    var = min;
  }
}
}

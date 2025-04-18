#ifndef UUID_UTILS_H
#define UUID_UTILS_H

#include <random>
#include <sstream>
#include <string>

inline std::string generateUUID() {
  std::random_device rd;
  std::uniform_int_distribution<int> dist(0, 15);
  std::stringstream ss;
  ss << std::hex;
  for (int i = 0; i < 32; ++i)
    ss << dist(rd);
  return ss.str();
}

#endif // UUID_UTILS_H

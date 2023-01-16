#ifndef PGM_H
#define PGM_H

#include <string>
#include "mat.h"

enum class pgm_mode { plain = 2, binary = 5 };

bool write(const std::string& filename, const mat<uint8_t>& img, pgm_mode mode);

#endif // PGM_H
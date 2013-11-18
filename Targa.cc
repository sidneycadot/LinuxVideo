
//////////////
// Targa.cc //
//////////////

#include "Targa.h"


Targa::Targa(const uint16_t width, const uint16_t height) :
    v(18 + 3 * height * width)
{
    v[ 2] = 2;

    v[12] = width % 256;
    v[13] = width / 256;

    v[14] = height % 256;
    v[15] = height / 256;

    v[16] = 24;
    v[17] = 32;
}

uint16_t Targa::width() const
{
  return v[12] + 256 * v[13];
}

uint16_t Targa::height() const
{
  return v[14] + 256 * v[15];
}

const std::vector<uint8_t> & Targa::dvec() const
{
    return v;
}

uint8_t * Targa::rgb_data()
{
    return &v[18];
}



/////////////
// targa.h //
/////////////

#ifndef Targa_h
#define Targa_h

#include <vector>
#include <cstdint>

class Targa
{
    public:

        Targa(const uint16_t width, const uint16_t height);

        uint16_t width() const;
        uint16_t height() const;

        const std::vector<uint8_t> & dvec() const;

        uint8_t * bgr_data(); // color order is Blue/Green/Red

    private:

    std::vector<uint8_t> v;
};

#endif // Targa_h

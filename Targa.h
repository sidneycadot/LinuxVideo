
/*************/
/** targa.h **/
/*************/

#ifndef TARGA_H
#define TARGA_H

struct Targa {
  unsigned char header[18];
  unsigned char rgbdata[1];
};

typedef struct Targa *TargaPtr;

TargaPtr targa_alloc(const unsigned short width, const unsigned short height);
void targa_free(const TargaPtr T);

unsigned short targa_width(const TargaPtr T);
unsigned short targa_height(const TargaPtr T);

void targa_plot(TargaPtr T, unsigned x, unsigned y,
                unsigned char R, unsigned char G, unsigned char B);

unsigned long targa_bytesize(const TargaPtr T);

#endif

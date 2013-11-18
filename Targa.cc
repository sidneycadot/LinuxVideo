
/*************/
/** targa.c **/
/*************/

#include <stdlib.h>
#include <assert.h>

#include "targa.h"


TargaPtr targa_alloc(const unsigned short width, const unsigned short height)

{
  TargaPtr T;

  assert(sizeof(struct Targa)==19);

  T = calloc(18+3*width*height,1);

  if (T==0)
    return 0;

  T->header[ 2] = 2;
  T->header[12] = width%256;  T->header[13] = width/256;
  T->header[14] = height%256; T->header[15] = height/256;
  
  T->header[16] = 24;
  T->header[17] = 32;
  
  return T;
}

unsigned short targa_width(const TargaPtr T)
{
  return T->header[12] + 256*T->header[13];
}

unsigned short targa_height(const TargaPtr T)
{
  return T->header[14] + 256*T->header[15];
}

void targa_free(const TargaPtr T)
{
  assert(T);
  free(T);
}

void targa_plot(TargaPtr T, unsigned x, unsigned y,
                unsigned char R, unsigned char G, unsigned char B)

{
  unsigned width, height;

  assert(T);
  
  width  = targa_width(T);
  height = targa_height(T);

  if ((x<width) && (y<height))
    {
      T->rgbdata[3*(x+width*y)+0] = B;
      T->rgbdata[3*(x+width*y)+1] = G;
      T->rgbdata[3*(x+width*y)+2] = R;
    }
}

unsigned long targa_bytesize(const TargaPtr T)

{
  assert(T);
  return 18+3*targa_width(T)*targa_height(T);
}

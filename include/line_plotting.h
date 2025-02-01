#include "epd_driver.h"

// Plots a line on the framebuffer using the Bresenham's Line Algorithm
void plotLineLow(int x0, int y0, int x1, int y1, int bottom, bool shadow, uint8_t *framebuffer)
{
  int dx = x1 - x0;
  int dy = y1 - y0;
  int yi = 1;

  if (dy < 0)
  {
    yi = -1;
    dy = -dy;
  }

  int D = (2 * dy) - dx;
  int y = y0;

  for (int x = x0; x < x1; x++)
  {
    // Update the current pixel's nibble in the framebuffer
    int index = y * EPD_WIDTH / 2 + (int)round(x / 2);
    uint8_t fullByte = framebuffer[index];
    uint8_t updatedByte;

    if (x % 2 == 0)
    {
      uint8_t nibbleLow = (fullByte & 0xF0) >> 4;
      updatedByte = (nibbleLow << 4) | 0x0;
    }
    else
    {
      uint8_t nibbleHigh = fullByte & 0x0F;
      updatedByte = (0x0 << 4) | nibbleHigh;
    }

    framebuffer[index] = updatedByte;

    // Draw shadow if enabled
    if (shadow)
    {
      for (int yg = y + 1; yg < bottom; yg++)
      {
        index = yg * EPD_WIDTH / 2 + (int)round(x / 2);
        fullByte = framebuffer[index];

        if (x % 2 == 0)
        {
          uint8_t nibbleLow = (fullByte & 0xF0) >> 4;
          updatedByte = (nibbleLow << 4) | 0xB;
        }
        else
        {
          uint8_t nibbleHigh = fullByte & 0x0F;
          updatedByte = (0xE << 4) | nibbleHigh;
        }

        framebuffer[index] = updatedByte;
      }
    }

    // Update the decision variable and the y-coordinate
    if (D > 0)
    {
      y = y + yi;
      D = D + (2 * (dy - dx));
    }
    else
    {
      D = D + 2 * dy;
    }
  }
}

void plotLineHigh(int x0, int y0, int x1, int y1, int bottom, bool shadow,
                  uint8_t *framebuffer)
{

  int dx = x1 - x0;
  int dy = y1 - y0;
  int xi = 1;

  if (dx < 0)
  {
    xi = -1;
    dx = -dx;
  }
  int D = (2 * dx) - dy;
  int x = x0;

  for (int y = y0; y < y1; y++)
  {

    if (x % 2 == 0)
    {
      unsigned char full_byte =
          framebuffer[y * EPD_WIDTH / 2 + (int)round(x / 2)];
      unsigned char nibblelow = (char)((full_byte & 0xF0) >> 4);
      framebuffer[y * EPD_WIDTH / 2 + (int)round(x / 2)] =
          (nibblelow << 4) | 0x0;
    }
    else
    {
      unsigned char full_byte =
          framebuffer[y * EPD_WIDTH / 2 + (int)round(x / 2)];
      unsigned char nibblehigh = (char)(full_byte & 0x0F);
      framebuffer[y * EPD_WIDTH / 2 + (int)round(x / 2)] =
          (0x0 << 4) | nibblehigh;
    }
    if (shadow == true)
    {
      for (int yg = y + 1; yg < bottom; yg++)
      {
        if (x % 2 == 0)
        {
          unsigned char full_byte =
              framebuffer[yg * EPD_WIDTH / 2 + (int)round(x / 2)];
          unsigned char nibblelow = (char)((full_byte & 0xF0) >> 4);
          framebuffer[yg * EPD_WIDTH / 2 + (int)round(x / 2)] =
              (nibblelow << 4) | 0xB;
        }
        else
        {
          unsigned char full_byte =
              framebuffer[yg * EPD_WIDTH / 2 + (int)round(x / 2)];
          unsigned char nibblehigh = (char)(full_byte & 0x0F);
          framebuffer[yg * EPD_WIDTH / 2 + (int)round(x / 2)] =
              (0xE << 4) | nibblehigh;
        }
      }
    }
    if (D > 0)
    {
      x = x + xi;
      D = D + (2 * (dx - dy));
    }
    else
    {
      D = D + 2 * dx;
    }
  }
}

void bresenham(int x0, int y0, int x1, int y1, int bottom, bool shadow,
               uint8_t *framebuffer)
{

  if (abs(y1 - y0) < abs(x1 - x0))
  {
    if (x0 > x1)
    {
      plotLineLow(x1, y1, x0, y0, bottom, shadow, framebuffer);
    }
    else
    {
      plotLineLow(x0, y0, x1, y1, bottom, shadow, framebuffer);
    }
  }
  else
  {
    if (y0 > y1)
    {
      plotLineHigh(x1, y1, x0, y0, bottom, shadow, framebuffer);
    }
    else
    {
      plotLineHigh(x0, y0, x1, y1, bottom, shadow, framebuffer);
    }
  }
}
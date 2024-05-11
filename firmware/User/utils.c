#include "utils.h"
#include <stdint.h>


void FifoInit(FIFO_Utils_TypeDef *f)
{
  f->head = 0U;
  f->tail = 0U;
  f->lock = 0U;
  f->size = 128U;
  f->buf = f->buffArr;
}


uint16_t FifoRead(FIFO_Utils_TypeDef *f, void *buf, uint16_t nbytes)
{
  uint16_t i;
  uint8_t *p;

  p = (uint8_t *) buf;

  if (f->lock == 0U)
  {
    f->lock = 1U;

    for (i = 0U; i < nbytes; i++)
    {
      if (f->tail != f->head)
      {
        *p++ = f->buf[f->tail];
        f->tail++;

        if (f->tail == f->size)
        {
          f->tail = 0U;
        }
      }
      else
      {
        f->lock = 0U;
        return i;
      }
    }
  }

  f->lock = 0U;

  return nbytes;
}

/**
  * @brief  USBH_HID_FifoWrite
  *         Write To FIFO.
  * @param  f: Fifo address
  * @param  buf: read buffer
  * @param  nbytes: number of item to write
  * @retval number of written items
  */
uint16_t FifoWrite(FIFO_Utils_TypeDef *f, void *buf, uint16_t  nbytes)
{
  uint16_t i;
  uint8_t *p;

  p = (uint8_t *) buf;

  if (f->lock == 0U)
  {
    f->lock = 1U;

    for (i = 0U; i < nbytes; i++)
    {
      if ((f->head + 1U == f->tail) ||
          ((f->head + 1U == f->size) && (f->tail == 0U)))
      {
        f->lock = 0U;
        return i;
      }
      else
      {
        f->buf[f->head] = *p++;
        f->head++;

        if (f->head == f->size)
        {
          f->head = 0U;
        }
      }
    }
  }

  f->lock = 0U;

  return nbytes;
}




uint16_t collect_bits(uint8_t *p, uint16_t offset, uint8_t size, int is_signed) {
  // mask unused bits of first byte
  uint8_t mask = 0xff << (offset&7);
  uint8_t byte = offset/8;
  uint8_t bits = size;
  uint8_t shift = offset&7;


  uint16_t rval = (p[byte++] & mask) >> shift;
  mask = 0xff;
  shift = 8-shift;
  bits -= shift;

  // first byte already contained more bits than we need
  if(shift > size) {
    // mask unused bits
    rval &= (1<<size)-1;
  } else {
    // further bytes if required
    while(bits) {
      mask = (bits<8)?(0xff>>(8-bits)):0xff;
      rval += (p[byte++] & mask) << shift;
      shift += 8;
      bits -= (bits>8)?8:bits;
    }
  }

  if(is_signed) {
    // do sign expansion
    uint16_t sign_bit = 1<<(size-1);
    if(rval & sign_bit) {
      while(sign_bit) {
	rval |= sign_bit;
	sign_bit <<= 1;
      }
    }
  }

  return rval;
}

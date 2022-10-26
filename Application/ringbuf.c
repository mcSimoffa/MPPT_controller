#include <string.h>
#include "stm8s_conf.h"
#include "ringbuf.h"


#define BUF_EMPTY 0
#define BUF_FULL  2

//-----------------------------------------------------------------------------
//   PUBLIC FUNCTIONS
//-----------------------------------------------------------------------------
void ringbufInit(ringbuf_t const * p_ringbuf)
{
  assert_param(p_ringbuf);
  assert_param(p_ringbuf->bufsize > 1);
  ringbufReset(p_ringbuf);
}

//-----------------------------------------------------------------------------
size_t ringbufGetFree(ringbuf_t const *p_ringbuf)
{
  assert_param(p_ringbuf);
  if (p_ringbuf->data->tail == p_ringbuf->data->head)
  {
    if (p_ringbuf->data->state == BUF_EMPTY)
    {
      return p_ringbuf->bufsize;
    }
    else
    {
      return 0;
    }
  }
  if (p_ringbuf->data->tail > p_ringbuf->data->head)
  {
    return p_ringbuf->data->tail - p_ringbuf->data->head;
  }
  else
  {
    return p_ringbuf->data->tail + p_ringbuf->bufsize - p_ringbuf->data->head;
  }
}

//-----------------------------------------------------------------------------
size_t ringbufGetFull(ringbuf_t const *p_ringbuf)
{
  assert_param(p_ringbuf);
  if (p_ringbuf->data->tail == p_ringbuf->data->head)
  {
    if (p_ringbuf->data->state == BUF_FULL)
    {
      return p_ringbuf->bufsize;
    }
    else
    {
      return 0;
    }
  }
  if (p_ringbuf->data->head > p_ringbuf->data->tail)
  {
    return p_ringbuf->data->head - p_ringbuf->data->tail;
  }
  else
  {
    return p_ringbuf->data->head + p_ringbuf->bufsize - p_ringbuf->data->tail;
  }
}

//-----------------------------------------------------------------------------
size_t ringbufPut(ringbuf_t const * p_ringbuf, uint8_t *p_data, size_t length)
{
  assert_param(p_ringbuf);
  assert_param(p_data);
  size_t qtt = ringbufGetFree(p_ringbuf);
  size_t processed = 0;

  if (length < qtt)
  {
    qtt = length;
  }

  if (qtt == 0)
  {
    return 0;
  }

  do
  {
    size_t  partSize = p_ringbuf->bufsize - p_ringbuf->data->head;
    partSize = MIN(partSize, qtt);
    memcpy(&p_ringbuf->p_buffer[p_ringbuf->data->head], p_data + processed, partSize);
    processed += partSize;
    qtt -= partSize;
    p_ringbuf->data->head += partSize;
    p_ringbuf->data->head %= p_ringbuf->bufsize;

    if (p_ringbuf->data->head == p_ringbuf->data->tail)
    {
      p_ringbuf->data->state = BUF_FULL;
    }

  } while (qtt > 0);

  return processed;
}

//-----------------------------------------------------------------------------
rb_retcode_t ringbufGet(ringbuf_t const *p_ringbuf, uint8_t *p_data, size_t *p_size)
{
  assert_param(p_ringbuf);
  assert_param(p_data);
  assert_param(p_size);

  if (*p_size == 0)
  {
    return RB_SUCCESS;
  }

  if (p_ringbuf->data->openedSize)
  {
    return RB_BUSY;
  }

  size_t qtt = ringbufGetFull(p_ringbuf);
  size_t processed = 0;
  qtt = MIN(*p_size, qtt);
  if (qtt == 0)
  {
   *p_size = 0;
    return RB_SUCCESS;
  }

  do
  {
    size_t  partSize = p_ringbuf->bufsize - p_ringbuf->data->tail;
    partSize = MIN(partSize, qtt);
    memcpy(p_data + processed, &p_ringbuf->p_buffer[p_ringbuf->data->tail] , partSize);
    processed += partSize;
    qtt -= partSize;
    p_ringbuf->data->tail += partSize;
    p_ringbuf->data->tail %= p_ringbuf->bufsize;

    if (p_ringbuf->data->head == p_ringbuf->data->tail)
    {
      p_ringbuf->data->state = BUF_EMPTY;
    }

  } while (qtt > 0);

  *p_size = processed;
  return RB_SUCCESS;
}

//-----------------------------------------------------------------------------
rb_retcode_t ringbufOpenDirectRead(ringbuf_t const *p_ringbuf, uint8_t **pp_data, size_t *p_size)
{
  assert_param(p_ringbuf);
  assert_param(pp_data);
  assert_param(p_size);

  if (p_ringbuf->data->openedSize)
  {
    return RB_BUSY;
  }

  size_t qtt = ringbufGetFull(p_ringbuf);

  size_t  partSize = p_ringbuf->bufsize - p_ringbuf->data->tail;

  *p_size = p_ringbuf->data->openedSize = MIN(partSize, qtt);
  *pp_data = &p_ringbuf->p_buffer[p_ringbuf->data->tail];
  return RB_SUCCESS;
}

//-----------------------------------------------------------------------------
void ringbufApplyRead(ringbuf_t const *p_ringbuf, size_t size)
{
  assert_param(p_ringbuf);
  assert_param(p_ringbuf->data);
  assert_param(size <= p_ringbuf->data->openedSize);

  p_ringbuf->data->openedSize = 0;

  p_ringbuf->data->tail %= p_ringbuf->bufsize;
  if (p_ringbuf->data->head == p_ringbuf->data->tail)
  {
    p_ringbuf->data->state = BUF_EMPTY;
  }
}

//-----------------------------------------------------------------------------
void ringbufReset(ringbuf_t const *p_ringbuf)
{
  p_ringbuf->data->head = p_ringbuf->data->tail = 0;
  p_ringbuf->data->state = BUF_EMPTY;
  p_ringbuf->data->openedSize = 0;
}

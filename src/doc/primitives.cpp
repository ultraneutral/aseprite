// Aseprite Document Library
// Copyright (c) 2001-2015 David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "doc/primitives.h"

#include "doc/algo.h"
#include "doc/brush.h"
#include "doc/image_impl.h"
#include "doc/palette.h"
#include "doc/rgbmap.h"

#include <stdexcept>

namespace doc {

color_t get_pixel(const Image* image, int x, int y)
{
  ASSERT(image);

  if ((x >= 0) && (y >= 0) && (x < image->width()) && (y < image->height()))
    return image->getPixel(x, y);
  else
    return -1;
}

void put_pixel(Image* image, int x, int y, color_t color)
{
  ASSERT(image);

  if ((x >= 0) && (y >= 0) && (x < image->width()) && (y < image->height()))
    image->putPixel(x, y, color);
}

void clear_image(Image* image, color_t color)
{
  ASSERT(image);

  image->clear(color);
}

void copy_image(Image* dst, const Image* src)
{
  ASSERT(dst);
  ASSERT(src);

  dst->copy(src, gfx::Clip(0, 0, 0, 0, src->width(), src->height()));
}

void copy_image(Image* dst, const Image* src, int x, int y)
{
  ASSERT(dst);
  ASSERT(src);

  dst->copy(src, gfx::Clip(x, y, 0, 0, src->width(), src->height()));
}

Image* crop_image(const Image* image, int x, int y, int w, int h, color_t bg, const ImageBufferPtr& buffer)
{
  ASSERT(image);

  if (w < 1) throw std::invalid_argument("image_crop: Width is less than 1");
  if (h < 1) throw std::invalid_argument("image_crop: Height is less than 1");

  Image* trim = Image::create(image->pixelFormat(), w, h, buffer);
  trim->setMaskColor(image->maskColor());

  clear_image(trim, bg);
  trim->copy(image, gfx::Clip(0, 0, x, y, w, h));

  return trim;
}

void rotate_image(const Image* src, Image* dst, int angle)
{
  ASSERT(src);
  ASSERT(dst);
  int x, y;

  switch (angle) {

    case 180:
      ASSERT(dst->width() == src->width());
      ASSERT(dst->height() == src->height());

      for (y=0; y<src->height(); ++y)
        for (x=0; x<src->width(); ++x)
          dst->putPixel(src->width() - x - 1,
                        src->height() - y - 1, src->getPixel(x, y));
      break;

    case 90:
      ASSERT(dst->width() == src->height());
      ASSERT(dst->height() == src->width());

      for (y=0; y<src->height(); ++y)
        for (x=0; x<src->width(); ++x)
          dst->putPixel(src->height() - y - 1, x, src->getPixel(x, y));
      break;

    case -90:
      ASSERT(dst->width() == src->height());
      ASSERT(dst->height() == src->width());

      for (y=0; y<src->height(); ++y)
        for (x=0; x<src->width(); ++x)
          dst->putPixel(y, src->width() - x - 1, src->getPixel(x, y));
      break;

    // bad angle
    default:
      throw std::invalid_argument("Invalid angle specified to rotate the image");
  }
}

void draw_hline(Image* image, int x1, int y, int x2, color_t color)
{
  ASSERT(image);
  int t;

  if (x1 > x2) {
    t = x1;
    x1 = x2;
    x2 = t;
  }

  if ((x2 < 0) || (x1 >= image->width()) || (y < 0) || (y >= image->height()))
    return;

  if (x1 < 0) x1 = 0;
  if (x2 >= image->width()) x2 = image->width()-1;

  image->drawHLine(x1, y, x2, color);
}

void draw_vline(Image* image, int x, int y1, int y2, color_t color)
{
  ASSERT(image);
  int t;

  if (y1 > y2) {
    t = y1;
    y1 = y2;
    y2 = t;
  }

  if ((y2 < 0) || (y1 >= image->height()) || (x < 0) || (x >= image->width()))
    return;

  if (y1 < 0) y1 = 0;
  if (y2 >= image->height()) y2 = image->height()-1;

  for (t=y1; t<=y2; t++)
    image->putPixel(x, t, color);
}

void draw_rect(Image* image, int x1, int y1, int x2, int y2, color_t color)
{
  ASSERT(image);
  int t;

  if (x1 > x2) {
    t = x1;
    x1 = x2;
    x2 = t;
  }

  if (y1 > y2) {
    t = y1;
    y1 = y2;
    y2 = t;
  }

  if ((x2 < 0) || (x1 >= image->width()) || (y2 < 0) || (y1 >= image->height()))
    return;

  draw_hline(image, x1, y1, x2, color);
  draw_hline(image, x1, y2, x2, color);
  if (y2-y1 > 1) {
    draw_vline(image, x1, y1+1, y2-1, color);
    draw_vline(image, x2, y1+1, y2-1, color);
  }
}

void fill_rect(Image* image, int x1, int y1, int x2, int y2, color_t color)
{
  ASSERT(image);
  int t;

  if (x1 > x2) {
    t = x1;
    x1 = x2;
    x2 = t;
  }

  if (y1 > y2) {
    t = y1;
    y1 = y2;
    y2 = t;
  }

  if ((x2 < 0) || (x1 >= image->width()) || (y2 < 0) || (y1 >= image->height()))
    return;

  if (x1 < 0) x1 = 0;
  if (y1 < 0) y1 = 0;
  if (x2 >= image->width()) x2 = image->width()-1;
  if (y2 >= image->height()) y2 = image->height()-1;

  image->fillRect(x1, y1, x2, y2, color);
}

void fill_rect(Image* image, const gfx::Rect& rc, color_t c)
{
  ASSERT(image);

  gfx::Rect clip = rc.createIntersection(image->bounds());
  if (!clip.isEmpty())
    image->fillRect(clip.x, clip.y,
      clip.x+clip.w-1, clip.y+clip.h-1, c);
}

void blend_rect(Image* image, int x1, int y1, int x2, int y2, color_t color, int opacity)
{
  ASSERT(image);
  int t;

  if (x1 > x2) {
    t = x1;
    x1 = x2;
    x2 = t;
  }

  if (y1 > y2) {
    t = y1;
    y1 = y2;
    y2 = t;
  }

  if ((x2 < 0) || (x1 >= image->width()) || (y2 < 0) || (y1 >= image->height()))
    return;

  if (x1 < 0) x1 = 0;
  if (y1 < 0) y1 = 0;
  if (x2 >= image->width()) x2 = image->width()-1;
  if (y2 >= image->height()) y2 = image->height()-1;

  image->blendRect(x1, y1, x2, y2, color, opacity);
}

struct Data {
  Image* image;
  color_t color;
};

static void pixel_for_image(int x, int y, Data* data)
{
  put_pixel(data->image, x, y, data->color);
}

static void hline_for_image(int x1, int y, int x2, Data* data)
{
  draw_hline(data->image, x1, y, x2, data->color);
}

void draw_line(Image* image, int x1, int y1, int x2, int y2, color_t color)
{
  Data data = { image, color };
  algo_line(x1, y1, x2, y2, &data, (AlgoPixel)pixel_for_image);
}

void draw_ellipse(Image* image, int x1, int y1, int x2, int y2, color_t color)
{
  Data data = { image, color };
  algo_ellipse(x1, y1, x2, y2, &data, (AlgoPixel)pixel_for_image);
}

void fill_ellipse(Image* image, int x1, int y1, int x2, int y2, color_t color)
{
  Data data = { image, color };
  algo_ellipsefill(x1, y1, x2, y2, &data, (AlgoHLine)hline_for_image);
}

namespace {

template<typename ImageTraits>
int count_diff_between_images_templ(const Image* i1, const Image* i2)
{
  int diff = 0;
  const LockImageBits<ImageTraits> bits1(i1);
  const LockImageBits<ImageTraits> bits2(i2);
  typename LockImageBits<ImageTraits>::const_iterator it1, it2, end1, end2;
  for (it1 = bits1.begin(), end1 = bits1.end(),
       it2 = bits2.begin(), end2 = bits2.end();
       it1 != end1 && it2 != end2; ++it1, ++it2) {
    if (*it1 != *it2)
      diff++;
  }

  ASSERT(it1 == end1);
  ASSERT(it2 == end2);
  return diff;
}

} // anonymous namespace

int count_diff_between_images(const Image* i1, const Image* i2)
{
  if ((i1->pixelFormat() != i2->pixelFormat()) ||
      (i1->width() != i2->width()) ||
      (i1->height() != i2->height()))
    return -1;

  switch (i1->pixelFormat()) {
    case IMAGE_RGB:       return count_diff_between_images_templ<RgbTraits>(i1, i2);
    case IMAGE_GRAYSCALE: return count_diff_between_images_templ<GrayscaleTraits>(i1, i2);
    case IMAGE_INDEXED:   return count_diff_between_images_templ<IndexedTraits>(i1, i2);
    case IMAGE_BITMAP:    return count_diff_between_images_templ<BitmapTraits>(i1, i2);
  }

  ASSERT(false);
  return -1;
}

} // namespace doc

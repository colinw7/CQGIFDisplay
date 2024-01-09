#include <CPNGImage.h>
#include <CGenImage.h>
#include <CFile.h>
#include <CRGBA.h>

#include <png.h>

static void pngWriteErrorHandler(png_structp png_ptr, png_const_charp msg);

CPNGImage::
CPNGImage()
{
}

bool
CPNGImage::
read(CFile *file, CGenImage *image)
{
  try {
    file->rewind();

    uchar header[8];

    file->read(header, 8);

    if (! png_check_sig(header, 8))
      return false;
  }
  catch (...) {
    return false;
  }

  png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);

  if (png_ptr == nullptr)
    return false;

  png_infop info_ptr = png_create_info_struct(png_ptr);

  if (info_ptr == nullptr) {
    png_destroy_read_struct(&png_ptr, nullptr, nullptr);
    return false;
  }

  png_infop end_info = png_create_info_struct(png_ptr);

  if (end_info == nullptr) {
    png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
    return false;
  }

  if (setjmp(png_jmpbuf(png_ptr))) {
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    return false;
  }

  png_init_io(png_ptr, file->getFP());

  png_set_sig_bytes(png_ptr, 8);

  png_read_info(png_ptr, info_ptr);

  int bit_depth  = png_get_bit_depth(png_ptr, info_ptr);
  int color_type = png_get_color_type(png_ptr, info_ptr);

  png_colorp palette;
  int        num_palette;

  if (! png_get_PLTE(png_ptr, info_ptr, &palette, &num_palette))
    num_palette = 0;

  if (color_type == PNG_COLOR_TYPE_PALETTE && bit_depth <= 8)
    png_set_expand(png_ptr);

  if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
    png_set_expand(png_ptr);

  if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
    png_set_expand(png_ptr);

  if (bit_depth == 16)
    png_set_strip_16(png_ptr);

#if 0
  if (color_type & PNG_COLOR_MASK_ALPHA)
    png_set_strip_alpha(png_ptr);
#endif

  if (bit_depth < 8)
    png_set_packing(png_ptr);

  if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
    png_set_gray_to_rgb(png_ptr);

#if 0
  int number_of_passes = png_set_interlace_handling(png_ptr);
#endif

  png_read_update_info(png_ptr, info_ptr);

  int width            = int(png_get_image_width (png_ptr, info_ptr));
  int height           = int(png_get_image_height(png_ptr, info_ptr));
#if 0
  int filter_type      = png_get_filter_type(png_ptr, info_ptr);
  int compression_type = png_get_compression_type(png_ptr, info_ptr);
  int interlace_type   = png_get_interlace_type(png_ptr, info_ptr);
  int channels         = png_get_channels(png_ptr, info_ptr);
#endif
  int rowbytes         = int(png_get_rowbytes(png_ptr, info_ptr));

  bit_depth  = png_get_bit_depth(png_ptr, info_ptr);
  color_type = png_get_color_type(png_ptr, info_ptr);

  //------

  auto *row_pointers = new png_bytep [size_t(height)];

  for (int i = 0; i < height; ++i)
    row_pointers[i] = new uchar [size_t(rowbytes)];

  //------

  png_read_image(png_ptr, row_pointers);

  png_read_end(png_ptr, end_info);

  png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);

  //------

  int depth = 8;

  if (bit_depth == 8)
    depth = 24;

  auto *data = new uint [size_t(width*height)];

  if (depth == 24) {
    if (color_type & PNG_COLOR_MASK_ALPHA) {
      int k = 0;

      for (int i = 0; i < height; ++i) {
        int j1 = 0;

        for (int j = 0; j < width; ++j) {
          data[k++] = CRGBA::encodeARGB(row_pointers[i][j1 + 0], row_pointers[i][j1 + 1],
                                        row_pointers[i][j1 + 2], row_pointers[i][j1 + 3]);

          j1 += 4;
        }
      }
    }
    else {
      int k = 0;

      for (int i = 0; i < height; ++i) {
        int j1 = 0;

        for (int j = 0; j < width; ++j) {
          data[k++] = CRGBA::encodeARGB(row_pointers[i][j1 + 0], row_pointers[i][j1 + 1],
                                        row_pointers[i][j1 + 2], 255);

          j1 += 3;
        }
      }
    }
  }
  else {
    int k = 0;

    for (int i = 0; i < height; ++i)
      for (int j = 0; j < width; ++j)
        data[k++] = row_pointers[i][j];
  }

  //------

  for (int i = 0; i < height; ++i)
    delete [] row_pointers[i];

  delete [] row_pointers;

  //------

  image->setType(CGenImage::Type::PNG);

  image->setDataSize(uint(width), uint(height));

  if (depth <= 8) {
    for (int i = 0; i < num_palette; ++i) {
      CGenImage::RGBA rgba(palette[i].red/255.0, palette[i].green/255.0, palette[i].blue/255.0);

      image->addColor(rgba);
    }

    for (int y = 0, k = 0; y < height; ++y)
      for (int x = 0; x < width; ++x, ++k)
        image->setColorIndex(uint(x), uint(y), data[k]);
  }
  else {
    for (int y = 0, k = 0; y < height; ++y)
      for (int x = 0; x < width; ++x, ++k)
        image->setPixel(uint(x), uint(y), data[k]);
  }

  delete [] data;

  //------

  return true;
}

bool
CPNGImage::
readHeader(CFile *file, CGenImage *image)
{
  try {
    file->rewind();

    uchar header[8];

    file->read(header, 8);

    if (! png_check_sig(header, 8))
      return false;
  }
  catch (...) {
    return false;
  }

  png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);

  if (png_ptr == nullptr)
    return false;

  png_infop info_ptr = png_create_info_struct(png_ptr);

  if (info_ptr == nullptr) {
    png_destroy_read_struct(&png_ptr, nullptr, nullptr);
    return false;
  }

  png_infop end_info = png_create_info_struct(png_ptr);

  if (end_info == nullptr) {
    png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
    return false;
  }

  if (setjmp(png_jmpbuf(png_ptr))) {
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    return false;
  }

  png_init_io(png_ptr, file->getFP());

  png_set_sig_bytes(png_ptr, 8);

  png_read_info(png_ptr, info_ptr);

  int width  = int(png_get_image_width (png_ptr, info_ptr));
  int height = int(png_get_image_height(png_ptr, info_ptr));

  png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);

  //------

  image->setType(CGenImage::Type::PNG);

  image->setSize(uint(width), uint(height));

  //------

  return true;
}

bool
CPNGImage::
write(CFile *file, CGenImage *image)
{
  if (image->getWidth() == 0 && image->getHeight() == 0)
    return false;

  //---

  file->open(CFile::Mode::WRITE);

  if (image->hasColormap())
    return false;

  png_structp png_ptr =
    png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, pngWriteErrorHandler, nullptr);

  if (! png_ptr)
    return false;

  png_infop info_ptr = png_create_info_struct(png_ptr);

  if (! info_ptr) {
    png_destroy_write_struct(&png_ptr, nullptr);
    return false;
  }

  if (setjmp(png_jmpbuf(png_ptr))) {
    png_destroy_write_struct(&png_ptr, &info_ptr);
    return false;
  }

  png_init_io(png_ptr, file->getFP());

  // png_set_compression_level(png_ptr, Z_BEST_COMPRESSION);

  int color_type     = PNG_COLOR_TYPE_RGB_ALPHA;
  int interlace_type = PNG_INTERLACE_NONE;

  auto width  = image->getWidth ();
  auto height = image->getHeight();

  png_set_IHDR(png_ptr, info_ptr, width, height, 8, color_type, interlace_type,
               PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

  png_write_info(png_ptr, info_ptr);

  png_set_packing(png_ptr);

  uchar *row_data = new uchar [4*width];

  int i = 0, j = 0;

  uint r, g, b, a;

  for (uint y = 0; y < height; ++y) {
    j = 0;

    for (uint x = 0; x < width; ++x, ++i, j += 4) {
      auto pixel = image->getPixel(x, y);

      CRGBA::decodeARGB(pixel, &r, &g, &b, &a);

      row_data[j + 0] = uchar(r);
      row_data[j + 1] = uchar(g);
      row_data[j + 2] = uchar(b);
      row_data[j + 3] = uchar(a);
    }

    png_write_row(png_ptr, row_data);
  }

  png_write_end(png_ptr, nullptr);

  delete [] row_data;

  png_destroy_write_struct(&png_ptr, &info_ptr);

  return true;
}

static void
pngWriteErrorHandler(png_structp png_ptr, png_const_charp msg)
{
  fprintf(stderr, "PNG write %s\n", msg);

  longjmp(png_jmpbuf(png_ptr), 1);
}

#include <CJPGImage.h>
#include <CGenImage.h>
#include <CFile.h>
#include <csetjmp>

#if 0
extern "C" {
 #include <jpeg/jpeglib.h>
}

static jmp_buf jpeg_setjmp_buffer;
#endif

CJPGImage::
CJPGImage()
{
}

bool
CJPGImage::
read(CFile *file, CGenImage *image)
{
#if 0
  struct jpeg_error_mgr         jerr;
  struct jpeg_decompress_struct cinfo;

  //------

  cinfo.err = jpeg_std_error(&jerr);

  jerr.error_exit     = jpgErrorProc;
  jerr.output_message = jpgMessageProc;

  if (setjmp(jpeg_setjmp_buffer) == 1) {
    jpeg_destroy_decompress(&cinfo);
    return false;
  }

  //------

  jpeg_create_decompress(&cinfo);

/*
  jpeg_set_marker_processor(&cinfo, JPEG_COM, jpgProcessMarker);
*/

  //------

  file->open(CFile::READ);

  file->rewind();

  jpeg_stdio_src(&cinfo, file->getFP());

  //------

  jpeg_read_header(&cinfo, TRUE);

  //------

  int depth = 24;

  if (depth != 24) {
    cinfo.quantize_colors = TRUE;

    if (cinfo.desired_number_of_colors > 256)
      cinfo.desired_number_of_colors = 256;
  }
  else
    cinfo.quantize_colors = FALSE;

  if (cinfo.jpeg_color_space == JCS_GRAYSCALE)
    cinfo.out_color_space = JCS_GRAYSCALE;
  else
    cinfo.out_color_space = JCS_RGB;

  //------

  jpeg_start_decompress(&cinfo);

  //------

  uint *data = new uint [cinfo.output_width*cinfo.output_height];

  JSAMPROW buffer;

  if (depth == 24) {
    if (cinfo.jpeg_color_space == JCS_GRAYSCALE)
      buffer = new JSAMPLE [cinfo.output_width];
    else
      buffer = new JSAMPLE [3*cinfo.output_width];
  }
  else
    buffer = new JSAMPLE [cinfo.output_width];

  JSAMPROW rowptr[1];

  rowptr[0] = buffer;

  int j = 0;

  while (cinfo.output_scanline < cinfo.output_height) {
    jpeg_read_scanlines(&cinfo, rowptr, 1);

    if (depth == 24) {
      if (cinfo.jpeg_color_space == JCS_GRAYSCALE) {
        for (uint k = 0; k < cinfo.output_width; ++k)
          data[j++] = CRGBA::encodeARGB(buffer[k], buffer[k], buffer[k]);
      }
      else {
        for (uint k = 0; k < 3*cinfo.output_width; k += 3)
          data[j++] = CRGBA::encodeARGB(buffer[k + 0], buffer[k + 1], buffer[k + 2]);
      }
    }
    else {
      for (uint k = 0; k < cinfo.output_width; ++k)
        data[j++] = buffer[k];
    }
  }

  delete [] buffer;

  //------

  image->setType(CFILE_TYPE_IMAGE_JPG);

  image->setDataSize(cinfo.output_width, cinfo.output_height);

  if (depth != 24) {
    if (cinfo.out_color_space == JCS_RGB) {
      for (int i = 0; i < cinfo.actual_number_of_colors; ++i) {
        int r = cinfo.colormap[0][i];
        int g = cinfo.colormap[1][i];
        int b = cinfo.colormap[2][i];

        CRGBA rgba;

        rgba.setRGBAI(r, g, b);

        image->addColor(rgba);

        if (CJPGImage::getDebug())
          printf("%d) R %d G %d B %d\n", i + 1, r, g, b);
      }
    }
    else {
      for (int i = 0; i < cinfo.actual_number_of_colors; ++i) {
        int g = cinfo.colormap[0][i];

        CRGBA rgba;

        rgba.setRGBAI(g, g, g);

        image->addColor(rgba);

        if (CJPGImage::getDebug())
          printf("%d) R %d G %d B %d\n", i + 1, g, g, g);
      }
    }

    for (uint y = 0, k = 0; y < cinfo.output_height; ++y)
      for (uint x = 0; x < cinfo.output_width; ++x, ++k)
        image->setColorIndex(x, y, data[k]);
  }
  else {
    for (uint y = 0, k = 0; y < cinfo.output_height; ++y)
      for (uint x = 0; x < cinfo.output_width; ++x, ++k)
        image->setPixel(x, y, data[k]);
  }

  delete [] data;

  //------

  jpeg_finish_decompress(&cinfo);

  //------

  jpeg_destroy_decompress(&cinfo);

  //------

  return true;
#else
  return false;
#endif
}

bool
CJPGImage::
readHeader(CFile *file, CGenImage *image)
{
#if 0
  struct jpeg_error_mgr         jerr;
  struct jpeg_decompress_struct cinfo;

  //------

  cinfo.err = jpeg_std_error(&jerr);

  jerr.error_exit     = jpgErrorProc;
  jerr.output_message = jpgMessageProc;

  if (setjmp(jpeg_setjmp_buffer) == 1) {
    jpeg_destroy_decompress(&cinfo);
    return false;
  }

  //------

  jpeg_create_decompress(&cinfo);

/*
  jpeg_set_marker_processor(&cinfo, JPEG_COM, jpgProcessMarker);
*/

  //------

  file->open(CFile::READ);

  file->rewind();

  jpeg_stdio_src(&cinfo, file->getFP());

  //------

  jpeg_read_header(&cinfo, TRUE);

  //------

  image->setType(CFILE_TYPE_IMAGE_JPG);

  image->setSize(cinfo.image_width, cinfo.image_height);

  //------

  jpeg_destroy_decompress(&cinfo);

  //------

  return true;
#endif
}

int
CJPGImage::
jpgProcessMarker(struct jpeg_decompress_struct *cinfo)
{
#if 0
  std::cerr << "In jpgProcessMarker" << std::endl;

  int length = 0;

  length += jpgGetC(cinfo) << 8;
  length += jpgGetC(cinfo) - 2;

  for (int i = 0; i < length; ++i)
    jpgGetC(cinfo);

  return 1;
#else
  return 0;
#endif
}

int
CJPGImage::
jpgGetC(struct jpeg_decompress_struct *cinfo)
{
#if 0
  struct jpeg_source_mgr *datasrc = cinfo->src;

  if (datasrc->bytes_in_buffer == 0) {
    if (! (*datasrc->fill_input_buffer)(cinfo))
      longjmp(jpeg_setjmp_buffer, 1);
  }

  --datasrc->bytes_in_buffer;

  return GETJOCTET(*datasrc->next_input_byte++);
#else
  return 0;
#endif
}

void
CJPGImage::
jpgErrorProc(struct jpeg_common_struct *cinfo)
{
#if 0
  std::cerr << "JPEG Error" << std::endl;

  (*cinfo->err->output_message)(cinfo);

  longjmp(jpeg_setjmp_buffer, 1);
#endif
}

void
CJPGImage::
jpgMessageProc(struct jpeg_common_struct *cinfo)
{
#if 0
  char buffer[JMSG_LENGTH_MAX];

  (*cinfo->err->format_message)(cinfo, buffer);

  std::cerr << buffer << std::endl;
#endif
}

bool
CJPGImage::
write(CFile *file, CGenImage *image)
{
#if 0
  struct jpeg_error_mgr       jerr;
  struct jpeg_compress_struct cinfo;

  cinfo.err = jpeg_std_error(&jerr);

  jpeg_create_compress(&cinfo);

  file->open(CFile::WRITE);

  jpeg_stdio_dest(&cinfo, file->getFP());

  cinfo.image_width      = image->getWidth();
  cinfo.image_height     = image->getHeight();
  cinfo.input_components = 3;
  cinfo.in_color_space   = JCS_RGB;

  jpeg_set_defaults(&cinfo);

  jpeg_start_compress(&cinfo, TRUE);

  uchar *data = new uchar [3*image->getWidth()];

  JSAMPROW row_pointer[1];

  row_pointer[0] = data;

  int k = 0;

  uint r, g, b, a;

  for (int y = 0; cinfo.next_scanline < cinfo.image_height; ++y) {
    int j = 0;

    for (uint x = 0; x < image->getWidth(); ++x, j += 3, ++k) {
      uint pixel = image->getPixel(x, y);

      CRGBA::decodeARGB(pixel, &r, &g, &b, &a);

      data[j + 2] = b;
      data[j + 1] = g;
      data[j + 0] = r;
    }

    jpeg_write_scanlines(&cinfo, row_pointer, 1);
  }

  jpeg_finish_compress(&cinfo);

  jpeg_destroy_compress(&cinfo);

  delete [] data;

  return true;
#else
  return false;
#endif
}

#include <CGIFImage.h>
#include <CGIFAnim.h>
#include <CGenImage.h>
#include <CThrow.h>
#include <CStrUtil.h>
#include <CFile.h>

#define CIMAGE_GIF_DICT_SIZE 5021

#define GIF87a 0
#define GIF89a 0

#define IMAGE_ID   0x2C
#define CONTROL_ID 0x21
#define TRAILER_ID 0x3B

#define CONTROL_APPEXT_ID  0xFF
#define CONTROL_COMMENT_ID 0xFE
#define CONTROL_LABEL_ID   0xF9

#define UNUSED_CODE ((uint) -1)

struct CGIFImageHeader;
struct CGIFImageColorTable;

struct CGIFImageData {
  CGIFImageHeader     *header;
  CGIFImageColorTable *global_colors;
  int                  num_global_colors;
  CGIFImageColorTable *local_colors;
  int                  num_local_colors;
};

struct CGIFImageHeader {
  char   signature[3];
  char   version[3];
  ushort width;
  ushort height;
  uchar  flags;
  uchar  background;
  uchar  aspect;

  uchar  color_bits;
  uchar  colors_sorted;
  uchar  max_color_bits;
  uchar  global_color_table;
};

struct CGIFImageColorTable {
  uchar r;
  uchar g;
  uchar b;
};

struct CGIFImageImageHeader {
  ushort left;
  ushort top;
  ushort width;
  ushort height;
  uchar  flags;

  uchar  local_color_table;
  uchar  interlaced;
  uchar  colors_sorted;
  uchar  color_bits;
};

struct CGIFImageCompressData {
  uchar         bit_mask;
  uchar         code_size;
  uchar         init_code_size;
  uint          code_mask;
  uint          clear_code;
  uint          eof_code;
  uint          free_code;
  uint          max_code;
  uchar         max_code_size;
  uint          current_bit;
  uchar         current_byte;

  CGIFImageDict dictionary[CIMAGE_GIF_DICT_SIZE];
  uchar         buffer[256];

  int           num_code_bytes;
  uchar         code_bytes[256];
  uint          color_table_bits;

  uchar         out_bytes[1024];
};

static CGIFImageCompressData compress_data;

CGIFImage::
CGIFImage()
{
}

bool
CGIFImage::
read(CFile *file, CGenImage *image)
{
  CGIFAnim *image_anim = createAnim(file, image);

  if (image_anim->begin() != image_anim->end()) {
    CGIFFrame *frame = *(image_anim->begin());

    CGenImage *ptr = frame->getImage();

    image->assign(*ptr);
  }

  delete image_anim;

  return true;
}

bool
CGIFImage::
readHeader(CFile *file, CGenImage *image)
{
  file->rewind();

  try {
    CGIFImageHeader header;

    readHeader(file, image, &header);

    //------

    image->setType(CFILE_TYPE_IMAGE_GIF);

    image->setSize(header.width, header.height);
  }
  catch (...) {
    CTHROW("Failed to read GIF file");
    return false;
  }

  return true;
}

CGIFAnim *
CGIFImage::
createAnim(CFile *file, CGenImage *proto)
{
  CGIFAnim *image_anim = new CGIFAnim;

  //------

  file->rewind();

  //------

  CGIFImageData *gif_data = new CGIFImageData;

  gif_data->header = new CGIFImageHeader;

  gif_data->global_colors     = NULL;
  gif_data->num_global_colors = 0;
  gif_data->local_colors      = NULL;
  gif_data->num_local_colors  = 0;

  //------

  try {
    memset(&compress_data, 0, sizeof(CGIFImageCompressData));

    CGenImage *image = NULL;

    readHeader(file, image, gif_data->header);

    //------

    readGlobalColors(file, gif_data);

    //------

    readAnimData(file, proto, image_anim, gif_data);
  }
  catch (...) {
    CTHROW("Failed to read GIF file");
  }

  //------

  if (gif_data != NULL) {
    delete gif_data->header;

    delete [] gif_data->global_colors;
    delete [] gif_data->local_colors;
  }

  delete gif_data;

  //------

  return image_anim;
}

void
CGIFImage::
readHeader(CFile *file, CGenImage *, CGIFImageHeader *header)
{
  uchar byte1;
  uchar byte2;

  file->read((uchar *) header->signature, 3);
  file->read((uchar *) header->version  , 3);

  file->read(&byte1, 1);
  file->read(&byte2, 1);

  header->width = (byte2 << 8) | byte1;

  file->read(&byte1, 1);
  file->read(&byte2, 1);

  header->height = (byte2 << 8) | byte1;

  file->read(&header->flags     , 1);
  file->read(&header->background, 1);
  file->read(&header->aspect    , 1);

  header->color_bits         = (header->flags     ) & 0x07;
  header->colors_sorted      = (header->flags >> 3) & 0x01;
  header->max_color_bits     = (header->flags >> 4) & 0x07;
  header->global_color_table = (header->flags >> 7) & 0x01;

  if (CGIFImage::getDebug()) {
    std::cerr << "Signature     " << header->signature[0] << header->signature[1] <<
                                     header->signature[2] << std::endl;
    std::cerr << "Version       " << header->version[0] << header->version[1] <<
                                     header->version[2] << std::endl;
    std::cerr << "Width         " << header->width << std::endl;
    std::cerr << "Height        " << header->height << std::endl;
    std::cerr << "Num Colors    " << (1 << (header->color_bits + 1)) << std::endl;
    std::cerr << "Colors Sorted " << (int) header->colors_sorted << std::endl;
    std::cerr << "Max Colors    " << 1 << (header->max_color_bits + 1) << std::endl;
    std::cerr << "Global Colors " << (int) header->global_color_table << std::endl;
    std::cerr << "Background    " << (int) header->background << std::endl;
    std::cerr << "Aspect        " << (int) header->aspect << std::endl;
  }

  if (strncmp(header->signature, "GIF", 3) != 0)
    CTHROW("Not a GIF File");

  int type = 0;

  if      (strncmp(header->version, "87a", 3) == 0)
    type = GIF87a;
  else if (strncmp(header->version, "89a", 3) == 0)
    type = GIF89a;

  if (type != GIF87a && type != GIF89a)
    CTHROW("Invalid GIF Version");
}

void
CGIFImage::
readGlobalColors(CFile *file, CGIFImageData *gif_data)
{
  if (gif_data->header->global_color_table) {
    gif_data->num_global_colors = 1 << (gif_data->header->color_bits + 1);

    gif_data->global_colors = new CGIFImageColorTable [gif_data->num_global_colors];

    for (int i = 0; i < gif_data->num_global_colors; ++i)
      file->read((uchar *) &gif_data->global_colors[i], 3);
  }

  compress_data.bit_mask = gif_data->num_global_colors - 1;

  if (CGIFImage::getDebug()) {
    for (int i = 0; i < gif_data->num_global_colors; ++i)
      std::cerr << "Color: " << i << " " <<
                   (int) gif_data->global_colors[i].r << " " <<
                   (int) gif_data->global_colors[i].g << " " <<
                   (int) gif_data->global_colors[i].b << std::endl;
  }
}

void
CGIFImage::
readAnimData(CFile *file, CGenImage *proto, CGIFAnim *image_anim, CGIFImageData *gif_data)
{
  int  inum              = 0;
  int  delay             = 0;
  bool transparent       = false;
  uint transparent_color = 0;
  int  dispose           = 0;
  int  user_input        = 0;

  uint file_size = file->getSize();

  while (true) {
    uchar id;

    try {
      if (! file->read(&id, 1))
        break;
    }
    catch (...) {
      break;
    }

    if      (id == IMAGE_ID) {
      ++inum;

      if (CGIFImage::getDebug())
        std::cerr << "Image Id" << std::endl;

      CGIFImageImageHeader *image_header = new CGIFImageImageHeader;

      try {
        uchar byte1;
        uchar byte2;

        file->read(&byte1, 1);
        file->read(&byte2, 1);

        image_header->left = (byte2 << 8) | byte1;

        file->read(&byte1, 1);
        file->read(&byte2, 1);

        image_header->top = (byte2 << 8) | byte1;

        file->read(&byte1, 1);
        file->read(&byte2, 1);

        image_header->width = (byte2 << 8) | byte1;

        file->read(&byte1, 1);
        file->read(&byte2, 1);

        image_header->height = (byte2 << 8) | byte1;

        file->read(&image_header->flags, 1);

        image_header->local_color_table = (image_header->flags >> 7) & 0x01;
        image_header->interlaced        = (image_header->flags >> 6) & 0x01;
        image_header->colors_sorted     = (image_header->flags >> 5) & 0x01;
        image_header->color_bits        = (image_header->flags     ) & 0x07;

        if (CGIFImage::getDebug()) {
          std::cerr << "Left          " << image_header->left << std::endl;
          std::cerr << "Top           " << image_header->top << std::endl;
          std::cerr << "Width         " << image_header->width << std::endl;
          std::cerr << "Height        " << image_header->height << std::endl;
          std::cerr << "Local Colors  " << image_header->local_color_table << std::endl;
          std::cerr << "Interlaced    " << image_header->interlaced << std::endl;
          std::cerr << "Colors Sorted " << image_header->colors_sorted << std::endl;
          std::cerr << "Num Colors    " << (1 << (image_header->color_bits + 1)) << std::endl;
        }

        if (image_header->local_color_table &&
            image_header->color_bits > 0) {
          gif_data->num_local_colors = 1 << (image_header->color_bits + 1);

          gif_data->local_colors = new CGIFImageColorTable [gif_data->num_local_colors];

          for (int i = 0; i < gif_data->num_local_colors; ++i)
            file->read((uchar *) &gif_data->local_colors[i], 3);

          if (CGIFImage::getDebug()) {
            for (int i = 0; i < gif_data->num_local_colors; ++i)
              std::cerr << gif_data->local_colors[i].r << " " <<
                           gif_data->local_colors[i].g << " " <<
                           gif_data->local_colors[i].b << std::endl;
          }
        }

        file->read(&compress_data.code_size, 1);

        compress_data.clear_code = 1 << compress_data.code_size;
        compress_data.eof_code   = compress_data.clear_code + 1;
        compress_data.free_code  = compress_data.clear_code + 2;

        ++compress_data.code_size;

        compress_data.init_code_size = compress_data.code_size;

        compress_data.max_code  = 1 << compress_data.code_size;
        compress_data.code_mask = compress_data.max_code - 1;

        uint num_image_bytes = image_header->width*image_header->height;

        uchar *data = new uchar [file_size];

        uchar size;

        file->read(&size, 1);

        uint num_bytes_read = 0;

        while (size > 0) {
          while (size--) {
            file->read(&data[num_bytes_read], 1);

            ++num_bytes_read;
          }

          file->read(&size, 1);
        }

        if (num_bytes_read < file_size)
          memset(&data[num_bytes_read], 0, file_size - num_bytes_read);

        //------

        uchar *raw_data = new uchar [num_image_bytes];

        decompressData(data, num_bytes_read, raw_data, num_image_bytes);

        delete [] data;

        if (image_header->interlaced)
          deInterlace(raw_data, image_header);

        //------

        CGenImage *image = proto->dup();

        image->setType(CFILE_TYPE_IMAGE_GIF);

        image->setColormap(true);

        image->setDataSize(image_header->width, image_header->height);

        int bottom = gif_data->header->height - image_header->height - image_header->top;
        int right  = gif_data->header->width  - image_header->width  - image_header->left;

        //image->setBorder(image_header->left, bottom, right, image_header->top);
        if (bottom != 0 || right != 0) std::cerr << "Unhandled border" << std::endl;

        if (gif_data->num_local_colors > 0) {
          for (int i = 0; i < gif_data->num_local_colors; ++i) {
            CRGBA rgba;

            rgba.setRGBAI(gif_data->local_colors[i].r,
                          gif_data->local_colors[i].g,
                          gif_data->local_colors[i].b);

            image->addColor(rgba);
          }
        }
        else {
          for (int i = 0; i < gif_data->num_global_colors; ++i) {
            CRGBA rgba;

            rgba.setRGBAI(gif_data->global_colors[i].r,
                          gif_data->global_colors[i].g,
                          gif_data->global_colors[i].b);

            image->addColor(rgba);
          }

          //image->setBackground(image->getColor(gif_data->header->background));
        }

        //------

        if (transparent)
          image->setTransparentColor(transparent_color);

        //------

        for (int y = 0, k = 0; y < image_header->height; ++y)
          for (int x = 0; x < image_header->width; ++x, ++k)
            image->setColorIndex(x, y, raw_data[k]);

        delete [] raw_data;

        //------

        CGIFFrame *frame = new CGIFFrame(image);

        frame->setDelay(delay);
        frame->setDispose(dispose);
        frame->setUserInput(user_input);

        image_anim->add(frame);

        //------

        delay             = 0;
        transparent       = false;
        transparent_color = 0;
        dispose           = 0;
        user_input        = 0;

        //------

        delete image_header;

        image_header = NULL;
      }
      catch (...) {
        delete image_header;

        CTHROW("Failed to read GIF file");
      }
    }
    else if (id == CONTROL_ID) {
      if (CGIFImage::getDebug())
        std::cerr << "Control Id" << std::endl;

      try {
        uchar id1;

        file->read(&id1, 1);

        if (CGIFImage::getDebug())
          std::cerr << "Id = " << CStrUtil::toHexString(id1) << std::endl;

        uchar size;

        file->read(&size, 1);

        if (CGIFImage::getDebug())
          std::cerr << "Size = " << CStrUtil::toHexString(size) << std::endl;

        //if (id1 == CONTROL_APPEXT_ID) size = 11;

        if (size == 0)
          continue;

        file->read(compress_data.buffer, size);

        if (id1 == CONTROL_LABEL_ID) {
          if (CGIFImage::getDebug())
            std::cerr << "Graphics Control Extension" << std::endl;

          delay             = (compress_data.buffer[2] << 8) |
                              compress_data.buffer[1];
          transparent       = compress_data.buffer[0] & 0x01;
          transparent_color = compress_data.buffer[3];
          user_input        = ((compress_data.buffer[0] & 0x02) >> 1);
          dispose           = ((compress_data.buffer[0] & 0x1C) >> 2);

          if (CGIFImage::getDebug()) {
            std::cerr << "Delay       " << delay << std::endl;
            std::cerr << "Transparent " << transparent << " " << transparent_color << std::endl;
            std::cerr << "User Input  " << user_input << std::endl;
            std::cerr << "Dispose     " << dispose << std::endl;
          }
        }
        else if (id1 == CONTROL_APPEXT_ID) {
          if (CGIFImage::getDebug())
            std::cerr << "Application Extension" << std::endl;
        }
        else if (id1 == CONTROL_COMMENT_ID) {
          if (CGIFImage::getDebug())
            std::cerr << "Comment" << std::endl;

          uchar len = 0;

          file->read(&len, 1);

          while (len > 0) {
            uchar c;

            for (uint i = 0; i < len; ++i)
              file->read(&c, 1);

            file->read(&len, 1);
          }
        }
        else {
          std::cerr << "Unknown control block " << (int) id1 << std::endl;
        }

        // skip to block terminator
        while (true) {
          if (! file->read(&size, 1))
            break;

          if (size == 0)
            break;

          file->read(compress_data.buffer, size);
        }

        if (CGIFImage::getDebug())
          std::cerr << "@ " << file->getPos() << std::endl;
      }
      catch (...) {
        CTHROW("Failed to read GIF file");
      }
    }
    else if (id == TRAILER_ID) {
      if (CGIFImage::getDebug())
        std::cerr << "Trailer Id" << std::endl;

      break;
    }
    else if (id == 0) {
      uchar pad;

      file->read(&pad, 1);
    }
    else {
      std::cerr << "Invalid Id " << int(id) << " @ " << file->getPos() << std::endl;

      uchar c;

      file->read(&c, 1);

      while (c) {
        if (! file->read(&c, 1))
          break;
      }
    }
  }
}

bool
CGIFImage::
decompressData(uchar *in_data, int in_data_size,
               uchar *out_data, int out_data_size)
{
  int num_out_data = 0;

  uint bit_offset = 0;

  int byte_no = bit_offset/8;

  uint  last_code = 0;
  uchar last_byte = 0;

  while (byte_no < in_data_size) {
    uint code = readCode(&bit_offset, in_data);

    if (code == compress_data.eof_code)
      break;

    if (code == compress_data.clear_code) {
      compress_data.code_size = compress_data.init_code_size;

      compress_data.max_code  = 1 << compress_data.code_size;
      compress_data.code_mask = compress_data.max_code - 1;

      compress_data.free_code = compress_data.clear_code + 2;

      code = readCode(&bit_offset, in_data);

      last_code = code;

      last_byte = code & compress_data.bit_mask;

      if (num_out_data + 1 > out_data_size) {
        if (CGIFImage::getDebug())
          std::cerr << "Output Data Overflow !!!" << std::endl;

        break;
      }

      out_data[num_out_data++] = last_byte;
    }
    else {
      int num_out_bytes = 0;

      uint code1 = code;

      if (code >= compress_data.free_code) {
        code1 = last_code;

        compress_data.out_bytes[num_out_bytes++] = last_byte;
      }

      while (code1 > compress_data.bit_mask) {
        compress_data.out_bytes[num_out_bytes++] = compress_data.dictionary[code1].character;

        code1 = compress_data.dictionary[code1].parent_code;
      }

      last_byte = code1 & compress_data.bit_mask;

      compress_data.out_bytes[num_out_bytes++] = last_byte;

      if (num_out_data + num_out_bytes > out_data_size) {
        if (CGIFImage::getDebug())
          std::cerr << "Output Data Overflow !!!" << std::endl;

        break;
      }

      for (int i = num_out_bytes - 1; i >= 0; --i)
        out_data[num_out_data++] = compress_data.out_bytes[i] & compress_data.bit_mask;

      compress_data.dictionary[compress_data.free_code].parent_code = last_code;
      compress_data.dictionary[compress_data.free_code].character   = last_byte;

      last_code = code;

      ++compress_data.free_code;

      if (compress_data.free_code >= compress_data.max_code && compress_data.code_size < 12) {
        ++compress_data.code_size;

        compress_data.max_code  = 1 << compress_data.code_size;
        compress_data.code_mask = compress_data.max_code - 1;
      }
    }

    byte_no = bit_offset/8;
  }

  return true;
}

uint
CGIFImage::
readCode(uint *bit_offset, uchar *data)
{
  int byte_no = (*bit_offset)/8;

  int code = data[byte_no] & 0xFF;

  code |= (data[byte_no + 1] << 8) & 0xFF00;

  if (compress_data.code_size > 8)
    code |= (data[byte_no + 2] << 16) & 0xFF0000;

  code >>= (*bit_offset) % 8;

  code &= compress_data.code_mask;

  *bit_offset += compress_data.code_size;

  return code;
}

void
CGIFImage::
deInterlace(uchar *image, CGIFImageImageHeader *image_header)
{
  int image_size = image_header->width*image_header->height;

  uchar *image1 = new uchar [image_size];

  int i = 0;

  int j;

  for (j = 0; j < image_header->height; j += 8, ++i) {
    int i1 = i*image_header->width;
    int i2 = j*image_header->width;

    memcpy(&image1[i2], &image[i1], image_header->width);
  }

  for (j = 4; j < image_header->height; j += 8, ++i) {
    int i1 = i*image_header->width;
    int i2 = j*image_header->width;

    memcpy(&image1[i2], &image[i1], image_header->width);
  }

  for (j = 2; j < image_header->height; j += 4, ++i) {
    int i1 = i*image_header->width;
    int i2 = j*image_header->width;

    memcpy(&image1[i2], &image[i1], image_header->width);
  }

  for (j = 1; j < image_header->height; j += 2, ++i) {
    int i1 = i*image_header->width;
    int i2 = j*image_header->width;

    memcpy(&image1[i2], &image[i1], image_header->width);
  }

  memcpy(image, image1, image_size*sizeof(uchar));

  delete [] image1;
}

bool
CGIFImage::
write(CFile *file, CGenImage *image)
{
  memset(&compress_data, 0, sizeof(CGIFImageCompressData));

  if (! image->hasColormap()) {
    std::cerr << "GIF Image Depth greater than 8 not supported" << std::endl;
    return false;
  }

  writeChars(file, "GIF", 3);

  if (image->isTransparent())
    writeChars(file, "89a", 3);
  else
    writeChars(file, "87a", 3);

  writeHeader(file, image);

  writeGraphicsBlock(file, image, 0);

  writeByte(file, IMAGE_ID);

  int left = 0, top = 0;

  //image->getBorder(&left, &bottom, &right, &top);

  writeShort(file, left);
  writeShort(file, top );

  writeShort(file, image->getWidth ());
  writeShort(file, image->getHeight());

  uint color_table   = 0;
  uint color_bits    = 0;
  uint colors_sorted = 0;
  uint interlace     = 0;

  uint packed = 0;

  packed |= color_bits > 0 ? (color_bits - 1) << 5 : 0;
  packed |= colors_sorted << 2;
  packed |= interlace     << 1;
  packed |= color_table   << 0;

  writeByte(file, packed);

  compress_data.code_size = compress_data.color_table_bits;

  if (compress_data.code_size < 2)
    compress_data.code_size = 2;

  writeByte(file, compress_data.code_size);

  writeData(file, image);
  writeByte(file, 0);

  writeByte(file, TRAILER_ID);

  return true;
}

bool
CGIFImage::
writeAnim(CFile *file, const std::vector<CGenImage *> &images, int delay)
{
  if (images.empty())
    return false;

  writeChars(file, "GIF", 3);
  writeChars(file, "89a", 3);

  uint num_images = images.size();

  std::vector<CGenImage *> images1;

  for (uint i = 0; i < num_images; ++i) {
    CGenImage *image1 = images[i];

    if (! image1->hasColormap()) {
      std::cerr << "GIF Image Depth greater than 8 not supported" << std::endl;
      return false;
    }

    images1.push_back(image1);
  }

  CGenImage *image1 = images1[0];

  writeHeader(file, image1);

  int color_table_bits = compress_data.color_table_bits;

  for (uint i = 0; i < num_images; ++i) {
    memset(&compress_data, 0, sizeof(CGIFImageCompressData));

    compress_data.color_table_bits = color_table_bits;

    CGenImage *image1 = images1[i];

    writeGraphicsBlock(file, image1, delay);

    writeByte(file, IMAGE_ID);

    int left = 0, top = 0;

    //image1->getBorder(&left, &bottom, &right, &top);

    writeShort(file, left);
    writeShort(file, top );

    writeShort(file, image1->getWidth ());
    writeShort(file, image1->getHeight());

    uint color_table   = 0;
    uint color_bits    = 0;
    uint colors_sorted = 0;
    uint interlace     = 0;

    uint packed = 0;

    packed |= color_bits > 0 ? (color_bits - 1) << 5 : 0;
    packed |= colors_sorted << 2;
    packed |= interlace     << 1;
    packed |= color_table   << 0;

    writeByte(file, packed);

    compress_data.code_size = compress_data.color_table_bits;

    if (compress_data.code_size < 2)
      compress_data.code_size = 2;

    writeByte(file, compress_data.code_size);

    writeData(file, image1);
    writeByte(file, 0);
  }

  writeByte(file, TRAILER_ID);

  return true;
}

void
CGIFImage::
writeHeader(CFile *file, CGenImage *image)
{
  writeShort(file, image->getWidth() );
  writeShort(file, image->getHeight());

  uint color_table   = 1;
  uint color_bits    = 8;
  uint colors_sorted = 0;

  int i;

  for (i = 1; i < 8; ++i)
    if ((1<<i) >= int(image->getNumColors()))
      break;

  compress_data.color_table_bits = i;

  uint color_map_size = 1 << compress_data.color_table_bits;

  uint packed = 0;

  packed |=  color_table                         << 7;
  packed |= (color_bits - 1)                     << 4;
  packed |=  colors_sorted                       << 3;
  packed |= (compress_data.color_table_bits - 1) << 0;

  writeByte(file, packed);

  uint background = 0;

  writeByte(file, background);

  uint aspect_ratio = 0;

  writeByte(file, aspect_ratio);

  uint r, g, b, a;

  for (i = 0; i < (int) color_map_size; ++i) {
    if (i < int(image->getNumColors())) {
      const CRGBA &rgba = image->getColor(i);

      rgba.getRGBAI(&r, &g, &b, &a);

      writeByte(file, r);
      writeByte(file, g);
      writeByte(file, b);
    }
    else {
      writeByte(file, 0);
      writeByte(file, 0);
      writeByte(file, 0);
    }
  }
}

void
CGIFImage::
writeGraphicsBlock(CFile *file, CGenImage *image, int delay)
{
  int transparent_index = -1;

  if (image->isTransparent())
    transparent_index = image->getTransparentColor();

  writeByte (file, CONTROL_ID      );
  writeByte (file, CONTROL_LABEL_ID); // graphics control extension
  writeByte (file, 0x04            ); // length

  if (transparent_index >= 0) {
    writeByte (file, 0x01             ); // transparent flag
    writeShort(file, delay            ); // delay
    writeByte (file, transparent_index); // transparent index
  }
  else {
    writeByte (file, 0                ); // transparent flag
    writeShort(file, delay            ); // delay
    writeByte (file, 0                ); // transparent index
  }

  writeByte(file, 0); // EOF
}

void
CGIFImage::
writeData(CFile *file, CGenImage *image)
{
  compress_data.num_code_bytes = 0;

  compress_data.current_bit = 0;

  compress_data.max_code_size = 12;

  compress_data.init_code_size = compress_data.code_size + 1;

  compress_data.clear_code = 1 << compress_data.code_size;
  compress_data.eof_code   = compress_data.clear_code + 1;
  compress_data.free_code  = compress_data.clear_code + 2;

  ++compress_data.code_size;

  compress_data.max_code  = 1 << compress_data.code_size;
  compress_data.code_mask = compress_data.max_code - 1;

  clearDictionary();

  outputCode(file, compress_data.clear_code);

  int i = 0;

  uint code_value = (uint) image->getColorIndex(0, 0);

  ++i;

  int num_data = image->getWidth()*image->getHeight();

  while (i < num_data) {
    int x = i % image->getWidth();
    int y = i / image->getWidth();

    uint character = image->getColorIndex(x, y);

    ++i;

    uint ind = findChildCode(code_value, character);

    if (compress_data.dictionary[ind].code_value != UNUSED_CODE) {
      code_value = compress_data.dictionary[ind].code_value;

      continue;
    }

    if (compress_data.free_code <= compress_data.max_code) {
      compress_data.dictionary[ind].code_value  = compress_data.free_code++;
      compress_data.dictionary[ind].parent_code = code_value;
      compress_data.dictionary[ind].character   = character;
    }

    outputCode(file, code_value);

    code_value = character;

    if (compress_data.free_code > compress_data.max_code) {
      if (compress_data.code_size >= compress_data.max_code_size) {
        outputCode(file, compress_data.clear_code);

        clearDictionary();

        compress_data.code_size = compress_data.init_code_size - 1;

        compress_data.free_code = compress_data.clear_code + 2;
      }

      ++compress_data.code_size;

      compress_data.max_code  = 1 << compress_data.code_size;
      compress_data.code_mask = compress_data.max_code - 1;
    }
  }

  outputCode(file, code_value);
  outputCode(file, compress_data.eof_code);
}

uint
CGIFImage::
findChildCode(uint parent_code, uint character)
{
  int ind = (character << (compress_data.max_code_size - 8)) ^ parent_code;

  int offset;

  if (ind == 0)
    offset = 1;
  else
    offset = CIMAGE_GIF_DICT_SIZE - ind;

  while (true) {
    if (compress_data.dictionary[ind].code_value == UNUSED_CODE)
      return ind;

    if (compress_data.dictionary[ind].parent_code == parent_code &&
        compress_data.dictionary[ind].character   == character)
      return ind;

    ind -= offset;

    if (ind < 0)
      ind += CIMAGE_GIF_DICT_SIZE;
  }
}

void
CGIFImage::
clearDictionary()
{
  for (int i = 0; i < CIMAGE_GIF_DICT_SIZE; ++i)
    compress_data.dictionary[i].code_value = UNUSED_CODE;
}

void
CGIFImage::
outputCode(CFile *file, uint code)
{
  uint code1 = code & compress_data.code_mask;

  if     (compress_data.current_bit + compress_data.code_size > 16) {
    uchar byte1 = code1 << compress_data.current_bit;
    uchar byte2 = code1 >> (8  - compress_data.current_bit);
    uchar byte3 = code1 >> (16 - compress_data.current_bit);

    compress_data.current_byte |= byte1;

    writeCodeByte(file, compress_data.current_byte);

    compress_data.current_byte = byte2;

    writeCodeByte(file, compress_data.current_byte);

    compress_data.current_byte = byte3;

    compress_data.current_bit += compress_data.code_size - 16;
  }
  else if (compress_data.current_bit + compress_data.code_size > 8) {
    uchar byte1 = code1 << compress_data.current_bit;
    uchar byte2 = code1 >> (8 - compress_data.current_bit);

    compress_data.current_byte |= byte1;

    writeCodeByte(file, compress_data.current_byte);

    compress_data.current_byte = byte2;

    compress_data.current_bit += compress_data.code_size - 8;
  }
  else {
    uchar byte1 = code1 << compress_data.current_bit;

    compress_data.current_byte |= byte1;

    compress_data.current_bit += compress_data.code_size;
  }

  if (compress_data.current_bit == 8) {
    writeCodeByte(file, compress_data.current_byte);

    compress_data.current_bit  = 0;
    compress_data.current_byte = 0;
  }

  if (code == compress_data.eof_code) {
    if (compress_data.current_bit != 0)
      writeCodeByte(file, compress_data.current_byte);

    flushCodeBytes(file);
  }
}

void
CGIFImage::
writeCodeByte(CFile *file, int data)
{
  compress_data.code_bytes[compress_data.num_code_bytes++] = data;

  if (compress_data.num_code_bytes >= 254)
    flushCodeBytes(file);
}

void
CGIFImage::
flushCodeBytes(CFile *file)
{
  if (compress_data.num_code_bytes == 0)
    return;

  uchar code_byte = compress_data.num_code_bytes;

  file->write(&code_byte, 1);

  file->write(compress_data.code_bytes, compress_data.num_code_bytes);

  compress_data.num_code_bytes = 0;
}

void
CGIFImage::
writeChars(CFile *file, const char *chars, int len)
{
  file->write((uchar *) chars, len);
}

void
CGIFImage::
writeShort(CFile *file, int data)
{
  ushort s = data;

  uchar c[2];

  c[0] =  s       & 0xff;
  c[1] = (s >> 8) & 0xff;

  file->write(c, 2);
}

void
CGIFImage::
writeByte(CFile *file, int data)
{
  uchar b = data;

  file->write(&b, 1);
}

#ifndef CGIF_IMAGE_H
#define CGIF_IMAGE_H

#include <vector>
#include <iosfwd>
#include <sys/types.h>

#define CGIFImageInst CGIFImage::getInstance()

typedef unsigned char uchar;

struct CGIFImageDict {
  uint code_value;
  uint parent_code;
  uint character;
};

struct CGIFImageHeader;
struct CGIFImageImageHeader;
struct CGIFImageData;

class CGenImage;
class CGIFAnim;

class CGIFImage {
 public:
  static CGIFImage *getInstance() {
    static CGIFImage *instance;

    if (! instance)
      instance = new CGIFImage;

    return instance;
  }

 ~CGIFImage() { }

  CGIFImage(const CGIFImage &gif) = delete;
  CGIFImage &operator=(const CGIFImage &gif) = delete;

  static bool getDebug() { return false; }

  bool isGIF(std::istream &file) const;

  bool read(std::istream &file, CGenImage *image);

  bool readHeader(std::istream &file, CGenImage *image);

  bool write(std::ostream &os, CGenImage *image);

  static bool writeAnim(std::ostream &os, const std::vector<CGenImage *> &images, int delay=0);

 private:
  CGIFImage();

 public:
  static CGIFAnim *createAnim(std::istream &file, CGenImage *proto);

 private:
  static bool readHeader(std::istream &file, CGenImage *image, CGIFImageHeader *header);

  static void readGlobalColors(std::istream &file, CGIFImageData *gif_data);

  static bool readAnimData(std::istream &file, CGenImage *proto, CGIFAnim *image_anim,
                           CGIFImageData *gif_data);

  static bool decompressData(uchar *in_data, int in_data_size, uchar *out_data, int out_data_size);

  static uint readCode(uint *bit_offset, uchar *data);

  static void deInterlace(uchar *image_data, CGIFImageImageHeader *image_header);

  static void writeHeader(std::ostream &os, CGenImage *image);
  static void writeGraphicsBlock(std::ostream &os, CGenImage *image, int delay=0);
  static void writeData(std::ostream &os, CGenImage *image);

  static uint findChildCode(uint parent_code, uint character);

  static void clearDictionary();
  static void outputCode(std::ostream &os, uint code);
  static void writeCodeByte(std::ostream &os, int data);
  static void flushCodeBytes(std::ostream &os);
  static void writeChars(std::ostream &os, const char *chars, int len);
  static void writeShort(std::ostream &os, int data);
  static void writeByte(std::ostream &os, int data);
};

#endif

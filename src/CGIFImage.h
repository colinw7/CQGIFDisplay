#ifndef CGIF_IMAGE_H
#define CGIF_IMAGE_H

#include <vector>
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

class CFile;
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

  static bool getDebug() { return false; }

  bool read(CFile *file, CGenImage *image);

  bool readHeader(CFile *file, CGenImage *image);

  bool write(CFile *file, CGenImage *image);

  static bool writeAnim(CFile *file, const std::vector<CGenImage *> &images, int delay=0);

 private:
  CGIFImage();

 ~CGIFImage() { }

  CGIFImage(const CGIFImage &gif);

  const CGIFImage &operator=(const CGIFImage &gif);

 public:
  static CGIFAnim *createAnim(CFile *file, CGenImage *proto);

 private:
  static void readHeader(CFile *file, CGenImage *image, CGIFImageHeader *header);

  static void readGlobalColors(CFile *file, CGIFImageData *gif_data);

  static void readAnimData(CFile *file, CGenImage *proto, CGIFAnim *image_anim,
                           CGIFImageData *gif_data);

  static bool decompressData(uchar *in_data, int in_data_size, uchar *out_data, int out_data_size);

  static uint readCode(uint *bit_offset, uchar *data);

  static void deInterlace(uchar *image_data, CGIFImageImageHeader *image_header);

  static void writeHeader(CFile *file, CGenImage *image);
  static void writeGraphicsBlock(CFile *file, CGenImage *image, int delay=0);
  static void writeData(CFile *file, CGenImage *image);

  static uint findChildCode(uint parent_code, uint character);

  static void clearDictionary();
  static void outputCode(CFile *file, uint code);
  static void writeCodeByte(CFile *file, int data);
  static void flushCodeBytes(CFile *file);
  static void writeChars(CFile *file, const char *chars, int len);
  static void writeShort(CFile *file, int data);
  static void writeByte(CFile *file, int data);
};

#endif

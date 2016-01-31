#ifndef CJPG_IMAGE_H
#define CJPG_IMAGE_H

#define CJPGImageInst CJPGImage::getInstance()

class CFile;
class CGenImage;

class CJPGImage {
 public:
  static CJPGImage *getInstance() {
    static CJPGImage *instance;

    if (! instance)
      instance = new CJPGImage;

    return instance;
  }

  bool read(CFile *file, CGenImage *image);

  bool readHeader(CFile *file, CGenImage *image);

  bool write(CFile *file, CGenImage *image);

 private:
  CJPGImage();

 ~CJPGImage() { }

  CJPGImage(const CJPGImage &jpg);

  const CJPGImage &operator=(const CJPGImage &jpg);

  bool getDebug() const { return false; }

 private:
  int jpgProcessMarker(struct jpeg_decompress_struct *);
  int jpgGetC(struct jpeg_decompress_struct *);

  static void jpgErrorProc(struct jpeg_common_struct *);
  static void jpgMessageProc(struct jpeg_common_struct *);
};

#endif

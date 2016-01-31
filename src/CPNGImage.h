#ifndef CPNG_IMAGE_H
#define CPNG_IMAGE_H

#define CPNGImageInst CPNGImage::getInstance()

class CFile;
class CGenImage;

class CPNGImage {
 public:
  static CPNGImage *getInstance() {
    static CPNGImage *instance;

    if (! instance)
      instance = new CPNGImage;

    return instance;
  }

  bool read(CFile *file, CGenImage *image);

  bool readHeader(CFile *file, CGenImage *image);

  bool write(CFile *file, CGenImage *image);

 private:
  CPNGImage();

 ~CPNGImage() { }

  CPNGImage(const CPNGImage &png);

  const CPNGImage &operator=(const CPNGImage &png);
};

#endif

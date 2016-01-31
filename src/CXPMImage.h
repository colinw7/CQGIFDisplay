#ifndef CXPM_IMAGE_H
#define CXPM_IMAGE_H

#include <CRGBA.h>

#define CXPMImageInst CXPMImage::getInstance()

class  CFile;
class  CGenImage;
struct CXPMImageData;
struct CXPMImageColor;

class CXPMImage {
 public:
  static CXPMImage *getInstance() {
    static CXPMImage *instance;

    if (! instance)
      instance = new CXPMImage;

    return instance;
  }

  bool read(CFile *file, CGenImage *image);

  bool read(const char **strings, uint num_strings, CGenImage *image);

  bool readHeader(CFile *file, CGenImage *image);

  bool write(CFile *file, CGenImage *image);

  void setHotSpot(int x, int y);

 private:
  CXPMImage();

 ~CXPMImage() { }

  CXPMImage(const CXPMImage &xpm);

  const CXPMImage &operator=(const CXPMImage &xpm);

 private:
  bool readHeader(const char *data, int *i);

  bool skipDcl(const char *data, int *i);

  bool readValues(const char *data, int *i, CXPMImageData *xpm_data);
  bool readValuesString(CXPMImageData *xpm_data, const char *str);
  bool readColors(const char *data, int *i, CXPMImageData *xpm_data);
  bool readColorString(CXPMImageData *xpm_data, const char *str,
                       CXPMImageColor *color);

  void skipToColorKey(const char *str, int *i);

  bool isColorKey(const char *str);

  bool readData(const char *data, int *i, CXPMImageData *xpm_data);

  bool readDataString(CXPMImageData *xpm_data, const char *str,
                      uint *data, int *pos);
  bool readData24String(CXPMImageData *xpm_data, CRGBA *colors, const char *str,
                        uint *data, int *pos);

  CRGBA *createImageColors(CXPMImageData *xpm_data);

  bool skipComment(const char *data, int *i);

  char *readString(const char *data, int *i);

  bool lookupColor(const std::string &name, CRGBA &color);

  void getColorUsage(CGenImage *image, char **used, int *num_used);

  std::string pixelToSymbol(int pixel);
  std::string colorToString(const CRGBA &rgba);
  std::string colorToMonoString(const CRGBA &rgba);
};

#endif

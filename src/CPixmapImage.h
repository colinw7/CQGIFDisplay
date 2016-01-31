#ifndef CPixmapImage_H
#define CPixmapImage_H

#include <CGenImage.h>

class CPixmap;

class CPixmapImage : public CGenImage {
 public:
  CPixmapImage();

  CPixmapImage(const CPixmapImage &image);

  virtual ~CPixmapImage();

  virtual CPixmapImage *dup() const;

  virtual void assign(const CGenImage &image);

  virtual void setType(CFileType type);

  virtual void setSize(uint w, uint h);

  virtual void setDataSize(uint w, uint h);

  virtual uint getWidth () const;
  virtual uint getHeight() const;

  virtual CPixmap *getPixmap() const;

  virtual CPixmap *extractPixmap();

  virtual void setColormap(bool map);

  virtual bool hasColormap() const;

  virtual void addColor(const CRGBA &rgba);

  virtual const CRGBA &getColor(uint ind) const;

  virtual uint getNumColors() const;

  virtual void setColorIndex(uint x, uint y, uint ind);

  virtual void setPixel(uint x, uint y, uint ind);

  virtual uint getColorIndex(uint x, uint y) const;

  virtual uint getPixel(uint x, uint y) const;

  virtual bool isTransparent() const;

  virtual uint getTransparentColor() const;

  virtual void setTransparentColor(uint ind);

 private:
  CPixmap*           pixmap_;
  bool               colorMap_;
  std::vector<CRGBA> colors_;
  int                transparent_color_;
  CFileType          type_;
};

#endif

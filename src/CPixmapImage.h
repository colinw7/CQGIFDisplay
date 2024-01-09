#ifndef CPixmapImage_H
#define CPixmapImage_H

#include <CGenImage.h>

class CPixmap;

class CPixmapImage : public CGenImage {
 public:
  CPixmapImage();

  CPixmapImage(const CPixmapImage &image);

  virtual ~CPixmapImage();

  CPixmapImage *dup() const override;

  void assign(const CGenImage &image) override;

  void setType(Type type) override;

  void setSize(uint w, uint h) override;

  void setDataSize(uint w, uint h) override;

  uint getWidth () const override;
  uint getHeight() const override;

  virtual CPixmap *getPixmap() const;

  virtual CPixmap *extractPixmap();

  void setColormap(bool map) override;

  bool hasColormap() const override;

  void addColor(const RGBA &rgba) override;

  const RGBA &getColor(uint ind) const override;

  uint getNumColors() const override;

  void setColorIndexData(uint *data) override;

  void setColorIndex(uint x, uint y, uint ind) override;

  void setPixel(uint x, uint y, uint ind) override;

  uint getColorIndex(uint x, uint y) const override;

  uint getPixel(uint x, uint y) const override;

  bool isTransparent() const override;

  uint getTransparentColor() const override;

  void setTransparentColor(uint ind) override;

 private:
  CPixmap*          pixmap_   { nullptr };
  bool              colorMap_ { false };
  std::vector<RGBA> colors_;
  int               transparent_color_;
  Type              type_ { Type::NONE };
};

#endif

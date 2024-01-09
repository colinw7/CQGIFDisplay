#ifndef CGEN_IMAGE_H
#define CGEN_IMAGE_H

#include <vector>
#include <cstring>
#include <cassert>
#include <sys/types.h>

class CGenImage {
 public:
  enum class Type {
    NONE,
    GIF,
    JPG,
    PNG,
    BMP,
    XPM,
    XBM,
    PPM
  };

  struct RGBA {
    explicit RGBA() { }

    explicit RGBA(double r1, double g1, double b1, double a1=1.0) :
     r(r1), g(g1), b(b1), a(a1) {
    }

    static void decodeARGB(uint id, RGBA &rgba) {
      rgba.a = ((id >> 24) & 0xFF)/255.0;
      rgba.r = ((id >> 16) & 0xFF)/255.0;
      rgba.g = ((id >>  8) & 0xFF)/255.0;
      rgba.b = ((id      ) & 0xFF)/255.0;
    }

    static uint encodeARGB(const RGBA &rgba) {
      uint id = ((uint(rgba.a*255.0) & 0xFF) << 24) |
                ((uint(rgba.r*255.0) & 0xFF) << 16) |
                ((uint(rgba.g*255.0) & 0xFF) <<  8) |
                 (uint(rgba.b*255.0) & 0xFF);
      return id;
    }

    double getGray() const {
      constexpr double R_FACTOR = 0.3;
      constexpr double G_FACTOR = 0.59;
      constexpr double B_FACTOR = 0.11;

      return (R_FACTOR*r + G_FACTOR*g + B_FACTOR*b);
    }

    void setRGBAI(int r1, int g1, int b1, int a1=255) {
      r = std::min(std::max(r1/255.0, 0.0), 1.0);
      g = std::min(std::max(g1/255.0, 0.0), 1.0);
      b = std::min(std::max(b1/255.0, 0.0), 1.0);
      a = std::min(std::max(a1/255.0, 0.0), 1.0);
    }

    void getRGBAI(uint *r1, uint *g1, uint *b1, uint *a1) const {
      *r1 = std::min(std::max(int(r*255.0), 0), 255);
      *g1 = std::min(std::max(int(g*255.0), 0), 255);
      *b1 = std::min(std::max(int(b*255.0), 0), 255);
      *a1 = std::min(std::max(int(a*255.0), 0), 255);
    }

    uint getPixel() const {
      return encodeARGB(*this);
    }

    double r { 0.0 };
    double g { 0.0 };
    double b { 0.0 };
    double a { 1.0 };
  };

 public:
  CGenImage() { }

  CGenImage(const CGenImage &) { }

  virtual ~CGenImage() { }

  virtual CGenImage *dup() const = 0;

  virtual void assign(const CGenImage &image) = 0;

  virtual void setType(Type type) = 0;

  virtual void setSize(uint w, uint h) = 0;

  virtual void setDataSize(uint w, uint h) = 0;

  virtual uint getWidth () const = 0;
  virtual uint getHeight() const = 0;

  virtual void setColormap(bool map) = 0;

  virtual bool hasColormap() const = 0;

  virtual void addColor(const RGBA &rgba) = 0;

  virtual const RGBA &getColor(uint ind) const = 0;

  virtual uint getNumColors() const = 0;

  virtual void setColorIndexData(uint *data) = 0;

  virtual void setColorIndex(uint x, uint y, uint ind) = 0;

  virtual void setPixel(uint x, uint y, uint ind) = 0;

  virtual uint getColorIndex(uint ind) const {
    uint x = ind / getWidth();
    uint y = ind % getWidth();

    return getColorIndex(x, y);
  }

  virtual uint getColorIndex(uint x, uint y) const = 0;

  virtual uint getPixel(uint x, uint y) const = 0;

  virtual bool isTransparent() const = 0;

  virtual uint getTransparentColor() const = 0;

  virtual void setTransparentColor(uint ind) = 0;

 private:
  CGenImage &operator=(const CGenImage &image);
};

class CImageData : public CGenImage {
 public:
  CImageData() :
   CGenImage() {
  }

  CImageData(const CImageData &image) :
   CGenImage(image),
   width_   (image.width_),
   height_  (image.height_),
   colorMap_(image.colorMap_),
   colors_  (image.colors_),
   type_    (image.type_) {
    data_ = new uint [width_*height_];

    memcpy(data_, image.data_, width_*height_*sizeof(uint));
  }

  virtual ~CImageData() {
    delete [] data_;
  }

  CImageData &operator=(const CImageData &image) {
    width_    = image.width_;
    height_   = image.height_;
    colorMap_ = image.colorMap_;
    colors_   = image.colors_;
    type_     = image.type_;

    data_ = new uint [width_*height_];

    memcpy(data_, image.data_, width_*height_*sizeof(uint));

    return *this;
  }

  CImageData *dup() const override {
    return new CImageData(*this);
  }

  void assign(const CGenImage &image) override {
    const CImageData *data = dynamic_cast<const CImageData *>(&image);

    assert(data);

    width_    = data->width_;
    height_   = data->height_;
    colorMap_ = data->colorMap_;
    colors_   = data->colors_;
    type_     = data->type_;

    delete [] data_;

    data_ = new uint [width_*height_];

    memcpy(data_, data->data_, width_*height_*sizeof(uint));
  }

  const Type &getType() const { return type_; }
  void setType(Type type) override { type_ = type; }

  void setSize(uint w, uint h) override {
    if (w == width_ && h == height_) return;

    width_  = w;
    height_ = h;

    delete [] data_;

    data_ = NULL;
  }

  void setDataSize(uint w, uint h) override {
    if (w != width_ || h != height_ || data_ == NULL) {
      width_  = w;
      height_ = h;

      delete [] data_;

      data_ = new uint [width_*height_];
    }
  }

  uint getWidth () const override { return width_ ; }
  uint getHeight() const override { return height_; }

  void setColormap(bool map) override { colorMap_ = map; }
  bool hasColormap() const override { return colorMap_; }

  void addColor(const RGBA &rgba) override {
    assert(hasColormap());

    colors_.push_back(rgba);
  }

  const RGBA &getColor(uint ind) const override {
    assert(hasColormap() && ind < colors_.size());

    return colors_[ind];
  }

  uint getNumColors() const override { return uint(colors_.size()); }

  void setColorIndexData(uint *data) override {
    uint *p1 = data;
    uint *p2 = data_;

    for (uint y = 0; y < getHeight(); ++y)
      for (uint x = 0; x < getWidth(); ++x, ++p1, ++p2)
        *p2 = *p1;
  }

  void setColorIndex(uint x, uint y, uint ind) override {
    assert(x < getWidth() && y < getHeight() && hasColormap() && ind < colors_.size());

    data_[y*getWidth() + x] = ind;
  }

  void setPixel(uint x, uint y, uint ind) override {
    assert(x < getWidth() && y < getHeight() && ! hasColormap());

    data_[y*getWidth() + x] = ind;
  }

  uint getColorIndex(uint x, uint y) const override {
    assert(x < getWidth() && y < getHeight() && hasColormap());

    return data_[y*getWidth() + x];
  }

  uint getPixel(uint x, uint y) const override {
    assert(x < getWidth() && y < getHeight() && ! hasColormap());

    return data_[y*getWidth() + x];
  }

  bool isTransparent() const override {
    assert(hasColormap());

    return (transparentColor_ >= 0);
  }

  uint getTransparentColor() const override {
    assert(hasColormap() && transparentColor_ >= 0);

    return uint(transparentColor_);
  }

  void setTransparentColor(uint ind) override {
    assert(hasColormap() && ind < colors_.size());

    transparentColor_ = int(ind);
  }

 private:
  using Colors = std::vector<RGBA>;

  uint   width_            { 0 };
  uint   height_           { 0 };
  bool   colorMap_         { false };
  Colors colors_;
  int    transparentColor_ { 0 };
  uint*  data_             { nullptr };
  Type   type_             { Type::NONE };
};

#endif

#ifndef CGEN_IMAGE_H
#define CGEN_IMAGE_H

#include <CRGBA.h>
#include <CFileType.h>
#include <vector>
#include <cstring>

class CGenImage {
 public:
  CGenImage() { }

  virtual ~CGenImage() { }

  virtual CGenImage *dup() const = 0;

  virtual void assign(const CGenImage &image) = 0;

  virtual void setType(CFileType type) = 0;

  virtual void setSize(uint w, uint h) = 0;

  virtual void setDataSize(uint w, uint h) = 0;

  virtual uint getWidth () const = 0;
  virtual uint getHeight() const = 0;

  virtual void setColormap(bool map) = 0;

  virtual bool hasColormap() const = 0;

  virtual void addColor(const CRGBA &rgba) = 0;

  virtual const CRGBA &getColor(uint ind) const = 0;

  virtual uint getNumColors() const = 0;

  virtual void setColorIndex(uint x, uint y, uint ind) = 0;

  virtual void setPixel(uint x, uint y, uint ind) = 0;

  virtual uint getColorIndex(uint ind) const {
    int x = ind / getWidth();
    int y = ind % getWidth();

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

  CImageData *dup() const {
    return new CImageData(*this);
  }

  void assign(const CGenImage &image) {
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

  virtual void setType(CFileType type) {
    type_ = type;
  }

  virtual void setSize(uint w, uint h) {
    if (w == width_ && h == height_) return;

    width_  = w;
    height_ = h;

    delete [] data_;

    data_ = NULL;
  }

  virtual void setDataSize(uint w, uint h) {
    if (w != width_ || h != height_ || data_ == NULL) {
      width_  = w;
      height_ = h;

      delete [] data_;

      data_ = new uint [width_*height_];
    }
  }

  virtual uint getWidth () const { return width_ ; }
  virtual uint getHeight() const { return height_; }

  virtual void setColormap(bool map) {
    colorMap_ = map;
  }

  virtual bool hasColormap() const {
    return colorMap_;
  }

  virtual void addColor(const CRGBA &rgba) {
    assert(colorMap_);

    colors_.push_back(rgba);
  }

  virtual const CRGBA &getColor(uint ind) const {
    assert(colorMap_ && ind < colors_.size());

    return colors_[ind];
  }

  virtual uint getNumColors() const { return colors_.size(); }

  virtual void setColorIndex(uint x, uint y, uint ind) {
    assert(x < width_ && y < height_ && colorMap_ && ind < colors_.size());

    data_[y*width_ + x] = ind;
  }

  virtual void setPixel(uint x, uint y, uint ind) {
    assert(x < width_ && y < height_ && ! colorMap_);

    data_[y*width_ + x] = ind;
  }

  virtual uint getColorIndex(uint x, uint y) const {
    assert(x < width_ && y < height_ && colorMap_);

    return data_[y*width_ + x];
  }

  virtual uint getPixel(uint x, uint y) const {
    assert(x < width_ && y < height_ && ! colorMap_);

    return data_[y*width_ + x];
  }

  virtual bool isTransparent() const {
    assert(colorMap_);

    return (transparent_color_ >= 0);
  }

  virtual uint getTransparentColor() const {
    assert(colorMap_ && transparent_color_ >= 0);

    return uint(transparent_color_);
  }

  virtual void setTransparentColor(uint ind) {
    assert(colorMap_ && ind < colors_.size());

    transparent_color_ = ind;
  }

 private:
  uint               width_    { 0 };
  uint               height_   { 0 };
  bool               colorMap_ { false };
  std::vector<CRGBA> colors_;
  int                transparent_color_;
  uint*              data_     { nullptr };
  CFileType          type_     { CFILE_TYPE_NONE };
};

#endif

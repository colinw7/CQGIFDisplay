#ifndef CPixmap_H
#define CPixmap_H

#include <string>
#include <sys/types.h>

class CPixmap {
 public:
  class Pixel {
   public:
    Pixel() :
     pixel_(0) {
    }

    Pixel(uint r, uint g, uint b, uint a = 255) :
     pixel_(rgbaToPixel(r, g, b, a)) {
    }

    Pixel(uint pixel) :
     pixel_(pixel) {
    }

    uint getValue() const { return pixel_; }

    static uint rgbaToPixel(uint r, uint g, uint b, uint a) {
      return (((a & 0xff) << 24) | ((r & 0xff) << 16) |
              ((g & 0xff) <<  8) | ((b & 0xff) <<  0));
    }

   private:
    uint pixel_;
  };

  CPixmap(uint w, uint h);

  CPixmap(const CPixmap &pixmap);

  virtual ~CPixmap();

  CPixmap &operator=(const CPixmap &pixmap);

  virtual CPixmap *dup() const;

  virtual uint getWidth () const { return w_; }
  virtual uint getHeight() const { return h_; }

  virtual Pixel getPixel(uint x, uint y) const;

  virtual void setPixel(uint x, uint y, const Pixel &pixel);

  virtual int getScale() const { return scale_; }

  virtual void setScale(int scale);

 protected:
  uint   w_, h_;
  Pixel *data_;
  int    scale_;
};

class CPixmapFactory {
 public:
  CPixmapFactory() { }

  virtual ~CPixmapFactory() { }

  virtual CPixmap *createPixmap(uint w, uint h) = 0;
};

#define CPixmapMgrInst CPixmapMgr::getInstance()

class CPixmapMgr {
 public:
  static CPixmapMgr *getInstance();

 ~CPixmapMgr();

  void setFactory(CPixmapFactory *factory) {
    factory_ = factory;
  }

  CPixmap *createPixmap(uint w, uint h) const;

  CPixmap *read(const std::string &filename);

 private:
  CPixmapMgr();

 private:
  CPixmapFactory *factory_;
};

#endif

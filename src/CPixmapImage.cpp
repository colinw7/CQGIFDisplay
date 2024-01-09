#include <CPixmapImage.h>
#include <CPixmap.h>

CPixmapImage::
CPixmapImage() :
 CGenImage()
{
}

CPixmapImage::
CPixmapImage(const CPixmapImage &pixmap) :
 CGenImage(pixmap),
 colorMap_(pixmap.colorMap_),
 colors_  (pixmap.colors_),
 type_    (pixmap.type_)
{
  pixmap_ = CPixmapMgrInst->createPixmap(pixmap.getWidth(), pixmap.getHeight());

  for (uint y = 0; y < pixmap.getHeight(); ++y) {
    for (uint x = 0; x < pixmap.getWidth(); ++x) {
      pixmap_->setPixel(x, y, pixmap.pixmap_->getPixel(x, y));
    }
  }
}

CPixmapImage::
~CPixmapImage()
{
  delete pixmap_;
}

CPixmapImage *
CPixmapImage::
dup() const
{
  return new CPixmapImage(*this);
}

void
CPixmapImage::
assign(const CGenImage &image)
{
  const CPixmapImage *pixmap = dynamic_cast<const CPixmapImage *>(&image);

  assert(pixmap);

  colorMap_ = pixmap->colorMap_;
  colors_   = pixmap->colors_;
  type_     = pixmap->type_;

  delete pixmap_;

  pixmap_ = CPixmapMgrInst->createPixmap(pixmap->getWidth(), pixmap->getHeight());

  for (uint y = 0; y < pixmap_->getHeight(); ++y) {
    for (uint x = 0; x < pixmap_->getWidth(); ++x) {
      pixmap_->setPixel(x, y, pixmap->pixmap_->getPixel(x, y));
    }
  }
}

void
CPixmapImage::
setType(Type type)
{
  type_ = type;
}

void
CPixmapImage::
setSize(uint w, uint h)
{
  setDataSize(w, h);
}

void
CPixmapImage::
setDataSize(uint w, uint h)
{
  delete pixmap_;

  pixmap_ = CPixmapMgrInst->createPixmap(w, h);
}

uint
CPixmapImage::
getWidth () const
{
  return (pixmap_ ? pixmap_->getWidth () : 0);
}

uint
CPixmapImage::
getHeight() const
{
  return (pixmap_ ? pixmap_->getHeight() : 0);
}

CPixmap *
CPixmapImage::
getPixmap() const
{
  return pixmap_;
}

CPixmap *
CPixmapImage::
extractPixmap()
{
  CPixmap *pixmap = pixmap_;

  pixmap_ = NULL;

  return pixmap;
}

void
CPixmapImage::
setColormap(bool map)
{
  colorMap_ = map;
}

bool
CPixmapImage::
hasColormap() const
{
  return colorMap_;
}

void
CPixmapImage::
addColor(const RGBA &rgba)
{
  assert(colorMap_);

  colors_.push_back(rgba);
}

const CPixmapImage::RGBA &
CPixmapImage::
getColor(uint ind) const
{
  assert(colorMap_ && ind < colors_.size());

  return colors_[ind];
}

uint
CPixmapImage::
getNumColors() const
{
  return uint(colors_.size());
}

void
CPixmapImage::
setColorIndex(uint x, uint y, uint ind)
{
  assert(colorMap_ && ind < colors_.size());

  const auto &rgba = colors_[ind];

  pixmap_->setPixel(x, y, rgba.getPixel());
}

void
CPixmapImage::
setColorIndexData(uint *)
{
  assert(false);
}

void
CPixmapImage::
setPixel(uint x, uint y, uint ind)
{
  assert(! colorMap_);

  pixmap_->setPixel(x, y, CPixmap::Pixel(ind));
}

uint
CPixmapImage::
getColorIndex(uint x, uint y) const
{
  assert(colorMap_);

  return pixmap_->getPixel(x, y).getValue();
}

uint
CPixmapImage::
getPixel(uint x, uint y) const
{
  assert(! colorMap_);

  return pixmap_->getPixel(x, y).getValue();
}

bool
CPixmapImage::
isTransparent() const
{
  assert(colorMap_);

  return (transparent_color_ >= 0);
}

uint
CPixmapImage::
getTransparentColor() const
{
  assert(colorMap_ && transparent_color_ >= 0);

  return uint(transparent_color_);
}

void
CPixmapImage::
setTransparentColor(uint ind)
{
  assert(colorMap_ && ind < colors_.size());

  transparent_color_ = ind;
}

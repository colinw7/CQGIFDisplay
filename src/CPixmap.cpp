#include <CPixmap.h>
#include <CPixmapImage.h>
#include <CGIFImage.h>
#include <CJPGImage.h>
#include <CPNGImage.h>
#include <CXPMImage.h>

#include <CFileUtil.h>
#include <CFile.h>

#include <fstream>

CPixmap::
CPixmap(uint w, uint h) :
 w_(w), h_(h), data_(nullptr), scale_(1)
{
  data_ = new Pixel [w_*h_];
}

CPixmap::
CPixmap(const CPixmap &pixmap) :
 w_(pixmap.w_), h_(pixmap.h_), data_(nullptr), scale_(pixmap.scale_)
{
  data_ = new Pixel [w_*h_];
}

CPixmap::
~CPixmap()
{
  delete [] data_;
}

CPixmap &
CPixmap::
operator=(const CPixmap &pixmap)
{
  assert(w_ == pixmap.w_ && h_ == pixmap.h_);

  memcpy(data_, pixmap.data_, w_*h_*sizeof(Pixel));

  return *this;
}

CPixmap *
CPixmap::
dup() const
{
  return new CPixmap(*this);
}

CPixmap::Pixel
CPixmap::
getPixel(uint x, uint y) const
{
  assert(x < w_ && y < h_);

  return data_[y*w_ + x];
}

void
CPixmap::
setPixel(uint x, uint y, const Pixel &pixel)
{
  assert(x < w_ && y < h_);

  data_[y*w_ + x] = pixel;
}

void
CPixmap::
setScale(int scale)
{
  scale_ = scale;
}

//------

CPixmapMgr *
CPixmapMgr::
getInstance()
{
  static CPixmapMgr *instance;

  if (! instance)
    instance = new CPixmapMgr;

  return instance;
}

CPixmapMgr::
CPixmapMgr() :
 factory_(nullptr)
{
}

CPixmapMgr::
~CPixmapMgr()
{
}

CPixmap *
CPixmapMgr::
createPixmap(uint w, uint h) const
{
  if (! factory_)
    return new CPixmap(w, h);
  else
    return factory_->createPixmap(w, h);
}

CPixmap *
CPixmapMgr::
read(const std::string &filename)
{
  CFile file(filename);

  auto type = CFileUtil::getType(&file);
  if (! (type & CFILE_TYPE_IMAGE))
    return nullptr;

  CPixmapImage pixmapImage;

  if      (type == CFILE_TYPE_IMAGE_GIF) {
    auto ifile = std::ifstream(filename, std::ios::in);

    if (CGIFImageInst->read(ifile, &pixmapImage))
      return pixmapImage.extractPixmap();
  }
  else if (type == CFILE_TYPE_IMAGE_JPG) {
    if (CJPGImageInst->read(&file, &pixmapImage))
      return pixmapImage.extractPixmap();
  }
  else if (type == CFILE_TYPE_IMAGE_PNG) {
    if (CPNGImageInst->read(&file, &pixmapImage))
      return pixmapImage.extractPixmap();
  }
  else if (type == CFILE_TYPE_IMAGE_XPM) {
    if (CXPMImageInst->read(&file, &pixmapImage))
      return pixmapImage.extractPixmap();
  }

  return nullptr;
}

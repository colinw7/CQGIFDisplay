#include <CXPMImage.h>
#include <CGenImage.h>
#include <CRGBName.h>
#include <CFile.h>
#include <CStrUtil.h>
#include <CThrow.h>
#include <cstring>

static const std::string xpm_pixel_chars_ =
 ".#abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

static int xpm_chars_per_pixel_;
static int xpm_x_hot_;
static int xpm_y_hot_;

struct CXPMImageColor {
  std::string name;
  std::string mono;
  std::string symb;
  std::string grey4;
  std::string grey;
  std::string color;

  CXPMImageColor() :
   name (""),
   mono (""),
   symb (""),
   grey4(""),
   grey (""),
   color("") {
  }
};

struct CXPMImageData {
  int             width;
  int             height;
  int             num_colors;
  CXPMImageColor *colors;
  bool            transparent;
  uint            transparent_color;
  int             chars_per_pixel;
  int             x_hot;
  int             y_hot;
  bool            extension;
  uint           *data;

  CXPMImageData() :
   width            (0),
   height           (0),
   num_colors       (0),
   colors           (nullptr),
   transparent      (false),
   transparent_color(0),
   chars_per_pixel  (0),
   x_hot            (0),
   y_hot            (0),
   extension        (false),
   data             (nullptr) {
  }
};

//----------------------

void
CXPMImage::
setHotSpot(int x, int y)
{
  xpm_x_hot_ = x;
  xpm_y_hot_ = y;
}

//----------------------

CXPMImage::
CXPMImage()
{
}

bool
CXPMImage::
read(CFile *file, CGenImage *image)
{
  CXPMImageData xpm_data;

  CFileData *file_data = nullptr;

  try {
    file->rewind();

    file_data = file->readAll();

    if (! file_data) {
      CTHROW("Failed to read file");
      return false;
    }

    auto *data = reinterpret_cast<char *>(file_data->getData());

    //------

    int i = 0;

    //------

    if (! readHeader(data, &i)) {
      CTHROW("Failed to read header");
      return false;
    }

    //------

    if (! skipDcl(data, &i)) {
      CTHROW("Failed to read declarations");
      return false;
    }

    //------

    if (! readValues(data, &i, &xpm_data)) {
      CTHROW("Failed to read values");
      return false;
    }

    //------

    if (! readColors(data, &i, &xpm_data)) {
      CTHROW("Failed to read colors");
      return false;
    }

    //------

    if (! readData(data, &i, &xpm_data)) {
      CTHROW("Failed to read data");
      return false;
    }

    //------

    auto *colors = createImageColors(&xpm_data);

    //------

    image->setType(CGenImage::Type::XPM);

    image->setDataSize(uint(xpm_data.width), uint(xpm_data.height));

    if (xpm_data.num_colors <= 256) {
      image->setColormap(true);

      for (int ic = 0; ic < xpm_data.num_colors; ++ic) {
        const auto &c = colors[ic];

        CGenImage::RGBA rgba(c.getRed(), c.getGreen(), c.getBlue(), c.getAlpha());

        image->addColor(rgba);
      }

      uint *p = xpm_data.data;

      for (int y = 0; y < xpm_data.height; ++y)
        for (int x = 0; x < xpm_data.width; ++x, ++p)
          image->setColorIndex(uint(x), uint(y), *p);

      if (xpm_data.transparent)
        image->setTransparentColor(xpm_data.transparent_color);
    }
    else {
      uint *p = xpm_data.data;

      for (int y = 0; y < xpm_data.height; ++y)
        for (int x = 0; x < xpm_data.width; ++x, ++p)
          image->setPixel(uint(x), uint(y), *p);
    }

    delete [] xpm_data.data;

    //------

    delete [] colors;
    delete [] xpm_data.colors;

    delete file_data;

    //------

    return true;
  }
  catch (...) {
    delete [] xpm_data.colors;

    delete file_data;

    return false;
  }
}

bool
CXPMImage::
read(const char **strings, uint num_strings, CGenImage *image)
{
  if (num_strings <= 0)
    return false;

  //------

  CXPMImageData xpm_data;

  try {
    int i   = 0;
    int pos = 0;

    //------

    if (! readValuesString(&xpm_data, strings[i])) {
      CTHROW("Failed to read values");
      return false;
    }

    ++i;

    //------

    int j;

    xpm_data.colors = new CXPMImageColor [size_t(xpm_data.num_colors)];

    for (j = 0; j < xpm_data.num_colors; ++j) {
      if (i >= int(num_strings)) {
        CTHROW("Failed to read colors");
        return false;
      }

      if (! readColorString(&xpm_data, strings[i], &xpm_data.colors[j])) {
        CTHROW("Failed to read colors");
        return false;
      }

      ++i;
    }

    //------

    auto *colors = createImageColors(&xpm_data);

    xpm_data.data = new uint [size_t(xpm_data.width*xpm_data.height)];

    for (j = 0; j < xpm_data.height; ++j) {
      if (i >= int(num_strings)) {
        CTHROW("Failed to read data");
        return false;
      }

      if (xpm_data.num_colors <= 256) {
        if (! readDataString(&xpm_data, strings[i], xpm_data.data, &pos)) {
          CTHROW("Failed to read data");
          return false;
        }
      }
      else {
        if (! readData24String(&xpm_data, colors, strings[i], xpm_data.data, &pos)) {
          CTHROW("Failed to read data");
          return false;
        }
      }

      ++i;
    }

    //------

    if (xpm_data.num_colors > 256) {
      if (xpm_data.transparent) {
        const CRGBA &color = colors[xpm_data.transparent_color];

        xpm_data.transparent_color = color.getId();
      }
    }

    //------

    image->setType(CGenImage::Type::XPM);

    image->setDataSize(uint(xpm_data.width), uint(xpm_data.height));

    if (xpm_data.num_colors <= 256) {
      image->setColormap(true);

      for (int ic = 0; ic < xpm_data.num_colors; ++ic) {
        const auto &c = colors[ic];

        CGenImage::RGBA rgba(c.getRed(), c.getGreen(), c.getBlue(), c.getAlpha());

        image->addColor(rgba);
      }

      uint *p = xpm_data.data;

      for (int y = 0; y < xpm_data.height; ++y)
        for (int x = 0; x < xpm_data.width; ++x, ++p)
          image->setColorIndex(uint(x), uint(y), *p);

      if (xpm_data.transparent)
        image->setTransparentColor(xpm_data.transparent_color);
    }
    else {
      uint *p = xpm_data.data;

      for (int y = 0; y < xpm_data.height; ++y)
        for (int x = 0; x < xpm_data.width; ++x, ++p)
          image->setPixel(uint(x), uint(y), *p);
    }

    delete [] xpm_data.data;

    //------

    delete [] colors;
    delete [] xpm_data.colors;

    //------

    return true;
  }
  catch (...) {
    delete [] xpm_data.colors;

    return false;
  }
}

bool
CXPMImage::
readHeader(CFile *file, CGenImage *image)
{
  CFileData *file_data = nullptr;

  file->rewind();

  try {
    CXPMImageData xpm_data;

    file_data = file->readAll();

    char *data = reinterpret_cast<char *>(file_data->getData());

    //------

    int i = 0;

    //------

    if (! readHeader(data, &i)) {
      CTHROW("Failed to read header");
      return false;
    }

    if (! skipDcl(data, &i)) {
      CTHROW("Failed to read declarations");
      return false;
    }

    //------

    if (! readValues(data, &i, &xpm_data)) {
      CTHROW("Failed to read values");
      return false;
    }

    //------

    image->setType(CGenImage::Type::XPM);

    image->setSize(uint(xpm_data.width), uint(xpm_data.height));
  }
  catch (...) {
    CTHROW("Failed to read XPM file");
    return false;
  }

  delete file_data;

  return true;
}

bool
CXPMImage::
readHeader(const char *data, int *i)
{
  CStrUtil::skipSpace(data, i);

  if (data[*i] != '/' || data[*i + 1] != '*')
    return false;

  *i += 2;

  CStrUtil::skipSpace(data, i);

  if (data[*i] != 'X' || data[*i + 1] != 'P' || data[*i + 2] != 'M')
    return false;

  *i += 3;

  CStrUtil::skipSpace(data, i);

  if (data[*i] != '*' || data[*i + 1] != '/')
    return false;

  *i += 2;

  CStrUtil::skipSpace(data, i);

  return true;
}

bool
CXPMImage::
skipDcl(const char *data, int *i)
{
  while (skipComment(data, i))
    ;

  //------

  /* Skip to Brace */

  while (data[*i] != '\0' && data[*i] != '{')
    (*i)++;

  if (data[*i] == '{')
    (*i)++;

  //------

  return true;
}

bool
CXPMImage::
readValues(const char *data, int *i, CXPMImageData *xpm_data)
{
  while (skipComment(data, i))
    ;

  //------

  // skip to start of string
  while (data[*i] != '\0' && data[*i] != '\"')
    (*i)++;

  //------

  char *str = readString(data, i);

  if (str == nullptr)
    return false;

  //------

  bool flag = readValuesString(xpm_data, str);

  //------

  delete [] str;

  //------

  return flag;
}

bool
CXPMImage::
readValuesString(CXPMImageData *xpm_data, const char *str)
{
  uint i = 0;

  //------

  CStrUtil::skipSpace(str, &i);

  if (! CStrUtil::readInteger(str, &i, &xpm_data->width)) {
    CTHROW("Failed to read width");
    return false;
  }

  //------

  CStrUtil::skipSpace(str, &i);

  if (! CStrUtil::readInteger(str, &i, &xpm_data->height)) {
    CTHROW("Failed to read height");
    return false;
  }

  //------

  CStrUtil::skipSpace(str, &i);

  if (! CStrUtil::readInteger(str, &i, &xpm_data->num_colors)) {
    CTHROW("Failed to read num colors");
    return false;
  }

  //------

  CStrUtil::skipSpace(str, &i);

  if (! CStrUtil::readInteger(str, &i, &xpm_data->chars_per_pixel)) {
    CTHROW("Failed to read chars per pixel");
    return false;
  }

  //------

  CStrUtil::skipSpace(str, &i);

  if (! CStrUtil::readInteger(str, &i, &xpm_data->x_hot))
    xpm_data->x_hot = 0;

  //------

  CStrUtil::skipSpace(str, &i);

  if (! CStrUtil::readInteger(str, &i, &xpm_data->y_hot))
    xpm_data->y_hot = 0;

  //------

  CStrUtil::skipSpace(str, &i);

  if (strncmp(&str[i], "XPMEXT", 6) == 0)
    xpm_data->extension = true;

  return true;
}

bool
CXPMImage::
readColors(const char *data, int *i, CXPMImageData *xpm_data)
{
  xpm_data->colors = new CXPMImageColor [size_t(xpm_data->num_colors)];

  //------

  while (skipComment(data, i))
    ;

  //------

  for (int j = 0; j < xpm_data->num_colors; ++j) {
    // skip to start of string
    while (data[*i] != '\0' && data[*i] != '\"')
      (*i)++;

    //------

    char *str = readString(data, i);

    if (str == nullptr)
      return false;

    //------

    bool flag = readColorString(xpm_data, str, &xpm_data->colors[j]);

    if (! flag) {
      delete [] str;

      return false;
    }

    //------

    delete [] str;
  }

  //------

  return true;
}

bool
CXPMImage::
readColorString(CXPMImageData *xpm_data, const char *str, CXPMImageColor *color)
{
  int i = 0;

  //------

  color->name = std::string(&str[i], size_t(xpm_data->chars_per_pixel));

  i += xpm_data->chars_per_pixel;

  //------

  CStrUtil::skipSpace(str, &i);

  while (str[i] != '\0') {
    if      (str[i] == 'm' && isspace(str[i + 1])) {
      i += 2;

      CStrUtil::skipSpace(str, &i);

      int j = i;

      skipToColorKey(str, &i);

      color->mono = CStrUtil::stripSpaces(std::string(&str[j], size_t(i - j)));
    }
    else if (str[i] == 's' && isspace(str[i + 1])) {
      i += 2;

      CStrUtil::skipSpace(str, &i);

      int j = i;

      skipToColorKey(str, &i);

      color->symb = CStrUtil::stripSpaces(std::string(&str[j], size_t(i - j)));
    }
    else if (str[i] == 'g' && str[i + 1] == '4' && isspace(str[i + 2])) {
      i += 2;

      CStrUtil::skipSpace(str, &i);

      int j = i;

      skipToColorKey(str, &i);

      color->grey4 = CStrUtil::stripSpaces(std::string(&str[j], size_t(i - j)));
    }
    else if (str[i] == 'g' && isspace(str[i + 1])) {
      i += 2;

      CStrUtil::skipSpace(str, &i);

      int j = i;

      skipToColorKey(str, &i);

      color->grey = CStrUtil::stripSpaces(std::string(&str[j], size_t(i - j)));
    }
    else if (str[i] == 'c' && isspace(str[i + 1])) {
      i += 2;

      CStrUtil::skipSpace(str, &i);

      int j = i;

      skipToColorKey(str, &i);

      color->color = CStrUtil::stripSpaces(std::string(&str[j], size_t(i - j)));
    }
    else {
      return false;
    }

    CStrUtil::skipSpace(str, &i);
  }

  return true;
}

void
CXPMImage::
skipToColorKey(const char *str, int *i)
{
  while (str[*i] != '\0') {
    if (isspace(str[*i]) && isColorKey(&str[*i + 1]))
      break;

    (*i)++;
  }
}

bool
CXPMImage::
isColorKey(const char *str)
{
  if      (str[0] == 'm' && isspace(str[1]))
    return true;
  else if (str[0] == 's' && isspace(str[1]))
    return true;
  else if (str[0] == 'g' && str[1] == '4' && isspace(str[2]))
    return true;
  else if (str[0] == 'g' && isspace(str[1]))
    return true;
  else if (str[0] == 'c' && isspace(str[1]))
    return true;

  return false;
}

bool
CXPMImage::
readData(const char *data, int *i, CXPMImageData *xpm_data)
{
  xpm_data->data = new uint [size_t(xpm_data->width*xpm_data->height)];

  //------

  while (skipComment(data, i))
    ;

  //------

  auto *colors = createImageColors(xpm_data);

  //------

  int pos = 0;

  for (int y = 0; y < xpm_data->height; ++y) {
    // skip to start of string
    while (data[*i] != '\0' && data[*i] != '\"')
      (*i)++;

    //------

    char *str = readString(data, i);

    if (str == nullptr)
      return false;

    //------

    if (xpm_data->num_colors <= 256) {
      if (! readDataString(xpm_data, str, xpm_data->data, &pos))
        return false;
    }
    else {
      if (! readData24String(xpm_data, colors, str, xpm_data->data, &pos))
        return false;
    }

    //------

    delete [] str;
  }

  //------

  if (xpm_data->num_colors > 256) {
    if (xpm_data->transparent) {
      const CRGBA &color = colors[xpm_data->transparent_color];

      xpm_data->transparent_color = color.getId();
    }
  }

  //------

  delete [] colors;

  //------

  return true;
}

bool
CXPMImage::
readDataString(CXPMImageData *xpm_data, const char *str, uint *data, int *pos)
{
  auto len = strlen(str);
  int  elen = xpm_data->width*xpm_data->chars_per_pixel;

  if (int(len) != elen)
    return false;

  //------

  int i = 0;

  if (xpm_data->chars_per_pixel == 1) {
    for (int x = 0; x < xpm_data->width; ++x) {
      int j;

      for (j = 0; j < xpm_data->num_colors; ++j)
        if (str[i] == xpm_data->colors[j].name[0])
          break;

      if (j >= xpm_data->num_colors) {
        std::cerr << "No color for symbol " << xpm_data->colors[j].name << "\n";
        j = 0;
      }

      data[(*pos)++] = uint(j);

      //------

      ++i;
    }
  }
  else {
    for (int x = 0; x < xpm_data->width; ++x) {
      std::string name = std::string(&str[i], size_t(xpm_data->chars_per_pixel));

      int j = 0;

      for (j = 0; j < xpm_data->num_colors; ++j) {
        if (name == xpm_data->colors[j].name)
          break;
      }

      if (j >= xpm_data->num_colors) {
        std::cerr << "No color for symbol " << xpm_data->colors[j].name << "\n";
        j = 0;
      }

      data[(*pos)++] = uint(j);

      //------

      i += xpm_data->chars_per_pixel;
    }
  }

  //------

  return true;
}

bool
CXPMImage::
readData24String(CXPMImageData *xpm_data, CRGBA *colors, const char *str, uint *data, int *pos)
{
  auto len  = strlen(str);
  int  elen = xpm_data->width*xpm_data->chars_per_pixel;

  if (int(len) != elen)
    return false;

  //------

  int i = 0;

  for (int x = 0; x < xpm_data->width; ++x) {
    std::string name = std::string(&str[i], size_t(xpm_data->chars_per_pixel));

    int j = 0;

    for (j = 0; j < xpm_data->num_colors; ++j) {
      if (name == xpm_data->colors[j].name)
        break;
    }

    if (j >= xpm_data->num_colors) {
      std::cerr << "No color for symbol " << xpm_data->colors[j].name << "\n";
      j = 0;
    }

    data[(*pos)++] = colors[j].getId();

    //------

    i += xpm_data->chars_per_pixel;
  }

  //------

  return true;
}

CRGBA *
CXPMImage::
createImageColors(CXPMImageData *xpm_data)
{
  auto *colors = new CRGBA [size_t(xpm_data->num_colors)];

  for (int i = 0; i < xpm_data->num_colors; ++i) {
    std::string name = xpm_data->colors[i].color;

    if (name != "") {
      if (CStrUtil::casecmp(name, "none") == 0) {
        colors[i].setRGBAI(0, 0, 0);

        xpm_data->transparent       = true;
        xpm_data->transparent_color = uint(i);
      }
      else
        lookupColor(name, colors[i]);
    }
    else {
      colors[i].setRGBAI(0, 0, 0);
    }
  }

  return colors;
}

bool
CXPMImage::
skipComment(const char *data, int *i)
{
  CStrUtil::skipSpace(data, i);

  if (data[*i] != '/' || data[*i + 1] != '*')
    return false;

  *i += 2;

  while (data[*i] != '\0' && (data[*i] != '*' || data[*i + 1] != '/'))
    (*i)++;

  if (data[*i] == '\0')
    return true;

  *i += 2;

  CStrUtil::skipSpace(data, i);

  return true;
}

char *
CXPMImage::
readString(const char *data, int *i)
{
  CStrUtil::skipSpace(data, i);

  if (data[*i] != '\"')
    return nullptr;

  (*i)++;

  //------

  int j = *i;

  //------

  while (data[*i] != '\0' && data[*i] != '\"')
    (*i)++;

  if (data[*i] != '\"')
    return nullptr;

  char *str = CStrUtil::strndup(&data[j], uint(*i - j));

  (*i)++;

  //------

  CStrUtil::skipSpace(data, i);

  if (data[*i] == ',') {
    (*i)++;

    CStrUtil::skipSpace(data, i);
  }

  //------

  return str;
}

bool
CXPMImage::
lookupColor(const std::string &name, CRGBA &color)
{
  if (name[0] != '#') {
    double r, g, b;

    if (CRGBName::lookup(name, &r, &g, &b))
      color.setRGBA(r, g, b);
    else {
      std::cerr << "Color name '" << name << "' not found\n";

      color.setRGBAI(0, 0, 0);
    }
  }
  else {
    char buffer[5];

    auto len = name.size() - 1;

    if      (len == 12) {
      double rgb_scale = 1.0/65535.0;

      buffer[4] = '\0';

      uint r, g, b;

      strcpy(buffer, name.substr(1, 4).c_str()); sscanf(buffer, "%x", &r);
      strcpy(buffer, name.substr(5, 4).c_str()); sscanf(buffer, "%x", &g);
      strcpy(buffer, name.substr(9, 4).c_str()); sscanf(buffer, "%x", &b);

      color.setRGBA(r*rgb_scale, g*rgb_scale, b*rgb_scale);
    }
    else if (len == 6) {
      buffer[2] = '\0';

      uint r, g, b;

      strcpy(buffer, name.substr(1, 2).c_str()); sscanf(buffer, "%x", &r);
      strcpy(buffer, name.substr(3, 2).c_str()); sscanf(buffer, "%x", &g);
      strcpy(buffer, name.substr(5, 2).c_str()); sscanf(buffer, "%x", &b);

      color.setRGBAI(r, g, b);
    }
    else {
      std::cerr << "Color name '" << name << "' not found\n";

      color.setRGBAI(0, 0, 0);
    }
  }

  return true;
}

//----------------------

bool
CXPMImage::
write(CFile *file, CGenImage *image)
{
  if (! image->hasColormap()) {
    std::cerr << "XPM Image Depth greater than 8 not supported\n";
    return false;
  }

  //------

  char *colors_used;
  int   num_colors_used;

  getColorUsage(image, &colors_used, &num_colors_used);

  //------

  std::string base = file->getBase();

  file->write("/* XPM */\n");
  file->write("\n");
  file->write("static const char *\n");
  file->write(base.c_str());
  file->write("_data[] = {\n");

  //------

  file->write("  /* width height num_colors chars_per_pixel x_hot y_hot */\n");

  xpm_chars_per_pixel_ = 1;

  auto count = xpm_pixel_chars_.size();

  while (num_colors_used > int(count)) {
    count *= xpm_pixel_chars_.size();

    xpm_chars_per_pixel_++;
  }

  file->write("  \"");
  file->write(CStrUtil::toString(image->getWidth()));
  file->write(" ");
  file->write(CStrUtil::toString(image->getHeight()));
  file->write(" ");
  file->write(CStrUtil::toString(num_colors_used));
  file->write(" ");
  file->write(CStrUtil::toString(xpm_chars_per_pixel_));
  file->write(" ");
  file->write(CStrUtil::toString(xpm_x_hot_));
  file->write(" ");
  file->write(CStrUtil::toString(xpm_y_hot_));
  file->write("\",\n");
  file->write("\n");

  //------

  file->write("  /* colors */\n");

  auto *pixel_map = new int [size_t(image->getNumColors())];

  int j = 0;

  for (uint i = 0; i < image->getNumColors(); ++i) {
    if (! colors_used[i])
      continue;

    if (image->isTransparent() && image->getTransparentColor() == i) {
      pixel_map[i] = -1;
      continue;
    }

    auto c = image->getColor(i);

    CRGBA rgba(c.r, c.g, c.b, c.a);

    pixel_map[i] = j;

    file->write("  \"");
    file->write(pixelToSymbol(j));
    file->write(" s symbol");
    file->write(CStrUtil::toString(j + 1));
    file->write(" c ");
    file->write(colorToString(rgba));
    file->write(" m ");
    file->write(colorToMonoString(rgba));
    file->write("\",\n");

    ++j;
  }

  if (image->isTransparent())
    file->write("  \"  s mask c none\",\n");

  file->write("\n");

  //------

  file->write("  /* pixels */\n");

  for (uint i = 0; i < image->getHeight(); ++i) {
    file->write("  \"");

    for (uint ij = 0; ij < image->getWidth(); ++ij) {
      auto pixel = image->getColorIndex(ij, i);

      file->write(pixelToSymbol(pixel_map[pixel]));
    }

    file->write("\",\n");
  }

  //------

  file->write("};\n");

  //------

  delete [] pixel_map;
  delete [] colors_used;

  return true;
}

void
CXPMImage::
getColorUsage(CGenImage *image, char **used, int *num_used)
{
  *used     = new char [size_t(image->getNumColors())];
  *num_used = 0;

  for (uint i = 0; i < image->getNumColors(); ++i)
    (*used)[i] = 0;

  for (uint y = 0; y < image->getHeight(); ++y)
    for (uint x = 0; x < image->getWidth(); ++x) {
      auto pixel = image->getColorIndex(x, y);

      if (! (*used)[pixel])
        (*num_used)++;

      (*used)[pixel] |= 0x01;
    }
}

std::string
CXPMImage::
pixelToSymbol(int pixel)
{
  std::string pixel_string;

  if (pixel < 0) {
    for (int i = 0; i < xpm_chars_per_pixel_; ++i)
      pixel_string += ' ';

    return(pixel_string);
  }

  auto pixel1 = uint(pixel);

  for (int i = 0; i < xpm_chars_per_pixel_ - 1; ++i) {
    auto pixel2 = pixel1 % xpm_pixel_chars_.size();

    pixel_string += xpm_pixel_chars_[pixel2];

    pixel1 /= uint(xpm_pixel_chars_.size());
  }

  pixel_string += xpm_pixel_chars_[pixel1];

  return pixel_string;
}

std::string
CXPMImage::
colorToString(const CRGBA &rgba)
{
  uint r, g, b, a;

  rgba.getRGBAI(&r, &g, &b, &a);

  char color_string[32];

  sprintf(color_string, "#%02x%02x%02x", r & 0xFF, g & 0xFF, b & 0xFF);

  return std::string(color_string);
}

std::string
CXPMImage::
colorToMonoString(const CRGBA &rgba)
{
  uint r, g, b, a;

  rgba.getRGBAI(&r, &g, &b, &a);

  int gray = (r + g + b)/3;

  if (gray > 127)
    return "white";
  else
    return "black";
}

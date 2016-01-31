#include <QWidget>

class QPaintEvent;
class QTimer;
class CImageAnim;
class CQImageCanvas;
class CQGIFDisplayStrip;
class CQGIFDisplayStripCanvas;
class CQIntegerEdit;
class QLineEdit;
class QMouseEvent;
class CQPixmap;
class QScrollBar;

class CQGIFDisplay : public QWidget {
  Q_OBJECT

 private:
  CQImageCanvas     *canvas_;
  CQIntegerEdit     *frame_edit_;
  CQIntegerEdit     *left_edit_;
  CQIntegerEdit     *top_edit_;
  CQIntegerEdit     *delay_edit_;
  QLineEdit         *dispose_edit_;
  CQGIFDisplayStrip *strip_;
  CQPixmap          *pixmap_;
  CImageAnim        *anim_;
  bool               playing_;
  int                frame_size_;
  int                frame_border_;
  int                frame_;
  bool               clear_;
  QTimer            *timer_;

 public:
  CQGIFDisplay();
 ~CQGIFDisplay();

  void loadFile(const std::string &filename);

  void draw(QPainter *painter);

  CImageAnim *getAnim() const { return anim_; }

  int getFrameSize  () const { return frame_size_; }
  int getFrameBorder() const { return frame_border_; }

  int  getFrame() const { return frame_; }
  void setFrame(int frame);

  uint getNumFrames() const;

  void incrFrame() { setFrame(getFrame() + 1); }
  void decrFrame() { setFrame(getFrame() - 1); }

  void pause();

 private slots:
  void playSlot();
  void pauseSlot();
  void stepSlot();
  void bstepSlot();

  void editSlot();

  void timeout();
};

class CQImageCanvas : public QWidget {
  Q_OBJECT

 private:
  CQGIFDisplay *display_;

 public:
  CQImageCanvas(CQGIFDisplay *display);

  void resizeEvent(QResizeEvent *event);

  void paintEvent(QPaintEvent *event);

 private:
  QImage qimage_;
};

class CQGIFDisplayStrip : public QWidget {
  Q_OBJECT

 public:
  CQGIFDisplayStrip(CQGIFDisplay *display);

  void updateScrollbar();

  CQGIFDisplay *getDisplay() const { return display_; }

 private slots:
  void scrollSlot(int);

 private:
  CQGIFDisplay            *display_;
  QScrollBar              *hbar_;
  CQGIFDisplayStripCanvas *canvas_;
};

class CQGIFDisplayStripCanvas : public QWidget {
  Q_OBJECT

 public:
  CQGIFDisplayStripCanvas(CQGIFDisplayStrip *strip);

  void setOffset(int offset);

  void paintEvent(QPaintEvent *);

  void mousePressEvent(QMouseEvent *e);

 private:
  CQGIFDisplayStrip *strip_;
  int                offset_;
};

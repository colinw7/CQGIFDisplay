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
  void saveSlot();

  void timeout();

 private:
  CQImageCanvas     *canvas_ { nullptr };
  CQIntegerEdit     *frame_edit_ { nullptr };
  CQIntegerEdit     *left_edit_ { nullptr };
  CQIntegerEdit     *top_edit_ { nullptr };
  CQIntegerEdit     *delay_edit_ { nullptr };
  QLineEdit         *dispose_edit_ { nullptr };
  CQGIFDisplayStrip *strip_ { nullptr };
  CQPixmap          *pixmap_ { nullptr };
  CImageAnim        *anim_ { nullptr };
  bool               playing_ { false };
  int                frame_size_ { 64 };
  int                frame_border_ { 4 };
  int                frame_ { 0 };
  bool               clear_ { false };
  QTimer            *timer_ { nullptr };
};

//---

class CQImageCanvas : public QWidget {
  Q_OBJECT

 private:
  CQGIFDisplay *display_;

 public:
  CQImageCanvas(CQGIFDisplay *display);

  void resizeEvent(QResizeEvent *event) override;

  void paintEvent(QPaintEvent *event) override;

 private:
  QImage qimage_;
};

//---

class CQGIFDisplayStrip : public QWidget {
  Q_OBJECT

 public:
  CQGIFDisplayStrip(CQGIFDisplay *display);

  void updateScrollbar();

  CQGIFDisplay *getDisplay() const { return display_; }

 private slots:
  void scrollSlot(int);

 private:
  CQGIFDisplay            *display_ { nullptr };
  QScrollBar              *hbar_ { nullptr };
  CQGIFDisplayStripCanvas *canvas_ { nullptr };
};

//---

class CQGIFDisplayStripCanvas : public QWidget {
  Q_OBJECT

 public:
  CQGIFDisplayStripCanvas(CQGIFDisplayStrip *strip);

  void setOffset(int offset);

  void paintEvent(QPaintEvent *) override;

  void mousePressEvent(QMouseEvent *e) override;

 private:
  CQGIFDisplayStrip *strip_ { nullptr };
  int                offset_ { 0 };
};

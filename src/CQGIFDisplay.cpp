#include <CQGIFDisplay.h>
#include <CImageLib.h>
#include <CQApp.h>
#include <CQImageButton.h>
#include <CQIntegerEdit.h>
#include <CQLabel.h>
#include <CQPixmapEd.h>
#include <CQPixmapCache.h>
#include <CQUtil.h>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTimer>
#include <QPainter>
#include <QMouseEvent>
#include <QScrollBar>

#include <svg/play_svg.h>
#include <svg/pause_svg.h>
#include <svg/play_one_svg.h>
#include <svg/rewind_one_svg.h>

int
main(int argc, char **argv)
{
  CQApp app(argc, argv);

  CQGIFDisplay *display = new CQGIFDisplay;

  if (argc > 1)
    display->loadFile(argv[1]);

  display->show();

  return app.exec();
}

CQGIFDisplay::
CQGIFDisplay() :
 canvas_(0), pixmap_(0), playing_(false), frame_size_(64), frame_border_(4)
{
  resize(400, 300);

  setWindowTitle("CQGIFDisplay");

  QVBoxLayout *layout1 = new QVBoxLayout(this);
  layout1->setMargin(0); layout1->setSpacing(2);

  QHBoxLayout *layout2 = new QHBoxLayout;
  layout2->setMargin(0); layout2->setSpacing(2);

  QVBoxLayout *layout3 = new QVBoxLayout;
  layout3->setMargin(0); layout3->setSpacing(2);

  QVBoxLayout *layout4 = new QVBoxLayout;
  layout4->setMargin(0); layout4->setSpacing(2);

  frame_edit_   = new CQIntegerEdit();
  left_edit_    = new CQIntegerEdit();
  top_edit_     = new CQIntegerEdit();
  frame_edit_   = new CQIntegerEdit();
  delay_edit_   = new CQIntegerEdit();
  dispose_edit_ = new CQIntegerEdit();

  QGridLayout *glayout = new QGridLayout;

  glayout->addWidget(new CQLabel("Frame"  ), 0, 0);
  glayout->addWidget(new CQLabel("Left"   ), 1, 0);
  glayout->addWidget(new CQLabel("Top"    ), 2, 0);
  glayout->addWidget(new CQLabel("Delay"  ), 3, 0);
  glayout->addWidget(new CQLabel("Dispose"), 4, 0);

  glayout->addWidget(frame_edit_  , 0, 1);
  glayout->addWidget(left_edit_   , 1, 1);
  glayout->addWidget(top_edit_    , 2, 1);
  glayout->addWidget(delay_edit_  , 3, 1);
  glayout->addWidget(dispose_edit_, 4, 1);

  QHBoxLayout *blayout = new QHBoxLayout;
  blayout->setMargin(0); blayout->setSpacing(2);

  QPushButton *edit_button   = new QPushButton("Edit");
  QPushButton *delete_button = new QPushButton("Delete");

  connect(edit_button, SIGNAL(clicked()), this, SLOT(editSlot()));

  blayout->addWidget(edit_button);
  blayout->addWidget(delete_button);
  blayout->addStretch();

  layout4->addLayout(glayout);
  layout4->addStretch();
  layout4->addLayout(blayout);

  canvas_ = new CQImageCanvas(this);

  //CQAppInst->addObjEditFilter(canvas_);

  layout3->addWidget(canvas_);

  QHBoxLayout *clayout = new QHBoxLayout;
  clayout->setMargin(2); clayout->setSpacing(16);

  CQImageButton *play_button  = new CQImageButton(CQPixmapCacheInst->getIcon("PLAY"));
  CQImageButton *pause_button = new CQImageButton(CQPixmapCacheInst->getIcon("PAUSE"));
  CQImageButton *step_button  = new CQImageButton(CQPixmapCacheInst->getIcon("PLAY_ONE"));
  CQImageButton *bstep_button = new CQImageButton(CQPixmapCacheInst->getIcon("REWIND_ONE"));

  play_button ->setToolTip("Play");
  pause_button->setToolTip("Pause");
  step_button ->setToolTip("Step");
  bstep_button->setToolTip("Back Step");

  connect(play_button , SIGNAL(clicked()), this, SLOT(playSlot ()));
  connect(pause_button, SIGNAL(clicked()), this, SLOT(pauseSlot()));
  connect(step_button , SIGNAL(clicked()), this, SLOT(stepSlot ()));
  connect(bstep_button, SIGNAL(clicked()), this, SLOT(bstepSlot()));

  clayout->addWidget(play_button );
  clayout->addWidget(pause_button);
  clayout->addWidget(step_button );
  clayout->addWidget(bstep_button);
  clayout->addStretch();

  layout3->addLayout(clayout);
  layout3->addStretch();

  layout2->addLayout(layout4);
  layout2->addStretch();
  layout2->addLayout(layout3);

  strip_ = new CQGIFDisplayStrip(this);

  layout1->addLayout(layout2);
  layout1->addWidget(strip_);

  timer_ = new QTimer(this);

  connect(timer_, SIGNAL(timeout()), this, SLOT(timeout()));
}

CQGIFDisplay::
~CQGIFDisplay()
{
}

void
CQGIFDisplay::
loadFile(const std::string &filename)
{
  setWindowTitle(filename.c_str());

  CFile file(filename.c_str());

  anim_ = CImage::createGIFAnim(&file);

  setFrame(0);

  if (anim_ && anim_->size() == 0)
    anim_ = 0;

  if (anim_) {
    CImageFrame *frame = (*anim_)[0];

    CImagePtr image = frame->getImage();

    int d = getFrameBorder();

    int w = image->getWidth ();
    int h = image->getHeight();

    canvas_->setFixedSize(w, h);

    int frame_size = h + 2*d;

    ((QWidget *) strip_)->setMinimumWidth(w + 2*d);

    ((QWidget *) strip_)->setFixedHeight(frame_size + 48);

    int x = d;

    CImageAnim::iterator p1 = anim_->begin();
    CImageAnim::iterator p2 = anim_->end  ();

    for ( ; p1 != p2; ++p1) {
      CImagePtr image = (*p1)->getImage();

      int s = std::max(image->getWidth(), image->getHeight());

      x += s + 2*d;
    }
  }
}

uint
CQGIFDisplay::
getNumFrames() const
{
  return anim_->size();
}

void
CQGIFDisplay::
setFrame(int frame)
{
  if (frame < 0)
    frame = getNumFrames() - 1;

  if (frame >= int(getNumFrames()))
    frame = 0;

  if (frame >= int(getNumFrames()))
    return;

  frame_ = frame;

  frame_edit_->setValue(frame_);

  CImageFrame *iframe = (*anim_)[frame_];

  delay_edit_->setValue(iframe->getDelay());

  CImagePtr image = iframe->getImage();

  int l = image->getLeft();
  int t = image->getTop ();

  left_edit_->setValue(l);
  top_edit_ ->setValue(t);

  int dispose = iframe->getDispose();

  if      (dispose == 0)
    dispose_edit_->setText("None");
  else if (dispose == 1)
    dispose_edit_->setText("Leave in Place");
  else if (dispose == 2)
    dispose_edit_->setText("Restore Background");
  else if (dispose == 3)
    dispose_edit_->setText("Restore Previous");
  else
    dispose_edit_->setText(QString("Unknown %1").arg(dispose));

  strip_->update();

  update();
}

void
CQGIFDisplay::
draw(QPainter *painter)
{
  if (! anim_) return;

  CImageFrame *frame = (*anim_)[getFrame()];

  CImagePtr image = frame->getImage();

  int l, b, r, t;

  image->getBorder(&l, &b, &r, &t);

  int dispose = frame->getDispose();

  if (dispose == 2) {
    CRGBA bg = image->getColor(image->getTransparentColor());

    bg.setAlpha(1);

    painter->fillRect(rect(), QBrush(CQUtil::rgbaToColor(bg)));
  }

  painter->drawImage(QPoint(l, t), CQUtil::toQImage(image));
}

void
CQGIFDisplay::
playSlot()
{
  if (playing_) return;

  if (anim_) {
    CImageFrame *frame = (*anim_)[getFrame()];

    int delay = frame->getDelay();

    if (delay < 1)
      delay = 10;

    timer_->start(delay*10);

    playing_ = true;
  }
}

void
CQGIFDisplay::
pauseSlot()
{
  pause();
}

void
CQGIFDisplay::
pause()
{
  if (! playing_) return;

  timer_->stop();

  playing_ = false;
}

void
CQGIFDisplay::
stepSlot()
{
  if (playing_)
    timer_->stop();

  incrFrame();

  playing_ = false;
}

void
CQGIFDisplay::
bstepSlot()
{
  if (playing_)
    timer_->stop();

  decrFrame();

  playing_ = false;
}

void
CQGIFDisplay::
editSlot()
{
  if (! pixmap_) {
    pixmap_ = new CQPixmap;

    pixmap_->init();
  }

  if (anim_) {
    CImageFrame *frame = (*anim_)[getFrame()];

    CImagePtr image = frame->getImage();

    pixmap_->loadImage(image);

    pixmap_->show();
  }
}

void
CQGIFDisplay::
timeout()
{
  if (anim_) {
    incrFrame();

    CImageFrame *frame = (*anim_)[getFrame()];

    int delay = frame->getDelay();

    if (delay < 1)
      delay = 10;

    timer_->setInterval(delay*10);
  }

  if (! playing_)
    timer_->stop();

  update();
}

//----------

CQImageCanvas::
CQImageCanvas(CQGIFDisplay *display) :
 display_(display)
{
  setFocusPolicy(Qt::StrongFocus);
}

void
CQImageCanvas::
resizeEvent(QResizeEvent *)
{
  qimage_ = QImage(QSize(width(), height()), QImage::Format_ARGB32);
}

void
CQImageCanvas::
paintEvent(QPaintEvent *)
{
  QPainter ipainter(&qimage_);

  display_->draw(&ipainter);

  QPainter painter(this);

  painter.drawImage(QPoint(0, 0), qimage_);
}

//----------

CQGIFDisplayStrip::
CQGIFDisplayStrip(CQGIFDisplay *display) :
 QWidget(0), display_(display)
{
  QVBoxLayout *layout = new QVBoxLayout(this);

  canvas_ = new CQGIFDisplayStripCanvas(this);

  hbar_ = new QScrollBar(Qt::Horizontal);

  connect(hbar_, SIGNAL(valueChanged(int)), this, SLOT(scrollSlot(int)));

  layout->addWidget(canvas_);
  layout->addWidget(hbar_);
}

void
CQGIFDisplayStrip::
scrollSlot(int pos)
{
  canvas_->setOffset(-pos);

  canvas_->update();
}

void
CQGIFDisplayStrip::
updateScrollbar()
{
  int ps = display_->getFrameSize();

  int vw = ps*display_->getNumFrames();

  hbar_->setPageStep(ps);

  hbar_->setMinimum(0);
  hbar_->setMaximum(vw - ps);
}

//----------

CQGIFDisplayStripCanvas::
CQGIFDisplayStripCanvas(CQGIFDisplayStrip *strip) :
 QWidget(0), strip_(strip), offset_(0)
{
}

void
CQGIFDisplayStripCanvas::
setOffset(int offset)
{
  offset_ = offset;
}

void
CQGIFDisplayStripCanvas::
paintEvent(QPaintEvent *)
{
  strip_->updateScrollbar();

  QPainter painter(this);

  CRGBA bg(0.8,0.8,0.8);

  painter.fillRect(rect(), QBrush(CQUtil::rgbaToColor(bg)));

  int d = strip_->getDisplay()->getFrameBorder();

  int x = d + offset_;
  int y = d;

  int frame = 0;

  CImageAnim *anim = strip_->getDisplay()->getAnim();

  CImageAnim::iterator p1 = anim->begin();
  CImageAnim::iterator p2 = anim->end  ();

  for ( ; p1 != p2; ++p1) {
    CImagePtr image = (*p1)->getImage();

    int l = image->getLeft();
    int t = image->getTop ();

    int s = std::max(image->getWidth(), image->getHeight());

    CRGBA bg;

    if (frame == strip_->getDisplay()->getFrame())
      bg = CRGBA(0.8,0.6,0.6);
    else
      bg = CRGBA(0.6,0.6,0.8);

    painter.fillRect(QRect(x + l - d/2, y + t - d/2, s + d, s + d),
                     QBrush(CQUtil::rgbaToColor(bg)));

    painter.drawImage(QPoint(x + l, y + t), CQUtil::toQImage(image));

    x += s + 2*d;

    ++frame;
  }
}

void
CQGIFDisplayStripCanvas::
mousePressEvent(QMouseEvent *e)
{
  int xe = e->x();
  int ye = e->y();

  int d = strip_->getDisplay()->getFrameBorder();

  int x1 = 0;
  int y1 = 0;
  int x2 = x1;
  int y2 = y1;

  int frame = 0;

  CImageAnim *anim = strip_->getDisplay()->getAnim();

  CImageAnim::iterator p1 = anim->begin();
  CImageAnim::iterator p2 = anim->end  ();

  for ( ; p1 != p2; ++p1) {
    CImagePtr image = (*p1)->getImage();

    int s = std::max(image->getWidth(), image->getHeight());

    x2 = x1 + s + 2*d;
    y2 = y1 + s + 2*d;

    if (xe >= x1 && xe <= x2 && ye >= y1 && ye <= y2)
      break;

    x1 = x2;

    ++frame;
  }

  strip_->getDisplay()->pause();

  strip_->getDisplay()->setFrame(frame);
}

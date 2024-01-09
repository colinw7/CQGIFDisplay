#include <CQGIFDisplay.h>
#include <CImageLib.h>
#include <CQApp.h>
#include <CQImageButton.h>
#include <CQIntegerEdit.h>
#include <CQLabel.h>
#include <CQPixmapEd.h>
#include <CQPixmapCache.h>
#include <CQImageUtil.h>
#include <CQUtil.h>
#include <CQUtilRGBA.h>

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

  auto *display = new CQGIFDisplay;

  if (argc > 1)
    display->loadFile(argv[1]);

  display->show();

  return app.exec();
}

//---

CQGIFDisplay::
CQGIFDisplay()
{
  resize(400, 300);

  setWindowTitle("CQGIFDisplay");

  auto *layout1 = CQUtil::makeLayout<QVBoxLayout>(this, 0, 2);

  auto *layout2 = CQUtil::makeLayout<QHBoxLayout>(0, 2);
  auto *layout3 = CQUtil::makeLayout<QVBoxLayout>(0, 2);
  auto *layout4 = CQUtil::makeLayout<QVBoxLayout>(0, 2);

  frame_edit_   = new CQIntegerEdit();
  left_edit_    = new CQIntegerEdit();
  top_edit_     = new CQIntegerEdit();
  frame_edit_   = new CQIntegerEdit();
  delay_edit_   = new CQIntegerEdit();
  dispose_edit_ = new CQIntegerEdit();

  auto *glayout = CQUtil::makeLayout<QGridLayout>(2, 2);

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

  auto *blayout = CQUtil::makeLayout<QHBoxLayout>(0, 2);

  auto *edit_button   = CQUtil::makeLabelWidget<QPushButton>("Edit", "edit");
//auto *delete_button = CQUtil::makeLabelWidget<QPushButton>("Delete", "delete");
  auto *save_button   = CQUtil::makeLabelWidget<QPushButton>("Save", "save");

  connect(edit_button, SIGNAL(clicked()), this, SLOT(editSlot()));
  connect(save_button, SIGNAL(clicked()), this, SLOT(saveSlot()));

  blayout->addWidget(edit_button);
//blayout->addWidget(delete_button);
  blayout->addWidget(save_button);
  blayout->addStretch();

  layout4->addLayout(glayout);
  layout4->addStretch();
  layout4->addLayout(blayout);

  canvas_ = new CQImageCanvas(this);

  //CQAppInst->addObjEditFilter(canvas_);

  layout3->addWidget(canvas_);

  auto *clayout = CQUtil::makeLayout<QHBoxLayout>(2, 16);

  auto *play_button  = new CQImageButton(CQPixmapCacheInst->getIcon("PLAY"));
  auto *pause_button = new CQImageButton(CQPixmapCacheInst->getIcon("PAUSE"));
  auto *step_button  = new CQImageButton(CQPixmapCacheInst->getIcon("PLAY_ONE"));
  auto *bstep_button = new CQImageButton(CQPixmapCacheInst->getIcon("REWIND_ONE"));

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
    anim_ = nullptr;

  if (anim_) {
    auto *frame = (*anim_)[0];

    auto image = frame->getImage();

    int d = getFrameBorder();

    int w = image->getWidth ();
    int h = image->getHeight();

    canvas_->setFixedSize(w, h);

    int frame_size = h + 2*d;

    strip_->QWidget::setMinimumWidth(w + 2*d);

    strip_->QWidget::setFixedHeight(frame_size + 48);

    int x = d;

    for (auto pa = anim_->begin(); pa != anim_->end(); ++pa) {
      auto image = (*pa)->getImage();

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

  auto *iframe = (*anim_)[frame_];

  delay_edit_->setValue(iframe->getDelay());

  auto image = iframe->getImage();

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

  auto *frame = (*anim_)[getFrame()];

  auto image = frame->getImage();

  int l, b, r, t;
  image->getBorder(&l, &b, &r, &t);

  int dispose = frame->getDispose();

  if (dispose == 2) {
    CRGBA bg = image->getColor(image->getTransparentColor());

    bg.setAlpha(1);

    painter->fillRect(rect(), QBrush(CQUtil::rgbaToColor(bg)));
  }

  painter->drawImage(QPoint(l, t), CQImageUtil::toQImage(image));
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

    auto image = frame->getImage();

    pixmap_->loadImage(image);

    pixmap_->show();
  }
}

void
CQGIFDisplay::
saveSlot()
{
  int w = 0, h = 0;

  uint i = 0;

  for (auto pa = anim_->begin(); pa != anim_->end(); ++pa) {
    auto image = (*pa)->getImage();

    int l, b, r, t;
    image->getBorder(&l, &b, &r, &t);

    auto qimage = CQImageUtil::toQImage(image);

    if (i == 0) {
      w = qimage.width();
      h = qimage.height();
    }
    else {
      w = std::max(w, qimage.width ());
      h = std::max(h, qimage.height());
    }

    ++i;
  }

  auto nstr = QString::number(i);
  auto nlen = nstr.length();

  //---

  auto saveImage = QImage(QSize(w, h), QImage::Format_ARGB32);

  QPainter painter(&saveImage);

  i = 0;

  for (auto pa = anim_->begin(); pa != anim_->end(); ++pa) {
    auto *frame = (*anim_)[i];

    auto image = (*pa)->getImage();

    int l, b, r, t;
    image->getBorder(&l, &b, &r, &t);

    int dispose = frame->getDispose();

    if (dispose == 2) {
      auto bg = image->getColor(image->getTransparentColor());

      bg.setAlpha(1);

      painter.fillRect(rect(), QBrush(CQUtil::rgbaToColor(bg)));
    }

    auto qimage = CQImageUtil::toQImage(image);

    painter.drawImage(QPoint(l, t), qimage);

    auto istr = QString::number(i);
    while (istr.length() < nlen) istr = "0" + istr;

    auto name = "frame_" + istr + ".png";

    saveImage.save(name, "PNG");

    ++i;
  }
}

void
CQGIFDisplay::
timeout()
{
  if (anim_) {
    incrFrame();

    auto *frame = (*anim_)[getFrame()];

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
 QWidget(nullptr), display_(display)
{
  auto *layout = CQUtil::makeLayout<QVBoxLayout>(this, 2, 2);

  canvas_ = new CQGIFDisplayStripCanvas(this);

  hbar_ = CQUtil::makeWidget<QScrollBar>("hscroll");
  hbar_->setOrientation(Qt::Horizontal);

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
 QWidget(nullptr), strip_(strip)
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

  auto *anim = strip_->getDisplay()->getAnim();

  for (auto pa = anim->begin(); pa != anim->end(); ++pa) {
    auto image = (*pa)->getImage();

    int l = image->getLeft();
    int t = image->getTop ();

    int s = std::max(image->getWidth(), image->getHeight());

    CRGBA bg;

    if (frame == strip_->getDisplay()->getFrame())
      bg = CRGBA(0.8, 0.6, 0.6);
    else
      bg = CRGBA(0.6, 0.6, 0.8);

    painter.fillRect(QRect(x + l - d/2, y + t - d/2, s + d, s + d),
                     QBrush(CQUtil::rgbaToColor(bg)));

    painter.drawImage(QPoint(x + l, y + t), CQImageUtil::toQImage(image));

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

  auto *anim = strip_->getDisplay()->getAnim();

  for (auto pa = anim->begin(); pa != anim->end(); ++pa) {
    auto image = (*pa)->getImage();

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

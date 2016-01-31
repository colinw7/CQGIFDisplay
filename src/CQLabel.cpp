#include <CQLabel.h>

#include <QPainter>

CQLabel::
CQLabel(const QString &label, QWidget *parent) :
 QLabel(label, parent), orientation_(Qt::Horizontal)
{
  updateState();
}

CQLabel::
CQLabel(const QString &label, Qt::Orientation orient, QWidget *parent) :
 QLabel(label, parent), orientation_(orient)
{
  updateState();
}

void
CQLabel::
setOrientation(Qt::Orientation orient)
{
  orientation_ = orient;

  updateState();

  update();
}

void
CQLabel::
updateState()
{
  //if (orientation() == Qt::Horizontal)
  //  setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  //else
  //  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
}

void
CQLabel::
paintEvent(QPaintEvent *)
{
  QPainter painter(this);

  QRect r = rect();

  //painter.fillRect(r, QBrush(QColor(100,100,200)));

  if (orientation() == Qt::Vertical) {
    painter.translate(0, r.height());
    painter.rotate(-90);
  }

  QString str = text();

  QFontMetrics fm(font());

  painter.setPen(palette().color(QPalette::Active, QPalette::WindowText));

  int dx, dy;

  if (orientation() == Qt::Horizontal) {
    dx = r.width () - fm.width(str);
    dy = r.height() - fm.height();
  }
  else {
    dx = r.height() - fm.width(str);
    dy = r.width () - fm.height();
  }

  painter.drawText(dx/2, dy/2 + fm.ascent(), str);
}

QSize
CQLabel::
sizeHint() const
{
  QFontMetrics fm(font());

  int w = fm.width (text()) + 2*margin();
  int h = fm.height()       + 2*margin();

  if (orientation() == Qt::Horizontal)
    return QSize(w, h);
  else
    return QSize(h, w);
}

QSize
CQLabel::
minimumSizeHint() const
{
  return sizeHint();
}

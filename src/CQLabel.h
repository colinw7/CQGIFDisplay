#ifndef CQLABEL_H
#define CQLABEL_H

#include <QLabel>

class CQLabel : public QLabel {
 public:
  Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation)

 public:
  CQLabel(const QString &label="", QWidget *parent=0);

  CQLabel(const QString &label, Qt::Orientation orient, QWidget *parent=0);

  virtual ~CQLabel() { }

  Qt::Orientation orientation() const { return orientation_; }

  void setOrientation(Qt::Orientation orient);

  QSize sizeHint() const;

  QSize minimumSizeHint() const;

 protected:
  void paintEvent(QPaintEvent *e);

 private:
  void updateState();

 private:
  Qt::Orientation orientation_;
};

#endif

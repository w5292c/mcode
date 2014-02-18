#include "customwidget.h"

#include <QDebug>
#include <QPainter>

AcCustomWidget::AcCustomWidget(QWidget *pParent) :
  QWidget(pParent)
{
  qDebug() << "AcCustomWidget::AcCustomWidget: " << (const void *)this;
  setMaximumSize(320, 480);
  setMinimumSize(320, 480);
}

AcCustomWidget::~AcCustomWidget()
{
  qDebug() << "AcCustomWidget::~AcCustomWidget: " << (const void *)this;
}

void AcCustomWidget::paintEvent(QPaintEvent *pEvent)
{
  QPainter qp(this);

  const QSize &size = this->size();
  qDebug() << "Paint: " << size;

  QColor myColor(100, 255, 255);
  QColor colors[] = {
    QColor(255, 0, 0),
    QColor(0, 255, 0),
    QColor(0, 0, 255),
    QColor(255, 255, 0)
  };
//  drawWidget(qp);
  qp.setPen(myColor);
  qp.setBrush(myColor);

  for (int x = 0; x < 320; x++) {
    for (int y = 0; y < 480; y++) {
      qp.setPen(colors[(x+y)%4]);
      qp.drawPoint(x, y);
    }
  }

//  qp.drawRect(0, 0, size.width(), size.height());
//  qp.setPen(redColor);
//  qp.setBrush(redColor);
//  qp.drawRect(full, 0, till-full, PANEL_HEIGHT);

  QWidget::paintEvent(pEvent);
}

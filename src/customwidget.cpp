#include "customwidget.h"

#include <QDebug>
#include <QPainter>

AcCustomWidget::AcCustomWidget(QWidget *pParent) :
  QWidget(pParent)
{
  qDebug() << "AcCustomWidget::AcCustomWidget: " << (const void *)this;
  m_pScreenData = (quint32 *)malloc (4*320*480);
  setMaximumSize(320, 480);
  setMinimumSize(320, 480);

  QColor colors[] = {
    QColor(255, 0, 0),
    QColor(0, 255, 0),
    QColor(0, 0, 255),
    QColor(255, 255, 0)
  };

  for (int x = 0; x < 320; x++) {
    for (int y = 0; y < 480; y++) {
      setPixel (x, y, colors[(x + y)%4].rgb ());
    }
  }
}

AcCustomWidget::~AcCustomWidget()
{
  qDebug() << "AcCustomWidget::~AcCustomWidget: " << (const void *)this;
  free (m_pScreenData);
}

void AcCustomWidget::setPixel (uint x, uint y, QRgb color)
{
  if (x >= 0 && x < 320 && y >= 0 && y < 480)
  {
    m_pScreenData[320*y+x] = color;
    update ();
  }
  else
  {
    printf ("AcCustomWidget::setPixel: wrong request: at (%u, %u), color: %u\r\n", x, y, color);
  }
}

QRgb AcCustomWidget::getPixel (uint x, uint y) const
{
  return m_pScreenData[320*y+x];
}

void AcCustomWidget::paintEvent(QPaintEvent *pEvent)
{
  QPainter qp(this);

  const QSize &size = this->size();

  QColor myColor(100, 255, 255);
  QColor colors[] = {
    QColor(255, 0, 0),
    QColor(0, 255, 0),
    QColor(0, 0, 255),
    QColor(255, 255, 0)
  };

  for (int x = 0; x < 320; x++) {
    for (int y = 0; y < 480; y++) {
      qp.setPen(getPixel (x, y));
      qp.drawPoint(x, y);
    }
  }

  QWidget::paintEvent(pEvent);
}

/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2014 Alexander Chumakov
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "customwidget.h"

#include <QDebug>
#include <QPainter>

AcCustomWidget::AcCustomWidget(QWidget *pParent) :
  QWidget(pParent),
  m_scrollPosition(0)
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

QRgb AcCustomWidget::getPixel(uint x, uint y) const
{
  y += m_scrollPosition;
  if (y >= 480) {
    y -= 480;
  }
  return m_pScreenData[320*y+x];
}

void AcCustomWidget::setScrollPosition(uint scrollPosition)
{
  m_scrollPosition = (scrollPosition % 480);
  update();
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
      qp.setPen(getPixel(x, y));
      qp.drawPoint(x, y);
    }
  }

  QWidget::paintEvent(pEvent);
}

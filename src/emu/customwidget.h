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

#ifndef AC_CUSTOM_WIDGET_H
#define AC_CUSTOM_WIDGET_H

#include <QWidget>

class AcCustomWidget : public QWidget
{
  Q_OBJECT
public:
  AcCustomWidget(uint width, uint height, QWidget *pParent = 0);
  ~AcCustomWidget();

  void reset();
  void turn(bool on);

  void setPixel(uint x, uint y, QRgb color);
  QRgb getPixel(uint x, uint y) const;

  uint width() const;
  uint height() const;
  void setSize(uint width, uint height);
  void setScrollPosition(uint scrollPosition);

protected:
  void requestUpdate();
  void paintEvent(QPaintEvent *pEvent);

protected slots:
  void xupdate();

signals:
  void updateSignal();

private:
  bool m_on;
  uint m_width;
  uint m_height;
  uint m_scrollPosition;
  quint32 *m_pScreenData;
  bool nOutstandingRequestToUpdate;
};

#endif /* AC_CUSTOM_WIDGET_H */

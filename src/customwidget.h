#ifndef AC_CUSTOM_WIDGET_H
#define AC_CUSTOM_WIDGET_H

#include <QWidget>

class AcCustomWidget : public QWidget
{
  Q_OBJECT
public:
  AcCustomWidget(QWidget *pParent = 0);
  ~AcCustomWidget();

  void setPixel (uint x, uint y, QRgb color);
  QRgb getPixel (uint x, uint y) const;

protected:
  void paintEvent(QPaintEvent *pEvent);

private:
  quint32 *m_pScreenData;
};

#endif /* AC_CUSTOM_WIDGET_H */

#ifndef AC_CUSTOM_WIDGET_H
#define AC_CUSTOM_WIDGET_H

#include <QWidget>

class AcCustomWidget : public QWidget
{
  Q_OBJECT
public:
  AcCustomWidget(QWidget *pParent = 0);
  ~AcCustomWidget();

protected:
  void paintEvent(QPaintEvent *pEvent);
};

#endif /* AC_CUSTOM_WIDGET_H */

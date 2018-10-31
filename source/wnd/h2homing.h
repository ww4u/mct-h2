#ifndef H2HOMING_H
#define H2HOMING_H

#include <QWidget>

namespace Ui {
class H2Homing;
}

class H2Homing : public QWidget
{
    Q_OBJECT

public:
    explicit H2Homing(QWidget *parent = 0);
    ~H2Homing();

protected slots:
    void slotChangePicture(QString str);

private:
    Ui::H2Homing *ui;
};

#endif // H2HOMING_H
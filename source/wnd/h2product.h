#ifndef H2PRODUCT_H
#define H2PRODUCT_H

#include <QWidget>
#include "xconfig.h"

namespace Ui {
class H2Product;
}

class H2Product : public XConfig
{
    Q_OBJECT

public:
    explicit H2Product(QString strDevInfo = "", QWidget *parent = 0);
    ~H2Product();
    int setApply(ViSession visa);

private:
    Ui::H2Product *ui;
    QString m_IP;
    QString m_Version;
    QString m_Type;
    QString m_SN;
};

#endif // H2PRODUCT_H
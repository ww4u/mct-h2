#ifndef H2MEASUREMENT_H
#define H2MEASUREMENT_H

#include <QWidget>
#include "xconfig.h"
namespace Ui {
class H2Measurement;
}

class H2Measurement : public XConfig
{
    Q_OBJECT

public:
    explicit H2Measurement(QWidget *parent = 0);
    ~H2Measurement();

    int readDeviceConfig(); //从设备上读取配置数据
    int writeDeviceConfig();//将软件界面的数据配置设置到设备
    int loadConfig();       //读取本地配置文件 启动软件或者添加新设备时调用
    int saveConfig();       //将软件界面的数据配置写入到本地配置文件
    void updateShow();      //更新界面

    void translateUI() override;

public slots:
    void setWorkStrokeX(double WorkStrokeX);
    void setWorkStrokeY(double WorkStrokeY);

signals:
    void signal_AxesZeroPoint_currentTextChanged(QString);

private slots:
    void slotChangeCornerPicture(int index);

    void on_doubleSpinBox_pzpX_valueChanged(double arg1);

    void on_doubleSpinBox_pzpY_valueChanged(double arg1);

private:
    Ui::H2Measurement *ui;

    int     m_ZeroPoint        ;
    double  m_ProjectZeroPointX;
    double  m_ProjectZeroPointY;
    double  m_SWLimitPositiveX ;
    double  m_SWLimitPositiveY ;
    double  m_SWLimitNegativeX ;
    double  m_SWLimitNegativeY ;

    double  m_WorkStrokeX;
    double  m_WorkStrokeY;
};

#endif // H2MEASUREMENT_H

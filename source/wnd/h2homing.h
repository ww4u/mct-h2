#ifndef H2HOMING_H
#define H2HOMING_H

#include <QWidget>
#include "xconfig.h"
namespace Ui {
class H2Homing;
}

class H2Homing : public XConfig
{
    Q_OBJECT

public:
    explicit H2Homing(QWidget *parent = 0);
    ~H2Homing();

    int readDeviceConfig(); //从设备上读取配置数据
    int writeDeviceConfig();//将软件界面的数据配置设置到设备
    int loadConfig();       //读取本地配置文件 启动软件或者添加新设备时调用
    int saveConfig();       //将软件界面的数据配置写入到本地配置文件
    void updateShow();      //更新界面

    void translateUI() override;

public slots:
    void slot_set_direction(QString text);

private:
    Ui::H2Homing *ui;

    QString m_Target;    
//    QString m_Movement;
    int     m_HomingMode;
    double  m_SearchVelocity;
    double  m_ForceLimit    ;
};

#endif // H2HOMING_H

#ifndef H2JOGMODE_H
#define H2JOGMODE_H

#include <QWidget>
#include "xconfig.h"
namespace Ui {
class H2JogMode;
}

class H2JogMode : public XConfig
{
    Q_OBJECT

public:
    explicit H2JogMode(QWidget *parent = 0);
    ~H2JogMode();

    int readDeviceConfig(); //从设备上读取配置数据
    int writeDeviceConfig();//将软件界面的数据配置设置到设备
    int loadConfig();       //读取本地配置文件 启动软件或者添加新设备时调用
    int saveConfig();       //将软件界面的数据配置写入到本地配置文件
    void updateShow();      //更新界面

    void translateUI() override;

private:
    Ui::H2JogMode *ui;

    double m_CrawlingVelocity;
    double m_CrawlingTime    ;
    double m_MaximumVelocity ;
};

#endif // H2JOGMODE_H

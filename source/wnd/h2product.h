#ifndef H2PRODUCT_H
#define H2PRODUCT_H

#include <QWidget>
#include "xconfig.h"

namespace Ui {
class H2Product;
}

class LanCheckedThread : public QThread
{
    Q_OBJECT
public:
    LanCheckedThread(QObject *parent = NULL);
    ~LanCheckedThread();
    void setIp(const QString &ip);

signals:
    void signalCheckConnect(bool);

protected:
    virtual void run();

private slots:
    void slotTimerTimeout();

private:
    QString m_ip;
    int m_timerInterval;
    int m_timerTimeoutCounter;
};




class H2Product : public XConfig
{
    Q_OBJECT

public:
    explicit H2Product(QString strDevInfo = "", QWidget *parent = 0);
    ~H2Product();

    void change_online_status(bool bl);

    int readDeviceConfig(); //从设备上读取配置数据
    int writeDeviceConfig();//将软件界面的数据配置设置到设备
    int loadConfig();       //读取本地配置文件 启动软件或者添加新设备时调用
    int saveConfig();       //将软件界面的数据配置写入到本地配置文件
    void updateShow();      //更新界面

    void translateUI() override;

    bool handCloseFlag() const;
    void setHandCloseFlag(bool handCloseFlag);

signals:
    void signal_online_clicked(QString);

private slots:
    void on_pushButton_status_clicked();

    void slotCheckLanConnect(bool isConnect);

private:
    Ui::H2Product *ui;
    LanCheckedThread *m_thread;

    QString m_strDevInfo;
    QString m_IP;
    QString m_Version;
    QString m_Type;
    QString m_SN;

    bool m_handCloseFlag;
};

#endif // H2PRODUCT_H

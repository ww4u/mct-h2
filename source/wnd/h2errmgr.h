#ifndef H2ERRMGR_H
#define H2ERRMGR_H

#include <QWidget>
#include "xconfig.h"
#include "comboxdelegate.h"
#include "checkdelegate.h"
#include "errmgrmodel.h"

namespace Ui {
class H2ErrMgr;
}

class H2ErrMgr : public XConfig
{
    Q_OBJECT

public:
    explicit H2ErrMgr(QWidget *parent = 0);
    ~H2ErrMgr();

    void setModel( ErrMgrModel *pModel );

    int readDeviceConfig(); //从设备上读取配置数据
    int writeDeviceConfig();//将软件界面的数据配置设置到设备
    int loadConfig();       //读取本地配置文件 启动软件或者添加新设备时调用
    int saveConfig();       //将软件界面的数据配置写入到本地配置文件
    void updateShow();      //更新界面

    void translateUI() override;

private:
    Ui::H2ErrMgr *ui;
    ErrMgrModel     mErrManager;

    checkDelegate *m_pCheckDelegate;
    checkDelegate *m_pRadioDelegate;
    comboxDelegate *m_pReactionCombox;

    QString m_errorFileInfo;

    QMap< int, QString> m_mapRespIntToStr;
    QMap< QString, int> m_mapRespStrToInt;

    int calcResponse(int reaction, int outputAble);
    int parseResponse(int response, int *reaction, int *outputAble);

};

#endif // H2ERRMGR_H

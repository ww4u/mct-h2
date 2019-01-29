#ifndef ROBO_CONFIG_H
#define ROBO_CONFIG_H

#include <QtWidgets>
#include "xrobo.h"
#include "megainterface.h"
#include "MegaGateway.h"

namespace Ui {
class RoboConfig;
}

class RoboConfig : public QWidget
{
    Q_OBJECT

signals:
    void signalCurrentRobotChanged(QString,int,int,int);

    void signal_focus_in( const QString &);
    void signalDataChanged();
    void signal_record_selected(int);
    void signalDeviceConnect(bool);

public:
    explicit RoboConfig(QWidget *parent = 0);
    ~RoboConfig();
    void changeLanguage(QString qmFile);
    void addDeviceWithIP(QString strID);

public slots:
//    void updateUI();
    void slotDownload();
    void slotUpload();
    void slotStore();
    void slotSync();
    void slotSearch();
    void slotExit();
    void slotWifi();
    void slotUpdateFirmware();
    void slotConnect();
    void slotSetOneRecord(int row,QString type,double x,double y,double v,double a);

private slots:
    void on_buttonBox_clicked(QAbstractButton *button);
    void slot_current_changed( QTreeWidgetItem* pre,QTreeWidgetItem* nxt );
    void slot_open_close(QString strIP);
    void slotShowContextmenu(const QPoint &pos);
    void slotAddNewRobot(QString strDevInfo );
    void soltActionClose();
    void soltActionDelete();

    void slotStoreEnd(int val);
    void slotDownloadEnd(QString str);
    void slotUploadEnd(QString str);
    void slotUpdateFirmwareEnd(int ret);


private:
    Ui::RoboConfig *ui;
    MegaInterface *m_megaSerachWidget;

    class RobotInfo
    {
    public:
        XRobo*  m_Robo;
        int     m_Visa;
        int     m_DeviceName;
        int     m_RoboName;
        QString m_strDevInfo;
    };

    QTreeWidgetItem *m_pRootNode;
    QList<RobotInfo> m_RobotList;
    int mIndex;

    void buildUI();
    void loadXmlConfig();
    void createRobot(QString strDevInfo);
    int  deviceOpen(QString strIP);
    int  deviceClose();
    int  setApply();
    int  setReset();

    QMenu *m_menu;
    QTranslator m_translator;
    void translateUi();

};

#endif // H2CONFIG_H

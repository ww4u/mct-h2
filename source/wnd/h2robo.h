#ifndef H2ROBO_H
#define H2ROBO_H

#include <QTreeWidgetItem>
#include "xrobo.h"

#include "mystd.h"

#include "xconfig.h"

#include "h2product.h"
#include "h2configuration.h"
#include "h2jogmode.h"
#include "h2homing.h"
#include "h2measurement.h"
#include "h2action.h"
#include "h2errmgr.h"

class H2Robo : public XRobo
{
    Q_OBJECT
public:
    H2Robo( QStackedWidget *pWig, QString strDevInfo = "", QObject *pObj = Q_NULLPTR );
    ~H2Robo();

public:
    QTreeWidgetItem *roboNode();
    QList<XConfig *> subConfigs() const;

    QString getDeviceName(QString strDevInfo);

    void translateUI() override;

public slots:
    void change_online_status(bool bl);

protected:
    void buildConnection();

signals:
    void signal_online_request(QString);
    void signal_action_selected(int);

private:
    QTreeWidgetItem *m_pRoboNode;
    QList<XConfig*> mSubConfigs;

    H2Product       *m_pProduct         ;
    H2Configuration *m_pH2Configuration ;
    H2Measurement   *m_pH2Measurement   ;
    H2Homing        *m_pH2Homing        ;
    H2JogMode       *m_pH2JogMode       ;
    H2Action        *m_pH2Action        ;
    H2ErrMgr        *m_pH2ErrMgr        ;
};

#endif // H2ROBO_H

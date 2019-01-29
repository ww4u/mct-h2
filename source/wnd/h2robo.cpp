#include "h2robo.h"

#define new_widget( type, var, icon ) \
do{ \
    var = new type;\
    Q_ASSERT( NULL != var ); \
    QTreeWidgetItem *plwItem = new QTreeWidgetItem();   \
    Q_ASSERT( NULL != plwItem ); \
    plwItem->setIcon( 0, QIcon( icon) ); \
    plwItem->setData( 0, Qt::UserRole, QVariant( QVariant::fromValue(var) ) ); \
    m_pRoboNode->addChild(plwItem); \
    mSubConfigs.append( var );  \
    pWig->addWidget( var );     \
}while(0)

H2Robo::H2Robo(QStackedWidget *pWig, QString strDevInfo, QObject *pObj ) : XRobo( pWig, pObj )
{
    Q_ASSERT( NULL != pWig );

    //! roboNode
    m_pProduct = new H2Product(strDevInfo);
    Q_ASSERT( NULL != m_pProduct );
    m_pRoboNode = new QTreeWidgetItem();
    m_pRoboNode->setText( 0, getDeviceName(strDevInfo) );
    m_pRoboNode->setIcon( 0, QIcon( ":/res/image/h2product/offline.png" ) );
    m_pRoboNode->setData( 0, Qt::UserRole, QVariant( QVariant::fromValue(m_pProduct) ) );
    mSubConfigs.append( m_pProduct );
    pWig->addWidget( m_pProduct );

    new_widget( H2Configuration, m_pH2Configuration, ":/res/image/icon/205.png" );
    new_widget( H2Measurement, m_pH2Measurement, ":/res/image/icon/54.png");
    new_widget( H2Homing, m_pH2Homing , ":/res/image/icon/address.png");
    new_widget( H2JogMode, m_pH2JogMode, ":/res/image/icon/409.png" );
    new_widget( H2Action, m_pH2Action, ":/res/image/icon/activity.png" );
    new_widget( H2ErrMgr, m_pH2ErrMgr, ":/res/image/icon/remind.png" );

    translateUI();

    //! connection
    buildConnection();
}

H2Robo::~H2Robo()
{
    delete m_pProduct;
    delete m_pH2Configuration;
    delete m_pH2Measurement;
    delete m_pH2Homing;
    delete m_pH2JogMode;
    delete m_pH2Action;
    delete m_pH2ErrMgr;
}

void H2Robo::translateUI()
{
    m_pRoboNode->child(0)->setText(0, tr("Configuration"));
    m_pRoboNode->child(1)->setText(0, tr("Measurements"));
    m_pRoboNode->child(2)->setText(0, tr("Homing"));
    m_pRoboNode->child(3)->setText(0, tr("Jog Mode"));
    m_pRoboNode->child(4)->setText(0, tr("Record Table"));
    m_pRoboNode->child(5)->setText(0, tr("Error Management"));
}

void H2Robo::buildConnection()
{
    connect(m_pProduct,SIGNAL(signal_online_clicked(QString)),this,SIGNAL(signal_online_request(QString)));

    connect(m_pH2Measurement,SIGNAL(signal_AxesZeroPoint_currentTextChanged(QString)),
            m_pH2Homing,SLOT(slot_set_direction(QString)));

    connect(m_pH2Configuration, SIGNAL(WorkStrokeXChanged(double)),
            m_pH2Measurement,SLOT(setWorkStrokeX(double)));

    connect(m_pH2Configuration, SIGNAL(WorkStrokeYChanged(double)),
            m_pH2Measurement,SLOT(setWorkStrokeY(double)));


    connect(m_pH2Configuration, SIGNAL(WorkStrokeXChanged(double)),
            m_pH2Action,SLOT(setWorkStrokeX(double)));

    connect(m_pH2Configuration, SIGNAL(WorkStrokeYChanged(double)),
            m_pH2Action,SLOT(setWorkStrokeY(double)));


    connect(m_pH2Action,SIGNAL(signalCurrentRowChanged(int)),this,SIGNAL(signal_action_selected(int)));
}

QTreeWidgetItem *H2Robo::roboNode()
{
    return m_pRoboNode;
}

QList<XConfig *> H2Robo::subConfigs() const
{
    return mSubConfigs;
}

QString H2Robo::getDeviceName(QString strDevInfo)
{
    QStringList strListDev = strDevInfo.split(',', QString::SkipEmptyParts);
    return strListDev.at(2) + "[" + strListDev.at(0) + "]";
}



void H2Robo::change_online_status(bool bl)
{

    m_pProduct->change_online_status(bl);

    if(bl)
    {   //online
        m_pRoboNode->setIcon( 0, QIcon( ":/res/image/h2product/online.png" ) );
    }
    else
    {   //offline
        m_pRoboNode->setIcon( 0, QIcon( ":/res/image/h2product/offline.png" ) );
    }
}

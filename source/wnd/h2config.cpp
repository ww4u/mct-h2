#include "h2config.h"
#include "ui_h2config.h"

#include "h2product.h"
#include "h2configuration.h"
#include "h2jogmode.h"
#include "h2homing.h"
#include "h2measurement.h"
#include "h2action.h"
#include "h2errmgr.h"

#include "../include/mystd.h"

#define new_widget( type, name, title, icon ) type *name = new type();\
                                        plwItem = new QTreeWidgetItem();\
                                        plwItem->setText( 0, title ); \
                                        plwItem->setIcon( 0, QIcon( icon) ); \
                                        plwItem->setData( 0, Qt::UserRole, QVariant( ui->stackedWidget->count() ) ); \
                                        pRoboNode->addChild( plwItem );\
                                        ui->stackedWidget->addWidget( name );

H2Config::H2Config(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::H2Config)
{
    ui->setupUi(this);

//    //! build data
//    for ( int i = 0; i < 31; i++ )
//    { mActions.insertRow( i ); }

    //! load data
    loadDataset();

    m_pRootNode = new QTreeWidgetItem();
    m_pRootNode->setText( 0, "Project" );
    ui->treeWidget->addTopLevelItem( m_pRootNode );
    m_strListRobot.clear();
}

void H2Config::slotAddNewRobot( const QString &strIP)
{
    if(m_strListRobot.contains(strIP))
        return;
    else
        m_strListRobot << strIP;

    QTreeWidgetItem *pRoboNode;
    pRoboNode = new QTreeWidgetItem();
    pRoboNode->setText( 0, "MRH-H2[" + strIP + "]" );
    pRoboNode->setIcon( 0, QIcon("") );
    pRoboNode->setData( 0, Qt::UserRole, QVariant( ui->stackedWidget->count() ) );
    m_pRootNode->addChild( pRoboNode );
    H2Product *pProduct = new H2Product();
    ui->stackedWidget->addWidget( pProduct );
    m_pRootNode->setExpanded(true);
    pRoboNode->setExpanded(true);

    //! list item
    QTreeWidgetItem *plwItem;

    //! configuration
    new_widget( H2Configuration, pConfiguration , tr("Configuration"), ":/res/image/icon/205.png" );

    //! Measurement
    new_widget( H2Measurement, pMeasurement , tr("Measurements") , ":/res/image/icon/54.png");

    //! Homing
    new_widget( H2Homing, pHoming , tr("Homing") , ":/res/image/icon/address.png");

    //! jog mode
    new_widget( H2JogMode, pJogMode , tr("Jog Mode"), ":/res/image/icon/409.png" );

    //! action
    new_widget( H2Action, pAction, tr("Record Table"), ":/res/image/icon/activity.png" )
    pAction->setModel( &mActions );

    //! err mgr
    new_widget( H2ErrMgr, pErrMgr, tr("Error Management"), ":/res/image/icon/remind.png" );
    pErrMgr->setModel( &mErrManager );

    //! connect
    connect( ui->treeWidget, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
             this, SLOT(slot_current_changed(QTreeWidgetItem*,QTreeWidgetItem*)));

    //! for each
    XConfig *pCfg;
    for ( int i = 0; i < ui->stackedWidget->count(); i++ )
    {
        pCfg = (XConfig*)ui->stackedWidget->widget( i );
        if ( NULL != pCfg )
        {
            connect( pCfg, SIGNAL(signal_focus_in( const QString &)),
                     this, SIGNAL(signal_focus_in( const QString &)));
        }
    }
}




H2Config::~H2Config()
{
    delete ui;
}

int H2Config::open()
{
    //! \todo open the device
//    mHandle = vi

    //! attach
    XConfig *pCfg;
    for ( int i = 0; i < ui->stackedWidget->count(); i++ )
    {
        pCfg = (XConfig*)ui->stackedWidget->widget( i );
        pCfg->attachHandle( mHandle );
    }

    return 0;
}
int H2Config::close()
{
    //! \todo close device

    XConfig *pCfg;
    for ( int i = 0; i < ui->stackedWidget->count(); i++ )
    {
        pCfg = (XConfig*)ui->stackedWidget->widget( i );
        pCfg->detachHandle( mHandle );
    }

    return 0;
}

int H2Config::loadDataset()
{
    //! load action
    mActions.input( QApplication::applicationDirPath() + "/dataset/mrx-h2_action.csv");

    //! load event
    //!
    mErrManager.load( QApplication::applicationDirPath() + "/dataset/mrx-h2_errmgr.xml");

    return 0;
}

int H2Config::setApply()
{
    int ret;
    XConfig *pCfg;
    for ( int i = 0; i < ui->stackedWidget->count(); i++ )
    {
        pCfg = (XConfig*)ui->stackedWidget->widget( i );
        Q_ASSERT( NULL != pCfg );

        ret = pCfg->setApply( m_visa );      //! \todo get from handle
        if ( ret != 0 )
        { return ret; }
    }

    return 0;
}

int H2Config::setReset()
{
    //! \todo for each prop

    return 0;
}
int H2Config::setOK()
{
    //! setApply


    //! close

    return 0;
}

void H2Config::on_buttonBox_clicked(QAbstractButton *button)
{
    Q_ASSERT( NULL != button );

    QDialogButtonBox::ButtonRole role = ui->buttonBox->buttonRole( button );
    if ( QDialogButtonBox::ResetRole == role )
    {

    }
    else if ( QDialogButtonBox::AcceptRole == role )
    {

    }
    else if ( QDialogButtonBox::ApplyRole == role )
    {
        setApply();
    }
    else
    {}
}

void H2Config::slot_current_changed( QTreeWidgetItem* cur,QTreeWidgetItem* prv )
{
    if ( cur != NULL )
    {}
    else
    { return; }

    QVariant var;
    var = cur->data( 0, Qt::UserRole );
    if ( var.isValid() && var.type() == QVariant::Int  )
    {
        ui->stackedWidget->setCurrentIndex( var.toInt() );
    }
}

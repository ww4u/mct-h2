#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "../include/mystd.h"

MainWindow *MainWindow::_pBackendProxy = NULL;

void MainWindow::requestLogout( const QString &str, log_level lev )
{
    if( NULL != MainWindow::_pBackendProxy )
    {}
    else
    { return; }

    MainWindow::_pBackendProxy->slot_logout( str, lev );
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->actionSearch,SIGNAL(triggered(bool)),this,SLOT(slot_action_search()));

    m_pHelpPanel = NULL;
    m_megaSerachWidget = NULL;

    setupWorkArea();

    setupToolBar();

    setupStatusBar();

    buildConnection();

    //! register the proxy
    MainWindow::_pBackendProxy = this;

}

MainWindow::~MainWindow()
{
    delete ui;

    if ( NULL != m_pHelpPanel )
    { delete m_pHelpPanel; }
}

void MainWindow::setupWorkArea()
{
    //! pref
    m_roboConfig = new RoboConfig(this);
    connect( m_roboConfig, SIGNAL(signal_focus_in( const QString &)),
             this, SLOT(slot_focus_in(const QString &)) );

    //! docks
    ui->centralWidget->insertTab( 0, m_roboConfig, tr("Pref") );

    //! dock
    m_pDockOps = new QDockWidget( tr("Ops"), this );
    m_pDockOps->setAllowedAreas(  Qt::BottomDockWidgetArea );
    m_pDockOps->setFeatures( QDockWidget::DockWidgetVerticalTitleBar
                           | m_pDockOps->features() );
    addDockWidget( Qt::BottomDockWidgetArea, m_pDockOps );

    m_pOps = new H2Ops();
    m_pDockOps->setWidget( m_pOps );
    connect( m_pOps, SIGNAL(signal_focus_in( const QString &)),
             this, SLOT(slot_focus_in(const QString &)) );

    connect(m_roboConfig,SIGNAL(signalCurrentRobotChanged(QString,int,int)),m_pOps,SLOT(slotSetCurrentRobot(QString,int,int)));
    connect(m_roboConfig,SIGNAL(signalCurrentRobotChanged(QString,int,int)),this,SLOT(slotSetDockOpsName(QString,int,int)));

    ui->menuView->addAction( m_pDockOps->toggleViewAction() );

    //! help
    m_pDockHelp = new QDockWidget( tr("Help"), this  );
    m_pDockHelp->setAllowedAreas(  Qt::RightDockWidgetArea );
    addDockWidget( Qt::RightDockWidgetArea, m_pDockHelp );

    m_pHelpPanel = new HelpPanel();
    m_pDockHelp->setWidget( m_pHelpPanel );
    ui->menuHelp->addAction( m_pDockHelp->toggleViewAction() );
}

void MainWindow::setupToolBar()
{
    ui->mainToolBar->addAction( ui->actionStop );
    ui->mainToolBar->addSeparator();
    ui->mainToolBar->addAction( ui->actionDownload );
    ui->mainToolBar->addAction( ui->actionUpload );
    ui->mainToolBar->addAction( ui->actionStore );
    ui->mainToolBar->addAction( ui->actionSync );
    ui->mainToolBar->addSeparator();
    ui->mainToolBar->addAction( ui->actionConnect );
}

void MainWindow::setupStatusBar()
{
    m_pLabStatus = new QLabel();
    m_pLabMctVer = new QLabel( QString("%1:%2").arg( ( qApp->applicationName() ) ).arg( qApp->applicationVersion() ) );
    m_pLabConVer = new QLabel();

    ui->statusBar->insertWidget( 0, m_pLabStatus, 1 );
    ui->statusBar->insertWidget( 1, m_pLabMctVer, 0 );
    ui->statusBar->insertWidget( 2, m_pLabConVer, 0 );
}

void MainWindow::buildConnection()
{
    QTimer::singleShot( 0, this, SLOT(slot_post_startup()));
}

void MainWindow::slot_post_startup()
{
    slot_logout( tr("start completed") );
    slot_logout( tr("start warning"), e_log_warning );
    slot_logout( tr("start error"), e_log_error );
}

void MainWindow::slot_logout( const QString &str, log_level lev )
{
    Q_ASSERT( NULL != m_pOps );

    m_pOps->outConsole( str, lev );
}

void MainWindow::slot_focus_in( const QString &name )
{
//    logDbg() << name;

    if ( name.length() <= 0 )
    { return; }

    if ( m_pHelpPanel == NULL )
    { return; }

    m_pHelpPanel->setFile( "./" + name + ".html" );
}

void MainWindow::slot_action_search()
{
    if(m_megaSerachWidget != NULL)
        delete m_megaSerachWidget;

    m_megaSerachWidget = new MegaInterface;
    m_megaSerachWidget->move(x()+100,y()+100);
    m_megaSerachWidget->show();
    connect(m_megaSerachWidget, SIGNAL(signal_selected_info(QString)), m_roboConfig, SLOT(slotAddNewRobot(QString)));
}

void MainWindow::on_actionAbout_triggered()
{
    aboutDlg dlg(this);
    dlg.exec();
}

void MainWindow::slotSetDockOpsName(QString strDevInfo, int visa, int name)
{
    QStringList strListDev = strDevInfo.split(',', QString::SkipEmptyParts);
    QString strDeviceName = strListDev.at(2) + "[" + strListDev.at(0) + "]";
    m_pDockOps->setWindowTitle("Ops: " + strDeviceName);
}



#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow *MainWindow::_pBackendProxy = NULL;

void MainWindow::requestLogout( const QString &str, LogLevel lev )
{
    if( NULL == MainWindow::_pBackendProxy )
    { return; }

    MainWindow::_pBackendProxy->slot_logout( str, lev );
}

void MainWindow::showStatus( const QString str)
{
    if( NULL == MainWindow::_pBackendProxy )
    { return; }

    MainWindow::_pBackendProxy->slotUpdateStatus(str);
}

void MainWindow::showProgressBar(bool isRun)
{
    if( NULL == MainWindow::_pBackendProxy )
    { return; }

    MainWindow::_pBackendProxy->slotShowProgressStatus(isRun);
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_pLabStatus    = NULL;
    m_pLabMctVer    = NULL;
    m_progressBar   = NULL;
    m_roboConfig    = NULL;
    m_pDockOps      = NULL;
    m_pOps          = NULL;
    m_pDockHelp     = NULL;
    m_pHelpPanel    = NULL;

#ifndef _WIN32
    //! 获取MRHT支持的分辨率
    system("/opt/vc/bin/tvservice -m DMT > /home/megarobo/.tvConfig");
    system("chmod 666 /home/megarobo/.tvConfig");
    QString str = readFile("/home/megarobo/.tvConfig");
    foreach (QString strLine, str.split("\n", QString::SkipEmptyParts)) {
        if(strLine.contains("@") && strLine.contains("clock:")){
            QRegExp rx("(\\d+):\\s+([^,]+),");
            rx.indexIn(strLine);
            m_mapDpiTable.insert(rx.cap(2), rx.cap(1).toInt());
        }
    }
#endif

    setupWorkArea();

    setupToolBar();

    setupStatusBar();

    buildConnection();

    loadConfig();

    //! register the proxy
    MainWindow::_pBackendProxy = this;
}

MainWindow::~MainWindow()
{
    delete m_pLabStatus;
    delete m_progressBar;
    delete m_pLabMctVer;

    delete m_roboConfig;

    delete m_pOps;
    delete m_pDockOps;

    if ( NULL != m_pHelpPanel )
    { delete m_pHelpPanel; }
    delete m_pDockHelp;

    delete ui;
}

void MainWindow::setupWorkArea()
{
    //! pref
    m_roboConfig = new RoboConfig(this);
    ui->centralWidget->insertTab( 0, m_roboConfig, tr("Pref") );

    //! dock
    m_pDockOps = new QDockWidget( tr("Ops"), this );
    m_pDockOps->setAllowedAreas(  Qt::BottomDockWidgetArea );
    m_pDockOps->setFeatures( QDockWidget::DockWidgetVerticalTitleBar | m_pDockOps->features() );
    addDockWidget( Qt::BottomDockWidgetArea, m_pDockOps );

    m_pOps = new H2Ops();
    m_pDockOps->setWidget( m_pOps );
    ui->menuView->addAction( m_pDockOps->toggleViewAction() );

    //! help
    m_pDockHelp = new QDockWidget( tr("Help"), this  );
    m_pDockHelp->setAllowedAreas(  Qt::RightDockWidgetArea );
    addDockWidget( Qt::RightDockWidgetArea, m_pDockHelp );

    m_pHelpPanel = new HelpPanel();
    m_pDockHelp->setWidget( m_pHelpPanel );    
    m_pDockHelp->toggleViewAction()->setText(tr("&ShowHelp"));
    ui->menuHelp->addAction( m_pDockHelp->toggleViewAction() );
    m_pDockHelp->hide();//默认不显示
}

void MainWindow::setupToolBar()
{
    ui->mainToolBar->addSeparator();
    ui->mainToolBar->addAction( ui->actionDownload );
    ui->mainToolBar->addAction( ui->actionUpload );
    ui->mainToolBar->addAction( ui->actionStore );
//    ui->mainToolBar->addAction( ui->actionSync );
    ui->mainToolBar->addSeparator();
    ui->mainToolBar->addAction( ui->actionSearch );
    ui->mainToolBar->addSeparator();
    ui->mainToolBar->addAction( ui->actionConnect );
    ui->mainToolBar->addSeparator();

    //! add stop button
    QWidget *t_widget = new QWidget();
    QHBoxLayout *t_hBoxLayout = new QHBoxLayout();
    t_widget->setLayout(t_hBoxLayout);
    ui->mainToolBar->addWidget(t_widget);

    m_stopButton =  new QToolButton();
    connect(m_stopButton,SIGNAL(clicked(bool)),ui->actionStop,SLOT(trigger()));
    m_stopButton->setIcon(QIcon(":/res/image/icon/stop.png"));
    m_stopButton->setIconSize(QSize(75,50));
    m_stopButton->setText(tr("STOP"));
//    m_stopButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    t_hBoxLayout->addStretch();
    t_hBoxLayout->addWidget(m_stopButton);

#ifdef _WIN32
    //! windows禁用这些功能
    ui->actionReboot->setVisible(false);
    ui->actionPoweroff->setVisible(false);
    ui->actionDPI->setVisible(false);
    ui->actionWifi->setVisible(false);
#endif

}

void MainWindow::setupStatusBar()
{
    qDebug() << "version" << qApp->applicationVersion();
    m_pLabStatus = new QLabel("MegaRobo Configuration Tool");
    m_progressBar = new QProgressBar;
    m_progressBar->setMaximum(100);
    m_progressBar->setHidden(true);
    m_pLabMctVer = new QLabel( QString("Version: %1  ").arg( qApp->applicationVersion() ) );

    ui->statusBar->insertWidget( 0, m_pLabStatus, 1);
    ui->statusBar->insertWidget( 1, m_progressBar, 0);
    ui->statusBar->insertWidget( 2, m_pLabMctVer, 0);
}

void MainWindow::slotUpdateStatus( const QString str )
{
    m_pLabStatus->setText(str);
}

void MainWindow::slotShowProgressStatus( bool isRun )
{
    if(isRun) {
        m_progressBar->setHidden(false);
        m_progressBar->setMaximum(0);
    } else {
        m_progressBar->setMaximum(100);
        m_progressBar->setHidden(true);
    }
}

void MainWindow::buildConnection()
{
    connect( m_pOps, SIGNAL(signal_focus_in( const QString &)),
             this, SLOT(slot_focus_in(const QString &)) );

    connect( m_roboConfig, SIGNAL(signal_focus_in( const QString &)),
             this, SLOT(slot_focus_in(const QString &)) );

    connect(m_roboConfig,SIGNAL(signalCurrentRobotChanged(QString,int,int,int)),
            this,SLOT(slotCurrentRobotChanged(QString,int,int,int)));

    connect(m_roboConfig,SIGNAL(signalCurrentRobotChanged(QString,int,int,int)),
            m_pOps,SLOT(slotSetCurrentRobot(QString,int,int,int)));

    connect(m_roboConfig,SIGNAL(signalDataChanged()),
            m_pOps,SLOT(slotLoadConfigAgain()));

    connect(m_roboConfig,SIGNAL(signal_record_selected(int)),
            m_pOps,SLOT(slotSetCurrentRecordNumber(int)));

    connect(m_pOps,SIGNAL(signal_apply_point(int,QString,double,double,double,double)),
            m_roboConfig,SLOT(slotSetOneRecord(int,QString,double,double,double,double)));

    connect(ui->actionStop,SIGNAL(triggered(bool)), m_pOps, SLOT(slotRobotStop()));

    connect(ui->actionDownload,SIGNAL(triggered(bool)), m_roboConfig, SLOT(slotDownload()));
    connect(ui->actionUpload,SIGNAL(triggered(bool)), m_roboConfig, SLOT(slotUpload()));
    connect(ui->actionStore,SIGNAL(triggered(bool)), m_roboConfig, SLOT(slotStore()));
    connect(ui->actionSync,SIGNAL(triggered(bool)), m_roboConfig, SLOT(slotSync()));
    connect(ui->actionSearch,SIGNAL(triggered(bool)), m_roboConfig, SLOT(slotSearch()));
    connect(ui->actionConnect,SIGNAL(triggered(bool)), m_roboConfig, SLOT(slotConnect()));

}

void MainWindow::loadConfig()
{
    MegaXML mXML;
    QString fileName = MCTHomeDIR() + "/config.xml";
    QMap<QString,QString> map = mXML.xmlRead(fileName);
    if(map["Style"] == ""){
        map.clear();
        map.insert("Style", "MegaRobo");
        m_style = STYLE_MEGAROBO;
        mXML.xmlNodeRemove(fileName, "WindowStyle");
        mXML.xmlNodeAppend(fileName, "WindowStyle", map);
    }

    if(map["Language"] == ""){
        map.clear();
        map.insert("Language", "English");
        m_language = LANG_EN;
        mXML.xmlNodeRemove(fileName, "WindowLanguage");
        mXML.xmlNodeAppend(fileName, "WindowLanguage", map);
    }

    map = mXML.xmlRead(fileName);
    if(map["Language"] == "English"){
        //默认缺省
        on_actionEnglish_triggered();
        on_actionEnglish_triggered();
    }else{
        on_actionChinese_triggered();
        on_actionChinese_triggered();
    }

    if(map["Style"] == "MegaRobo"){
        on_actionMega_triggered();
        on_actionMega_triggered();
    }else{
        on_actionClassic_triggered();
        on_actionClassic_triggered();
    }
}

void MainWindow::on_actionAbout_triggered()
{
    aboutDlg dlg(this);
    dlg.exec();
}

void MainWindow::on_actionChinese_triggered()
{
    if( !ui->actionEnglish->isChecked() && !ui->actionChinese->isChecked()){
        ui->actionChinese->setChecked(true);
        return;
    }
    ui->actionEnglish->setChecked(false);

    m_language = LANG_CN;
    changeLanguage();
    changeLanguage();
}

void MainWindow::on_actionEnglish_triggered()
{
    if( !ui->actionEnglish->isChecked() && !ui->actionChinese->isChecked()){
        ui->actionEnglish->setChecked(true);
        return;
    }

    ui->actionChinese->setChecked(false);

    m_language = LANG_EN;
    changeLanguage();
    changeLanguage();
}

void MainWindow::changeLanguage()
{
    QString qmFile = "";
    MegaXML mXML;
    QString fileName = MCTHomeDIR() + "/config.xml";
    QMap<QString,QString> map;

    if( m_language == LANG_CN ){
        qmFile = ":/res/ts/qt_CN.qm";
        map.insert("Language", "Chinese");
    }
    else if( m_language == LANG_EN ){
        qmFile = ":/res/ts/qt_EN.qm";
        map.insert("Language", "English");
    }
    qDebug() << "changeLanguage:" << qmFile;

    m_roboConfig->changeLanguage(qmFile);
    m_pOps->changeLanguage(qmFile);

    qApp->removeTranslator(&m_translator);
    m_translator.load(qmFile);
    qApp->installTranslator(&m_translator);
    ui->retranslateUi(this);
    m_pDockHelp->toggleViewAction()->setText(tr("&ShowHelp"));

    mXML.xmlNodeRemove(fileName, "WindowLanguage");
    mXML.xmlNodeAppend(fileName, "WindowLanguage", map);

    //update show
    QStringList strListDev = m_strDevInfo.split(',', QString::SkipEmptyParts);
    if(strListDev.length() > 3){
        QString strDeviceName = strListDev.at(2) + "[" + strListDev.at(0) + "]";
        m_pDockOps->setWindowTitle("Ops: " + strDeviceName);
        ui->actionConnect->setText(strDeviceName);
    }
}

void MainWindow::on_actionMega_triggered()
{
    if( !ui->actionClassic->isChecked() && !ui->actionMega->isChecked()){
        ui->actionMega->setChecked(true);
        return;
    }
    ui->actionClassic->setChecked(false);

    m_style = STYLE_MEGAROBO;
    setUiStyle(":/res/qss/mega.qss");
}

void MainWindow::on_actionClassic_triggered()
{
    if( !ui->actionClassic->isChecked() && !ui->actionMega->isChecked()){
        ui->actionClassic->setChecked(true);
        return;
    }

    ui->actionMega->setChecked(false);
    m_style = STYLE_CLASSIC;

    setUiStyle(":/res/qss/classic.qss");
}

void MainWindow::setUiStyle(const QString &styleFile)
{
    MegaXML mXML;
    QString fileName = MCTHomeDIR() + "/config.xml";
    QMap<QString,QString> map;
    if( m_style == STYLE_MEGAROBO){
        map.insert("Style", "MegaRobo");
    }else{
        map.insert("Style", "Classic");
    }

    mXML.xmlNodeRemove(fileName, "WindowStyle");
    mXML.xmlNodeAppend(fileName, "WindowStyle", map);

    if( ! QFile::exists(styleFile) )
    {
        qDebug() << "setStyleSheet file not exists!";
        return;
    }

    QFile qss(styleFile);
    if( qss.open(QFile::ReadOnly) ){
        qApp->setStyleSheet(qss.readAll());
        qss.close();

        qDebug() << "setStyleSheet:" << styleFile;
    }
}

void MainWindow::slotCurrentRobotChanged(QString strDevInfo, int visa, int deviceName,int roboName)
{
    if( (m_strDevInfo == strDevInfo) && (m_ViHandle == visa) && (m_RoboName == roboName) ){
        return;//没有切换机器人且状态没有改变
    }

    m_strDevInfo = strDevInfo;
    m_ViHandle = visa;
    m_DeviceName = deviceName;
    m_RoboName = roboName;

    QStringList strListDev = m_strDevInfo.split(',', QString::SkipEmptyParts);
    if(strListDev.length() > 3){
        QString strDeviceName = strListDev.at(2) + "[" + strListDev.at(0) + "]";
        m_pDockOps->setWindowTitle("Ops: " + strDeviceName);
        ui->actionConnect->setText(strDeviceName);
    }

    if(m_ViHandle == 0){
        //device closed
        ui->actionConnect->setIcon(QIcon(":/res/image/h2product/offline.png"));
    }
    else{
        //device opened
        ui->actionConnect->setIcon(QIcon(":/res/image/h2product/online.png"));
    }
}

void MainWindow::slot_logout( const QString &str, LogLevel lev )
{
    Q_ASSERT( NULL != m_pOps );
    m_pOps->outConsole( str, lev );
}

void MainWindow::slot_focus_in( const QString &name )
{
    QString strName =  QApplication::applicationDirPath() + "/doc/" + name + ".html";
//    logDbg() << name;
    if ( name.length() <= 0 )
    { return; }

    if ( m_pHelpPanel == NULL )
    { return; }

    m_pHelpPanel->setFile( strName );
}

void MainWindow::on_actionExit_triggered()
{
    this->closeEvent(NULL);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    m_roboConfig->slotExit();
    close();
}


void MainWindow::on_actionPoweroff_triggered()
{
    m_roboConfig->slotExit();
    QThread::msleep(1000);
#ifndef _WIN32
    system("poweroff");
#else
    system("shutdown -s -t 0");
#endif
}

void MainWindow::on_actionReboot_triggered()
{
    m_roboConfig->slotExit();
    QThread::msleep(1000);
#ifndef _WIN32
    system("reboot");
#else
    system("shutdown -s -r 0");
#endif
}

void MainWindow::on_actionWifi_triggered()
{
    m_roboConfig->slotWifi();
}

void MainWindow::on_actionUpdateFirmware_triggered()
{
    m_roboConfig->slotUpdateFirmware();
}

void MainWindow::on_actionDPI_triggered()
{
    if(m_mapDpiTable.isEmpty())
        return;

    QStringList dpiList;
    QMap<QString,int>::iterator itMap;
    for ( itMap=m_mapDpiTable.begin(); itMap != m_mapDpiTable.end(); ++itMap ) {
        dpiList << itMap.key();
    }

    //显示选择对话框
    qSort(dpiList);
    QString dpiItem = QInputDialog::getItem(this, tr("DPI"), tr("Please choose DPI:"), dpiList, -1, false);
    if(dpiItem == "")
        return;

    int dpiConfig = m_mapDpiTable[dpiItem];
    qDebug() << "selected DPI:" << dpiItem << dpiConfig;

    //! TODO 修改/boot/config.txt 的hdmi_mode=
    bool bl = updateConfigFile("/boot/config.txt", "hdmi_mode", QString::number(dpiConfig));
    if(bl){
        QMessageBox::information(this,tr("tips"),tr("Modify DPI success!") + "\n" + tr("Effective after system reboot"));
    }else{
        QMessageBox::critical(this,tr("error"),tr("Modify DPI failured!"));
    }

    return;
}



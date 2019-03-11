#include "h2ops.h"
#include "ui_h2ops.h"
#include "mydebug.h"

#include <QClipboard>
#include <QTime>
#include <QFileDialog>

#include "mystd.h"
#include "sysapi.h"
#include "megamessagebox.h"
#include "megaxml.h"

#define set_name( the, name )   the->setFocuHelpName( name );\
                                mSubTabs.append( the );

H2Ops::H2Ops(QWidget *parent) :
    XConfig(parent),
    ui(new Ui::H2Ops)
{
    ui->setupUi(this);

    mViHandle = 0;
    mDeviceName = 0;
    mRobotName = 0;
    m_strDevInfo = "";
    m_speedRatio = -1;

    m_recordNumber = -1;
    m_isDebugRunFlag = false;
    m_isHomgingRunFlag = false;
    m_isPrjZeroRunFlag = false;


    set_name( ui->tab_LogOut,   "tab_LogOut");
    set_name( ui->tab_Operate,  "tab_Operate");
    set_name( ui->tab_DigitalIO,"tab_DigitalIO");
    set_name( ui->tab_Homing,   "tab_Homing");
    set_name( ui->tab_Manual,   "tab_Manual");
    set_name( ui->tab_Monitor,  "tab_Monitor");
    set_name( ui->tab_Debug,    "tab_Debug");
    set_name( ui->tab_Diagnosis,"tab_Diagnosis");

    this->setupUi();

    m_timerOpsAll = new QTimer;
    m_timerOpsAll->setInterval(2000);

    m_timerOpsOpreate = new QTimer;
    m_timerOpsOpreate->setInterval(500);

    m_timerOpsIO      = new QTimer;
    m_timerOpsIO->setInterval(500);

    m_threadOpsHoming = NULL;
    m_threadOpsPrjZero = NULL;
    m_timerOpsHoming  = new QTimer;
    m_timerOpsHoming->setInterval(500);

    m_timerOpsManual  = new QTimer;
    m_timerOpsManual->setInterval(500);

    m_timerOpsMonitor = new QTimer;
    m_timerOpsMonitor->setInterval(1000);

    m_threadOpsDebug = NULL;
    m_timerOpsDebug = new QTimer;
    m_timerOpsDebug->setInterval(500);

    buildConnection();
}

H2Ops::~H2Ops()
{
    delete m_timerOpsAll;
    delete m_timerOpsOpreate;
    delete m_timerOpsIO;
    delete m_timerOpsHoming;
    delete m_timerOpsManual;
    delete m_timerOpsMonitor;
    delete m_timerOpsDebug;
    delete mp_logClearAction;
    delete mp_logSelectAllAction;
    delete mp_logCopyAction;
    delete mp_logSepAction;
    delete m_splineChart1;
    delete m_splineChart2;

    delete m_pDiagnosisModel;
    delete m_pDebugModel;
    delete m_dblSpinboxRecord;
    delete m_dblSpinboxDelayTime;

    if(m_threadOpsHoming != NULL){
        delete m_threadOpsHoming;
    }
    if(m_threadOpsPrjZero != NULL){
        delete m_threadOpsPrjZero;
    }
    if(m_threadOpsDebug != NULL){
        delete m_threadOpsDebug;
    }

    delete ui;
}

void H2Ops::setupUi()
{
    //! actions for the logout
    mp_logClearAction = new QAction( tr("Clear"), this );
    mp_logSelectAllAction = new QAction( tr("Select All"), this );
    mp_logCopyAction = new QAction( tr("Copy"), this );

    mp_logSepAction = new QAction( this );
    mp_logSepAction->setSeparator( true );

    QList<QAction*> actions;
    actions<<mp_logSelectAllAction<<mp_logCopyAction<<mp_logSepAction<<mp_logClearAction;

    ui->lstLogout->addActions( actions );

    //! monitor
    m_splineChart1 = new MegaSplineChart(tr("Energy1"));
    m_splineChart2 = new MegaSplineChart(tr("Energy2"));

    m_splineChart1->chart()->series1()->setPen(QPen(Qt::red));
    m_splineChart1->chart()->series2()->setPen(QPen(Qt::blue));

    m_splineChart2->chart()->series1()->setPen(QPen(Qt::red));
    m_splineChart2->chart()->series2()->setPen(QPen(Qt::blue));

    ui->horizontalLayout_3->addWidget(m_splineChart1);
    ui->horizontalLayout_3->addWidget(m_splineChart2);
    ui->horizontalLayout_3->addStretch();

    //! model
    m_pDiagnosisModel= new DiagnosisModel();
    ui->tvDiagnosis->setModel( m_pDiagnosisModel );

    {
        m_pDebugModel = new DebugModel();
        ui->tvDebug->setModel( m_pDebugModel );

        m_dblSpinboxRecord = new DoubleSpinBoxDelegate( 0, 1, 31 ,this);
        ui->tvDebug->setItemDelegateForColumn( 0, m_dblSpinboxRecord );

        m_dblSpinboxDelayTime = new DoubleSpinBoxDelegate( 2, 0, 999 , this);
        ui->tvDebug->setItemDelegateForColumn( 1, m_dblSpinboxDelayTime );
    }

    ui->tabWidget->setCurrentIndex(0);
    for(int i=1; i<ui->tabWidget->count(); i++){
        ui->tabWidget->setTabEnabled(i, false);
    }

}

void H2Ops::changeLanguage(QString qmFile)
{
    ui->h2Status->changeLanguage(qmFile);
    m_splineChart1->changeLanguage(qmFile);
    m_splineChart2->changeLanguage(qmFile);

    qApp->removeTranslator(&m_translator);
    m_translator.load(qmFile);
    qApp->installTranslator(&m_translator);
    ui->retranslateUi(this);
}

void H2Ops::buildConnection()
{
    connect(ui->h2Status,SIGNAL(signal_mct_checked(bool)),
            this,SLOT(slot_mct_checked(bool)));

    connect(ui->h2Status,SIGNAL(signal_power_checked(bool)),
            this,SLOT(slot_power_checked(bool)));


    connect(ui->h2Status,SIGNAL(signal_btnAckError_clicked()),
            this,SLOT(slot_btnAckError_clicked()));


    //! connect
    connect( mp_logClearAction, SIGNAL(triggered(bool)),
             this, SLOT(slot_logClear_action()) );
    connect( mp_logSelectAllAction, SIGNAL(triggered(bool)),
             this, SLOT(slot_logSelectAll_action()) );
    connect( mp_logCopyAction, SIGNAL(triggered(bool)),
             this, SLOT(slot_logCopy_action()) );

    //! option
    foreach( XConfig *pCfg, mSubTabs ){
        connect( pCfg, SIGNAL(signal_focus_in(const QString &)),
                 this, SIGNAL(signal_focus_in(const QString &)));
    }

    connect(m_timerOpsAll, SIGNAL(timeout()), this, SLOT(updateBackgroundStatus()));
    connect(m_timerOpsOpreate, SIGNAL(timeout()), this, SLOT(updateTabOpreate()));
    connect(m_timerOpsIO, SIGNAL(timeout()), this, SLOT(updateTabIO()));
    connect(m_timerOpsHoming, SIGNAL(timeout()), this, SLOT(updateTabHoming()));
    connect(m_timerOpsManual, SIGNAL(timeout()), this, SLOT(updateTabManual()));
    connect(m_timerOpsMonitor, SIGNAL(timeout()), this, SLOT(updateTabMonitor()));
    connect(m_timerOpsDebug, SIGNAL(timeout()), this, SLOT(updateTabDebug()));
}

void H2Ops::outConsole( const QString &str, LogLevel e )
{
    QString fmtStr = QDateTime::currentDateTime().toString("hh:mm:ss.zzz") + "   " + str;

    if ( e == eLogInfo )
    { outInfo( fmtStr); }
    else if ( e == eLogWarning )
    { outWarning( fmtStr ); }
    else if ( e == eLogError )
    { outError( fmtStr ); }
    else
    {}

    ui->lstLogout->scrollToBottom();
}
void H2Ops::outInfo( const QString &str )
{
    QListWidgetItem *pItem = new QListWidgetItem( QIcon(":/res/image/icon/xiaoxi.png"), str );
    ui->lstLogout->addItem( pItem );

}
void H2Ops::outWarning( const QString &str )
{
    QListWidgetItem *pItem = new QListWidgetItem( QIcon(":/res/image/icon/gantanhaozhong.png"), str );
    ui->lstLogout->addItem( pItem );
}
void H2Ops::outError( const QString &str )
{
    QListWidgetItem *pItem = new QListWidgetItem( QIcon(":/res/image/icon/shibai2.png"), str );
    ui->lstLogout->addItem( pItem );
}

void H2Ops::slot_logSelectAll_action()
{
    ui->lstLogout->selectAll();
}
void H2Ops::slot_logCopy_action()
{
    //! cat text
    QString str;
    for ( int i = 0; i < ui->lstLogout->count(); i++ )
    {
        if ( ui->lstLogout->item(i)->isSelected() )
        {
            str.append( ui->lstLogout->item( i )->text() );
            str.append( '\n' );
        }
    }

    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText( str );
}
void H2Ops::slot_logClear_action()
{
    ui->lstLogout->clear();
}


void H2Ops::slotSetCurrentRobot(QString strDevInfo, int visa, int deviceName, int roboName)
{
    if( (m_strDevInfo == strDevInfo) && (mViHandle == visa) && (mRobotName == roboName) ){
        return;//没有切换机器人且状态没有改变
    }

    m_strDevInfo = strDevInfo;

    mViHandle = visa;
    mDeviceName = deviceName;
    mRobotName = roboName;
    mConfigFileName = m_strDevInfo.split(',').at(3);

    m_Data.clear();

    qDebug() << "H2OPS "
             << "m_ViHandle:"  << mViHandle
             << "m_DeviceName:" << mDeviceName
             << "m_RoboName:" << mRobotName;

    ui->tabWidget->setCurrentIndex(0);
    if(mViHandle <= 0)
    {   //device closed
        m_recordNumber = -1;
        ui->h2Status->on_chkMct_toggled(false);
        ui->h2Status->set_chkMct_enabled(false);
        setAllTabStopWorking();
    }
    else
    {   //device opened
        slotLoadConfigAgain();
        ui->h2Status->set_chkMct_enabled(true);
        ui->h2Status->on_chkMct_toggled(false);
    }
}

//从设备查询加减速比
double H2Ops::getSpeedRatio()
{
#if 0
    //! 临时添加
    mrgMRQPVTTimeScale(mViHandle, mDeviceName, 0, 0, 250, 250);
    mrgMRQPVTTimeScale(mViHandle, mDeviceName, 1, 0, 250, 250);
#endif

    int speedup1, speedcut1;
    int speedup2, speedcut2;
    int ret = -1;
    ret = mrgMRQPVTTimeScale_Query(mViHandle, mDeviceName, 0, 0, &speedup1, &speedcut1);
    if(ret != 0)
    {
        qDebug() << "mrgMRQPVTTimeScale_Query0 error" << ret;
        sysError("mrgMRQPVTTimeScale_Query,0,0", ret);
        return -1;
    }
    ret = mrgMRQPVTTimeScale_Query(mViHandle, mDeviceName, 1, 0, &speedup2, &speedcut2);
    if(ret != 0)
    {
        qDebug() << "mrgMRQPVTTimeScale_Query1 error" << ret;
        sysError("mrgMRQPVTTimeScale_Query,1,0", ret);
        return -2;
    }
    if((speedup1 != speedup2) || (speedcut1 != speedcut2))
    {
        QMessageBox::critical(this,tr("error"),tr("The acceleration/deceleration ratios of the two channels are different!"));
        return -3;
    }

    return ((speedup1 + speedcut1)/2 + (1000 - speedup1 - speedcut1)) / 1000.0;
}

void H2Ops::slotSetCurrentRecordNumber(int number)
{
    m_recordNumber = number;
    ui->pushButton_apply->setText(tr("Apply as point ") + QString::number(m_recordNumber));
}

void H2Ops::slotLoadConfigAgain()
{
    m_Data.clear();
    MegaXML mXML;
    QString fileName = MCTHomeDIR() + "/robots/" + mConfigFileName + ".xml";
    QFile file(fileName);
    if( !file.exists() )
        fileName = qApp->applicationDirPath() + "/robots/default.xml";

    QMap<QString,QString> map = mXML.xmlRead(fileName);
    if(map.isEmpty()){
        sysError("slotLoadConfigAgain error");
        return;
    }

    m_Data = map;
}

void H2Ops::slotRobotStop()
{
//    setAllTabStopWorking();
    int index = ui->tabWidget->currentIndex();

    if(m_isHomgingRunFlag){
        on_pushButton_starting_home_clicked();
    }

    if(m_isDebugRunFlag){
        on_toolButton_debugRun_clicked();
    }

    if(m_isPrjZeroRunFlag){
        on_pushButton_go_prjZero_clicked();
    }

    on_tabWidget_currentChanged(index);
}

void H2Ops::slot_mct_checked(bool checked)
{
    for(int i=1; i<ui->tabWidget->count(); i++){
        ui->tabWidget->setTabEnabled(i, checked);
    }

    if(!checked){
        setTimerStop(m_timerOpsAll);
    }else{
        ui->tabWidget->setTabEnabled(3, false);
        ui->tabWidget->setTabEnabled(4, false);
        ui->tabWidget->setTabEnabled(5, false);
        ui->tabWidget->setTabEnabled(6, false);

        setTimerStart(m_timerOpsAll);
    }
}

void H2Ops::slot_power_checked(bool checked)
{
    ui->tabWidget->setCurrentIndex(0);
    if(mViHandle == 0) return;

    int ret = -1;
    if(checked){
        //电机使能
        ret = mrgMRQDriverState(mViHandle, mDeviceName, 0, 1);
        qDebug() << "mrgMRQDriverState0 ON" << ret;
        if(ret < 0){
            sysError("mrgMRQDriverState0 ON error", ret);
            return;
        }

        ret = mrgMRQDriverState(mViHandle, mDeviceName, 1, 1);
        qDebug() << "mrgMRQDriverState1 ON" << ret;
        if(ret < 0){
            sysError("mrgMRQDriverState1 ON error", ret);
            return;
        }

        m_speedRatio = getSpeedRatio();
        if(m_speedRatio < 0){
            return;
        }
        qDebug() << "m_speedRatio:" << m_speedRatio;

        ui->h2Status->setActionStatus(H2Status::ACTION_READY);
        ui->h2Status->setWarningError(H2Status::STATUS_EMPTY,"");

        ui->tabWidget->setTabEnabled(3, true);
        ui->tabWidget->setTabEnabled(4, true);
        ui->tabWidget->setTabEnabled(5, true);
        ui->tabWidget->setTabEnabled(6, true);
    }else{
        //电机关闭
        ret = mrgMRQDriverState(mViHandle, mDeviceName, 0, 0);
        qDebug() << "mrgMRQDriverState0 OFF" << ret;
        ret = mrgMRQDriverState(mViHandle, mDeviceName, 1, 0);
        qDebug() << "mrgMRQDriverState1 OFF" << ret;

        ui->h2Status->setActionStatus(H2Status::ACTION_EMPTY);
        ui->h2Status->setWarningError(H2Status::STATUS_EMPTY,"");

        ui->tabWidget->setTabEnabled(3, false);
        ui->tabWidget->setTabEnabled(4, false);
        ui->tabWidget->setTabEnabled(5, false);
        ui->tabWidget->setTabEnabled(6, false);

        setAllTabStopWorking();
    }
}

void H2Ops::slot_btnAckError_clicked()
{
    if(mViHandle == 0) return;
    if(0 == mrgErrorLogClear(mViHandle)){
        QMessageBox::information(this,tr("tips"),tr("Success!"));
    }
    else{
        QMessageBox::critical(this,tr("error"),tr("Failured!"));
    }
}

void H2Ops::setAllTabStopWorking()
{
    setTimerStop(m_timerOpsOpreate);
    setTimerStop(m_timerOpsIO);
    setTimerStop(m_timerOpsHoming);
    setTimerStop(m_timerOpsManual);
    setTimerStop(m_timerOpsDebug);
    setOpsMonitorTimerStop();

    if(!m_isHomgingRunFlag && !m_isDebugRunFlag){ //不是回零过程中
        on_pushButton_stop_clicked();
    }

//    if(m_isDebugRunFlag){
//        on_toolButton_debugRun_clicked();
//    }

    if(m_isPrjZeroRunFlag){
        on_pushButton_go_prjZero_clicked();
    }
}

void H2Ops::on_tabWidget_currentChanged(int index)
{
    emit signal_focus_in( mSubTabs.at(index)->focuHelpName() );

    if(mViHandle == 0) return;

    setAllTabStopWorking();

    switch (index) {
    case 0://Logout

        break;
    case 1://Operate
        setTimerStart(m_timerOpsOpreate);
        break;
    case 2://IO
        setTimerStart(m_timerOpsIO);
        QMessageBox::information(this,tr("tips"),tr("unable"));//暂不可用
        break;
    case 3://Homing
        setTimerStart(m_timerOpsHoming);
        break;
    case 4://Manual
        setTimerStart(m_timerOpsManual);
        break;
    case 5://Monitor
        setOpsMonitorTimerStart();
        break;
    case 6://Debug
        setTimerStart(m_timerOpsDebug);
        break;
    case 7://Diagnosis

        break;

    default:
        break;
    }

}



//! debug
void H2Ops::on_btnUp_clicked()
{
    //! current valid
    if ( ui->tvDebug->currentIndex().isValid() )
    {}
    else
    { return; }

    //! check up index
    QModelIndex prev = m_pDebugModel->index( ui->tvDebug->currentIndex().row() - 1,
                                             ui->tvDebug->currentIndex().column() );
    if ( prev.isValid() )
    {}
    else
    { return; }

    DebugItem *pItem = m_pDebugModel->items()->takeAt( ui->tvDebug->currentIndex().row() );
    if ( NULL == pItem )
    { return; }

    //! exchange
    m_pDebugModel->items()->insert( ui->tvDebug->currentIndex().row() - 1, pItem );
    m_pDebugModel->signal_dataChanged(m_pDebugModel->index( ui->tvDebug->currentIndex().row()-1, 0 ),
                                      m_pDebugModel->index( ui->tvDebug->currentIndex().row(), 1 ));

    ui->tvDebug->setCurrentIndex( prev );

}

void H2Ops::on_btnDown_clicked()
{
    if ( ui->tvDebug->currentIndex().isValid() )
    {}
    else
    { return; }

    //! check next index
    QModelIndex next = m_pDebugModel->index( ui->tvDebug->currentIndex().row() + 1,
                                             ui->tvDebug->currentIndex().column() );
    if ( next.isValid() )
    {}
    else
    { return; }

    DebugItem *pItem = m_pDebugModel->items()->takeAt( ui->tvDebug->currentIndex().row() );
    if ( NULL == pItem )
    { return; }

    m_pDebugModel->items()->insert( next.row(), pItem );
    m_pDebugModel->signal_dataChanged(m_pDebugModel->index( ui->tvDebug->currentIndex().row(), 0 ),
                                        m_pDebugModel->index( ui->tvDebug->currentIndex().row()+1, 1));
    ui->tvDebug->setCurrentIndex( next );
}

void H2Ops::on_btnAdd_clicked()
{
    if(m_recordNumber == -1)
        return;

    int row = ui->tvDebug->model()->rowCount();
    m_pDebugModel->insertRow( row );
    m_pDebugModel->setData( m_pDebugModel->index( row, 0),
                            QVariant( m_recordNumber),
                            Qt::EditRole );

    m_pDebugModel->setData( m_pDebugModel->index( row, 1),
                            QVariant( ui->spinDly->value()),
                            Qt::EditRole );
}

void H2Ops::on_btnDel_clicked()
{
    if ( ui->tvDebug->currentIndex().isValid() )
    {}
    else
    { return; }

    m_pDebugModel->removeRow( ui->tvDebug->currentIndex().row() );
}

void H2Ops::on_btnClr_clicked()
{
    MegaDeleteAffirmMessageBox msgBox;
    int ret = msgBox.exec();
    if ( ret == QMessageBox::Ok )
    {
        m_pDebugModel->removeRows( 0, m_pDebugModel->items()->count(), QModelIndex() );
    }
}

void H2Ops::on_btnImport_clicked()
{
    QFileDialog fDlg;

    fDlg.setAcceptMode( QFileDialog::AcceptOpen );
    fDlg.setNameFilter( "Debug (*.xml)" );

    if ( QDialog::Accepted != fDlg.exec() )
    { return; }

    m_pDebugModel->load( fDlg.selectedFiles().first() );
}

void H2Ops::on_btnExport_clicked()
{
    QFileDialog fDlg;

    fDlg.setAcceptMode( QFileDialog::AcceptSave );
    fDlg.setNameFilter( "Debug (*.xml)" );

    if ( QDialog::Accepted != fDlg.exec() )
    { return; }

    m_pDebugModel->save( fDlg.selectedFiles().first() );
}

//! diagnosis
void H2Ops::on_btnRead_clicked()
{
    int len = 1024*40;
    char *logBuff = (char *)malloc(len);
    memset(logBuff,0,len);
    int ret = mrgErrorLogUpload(mViHandle, 1, logBuff, len );
    QString strFileData = QString("%1").arg(logBuff);
    free(logBuff);

    qDebug() << "mrgErrorLogUpload" << ret;
    if(ret < 0)
    {
        sysError("mrgErrorLogUpload");
        QMessageBox::critical(this,tr("error"), tr("Error log upload error!"));
        return;
    }
    qDebug() << "mrgErrorLogUpload Data:" << strFileData;

    QList<QStringList> lstData;
    foreach (QString str, strFileData.split('\n', QString::SkipEmptyParts)) {
        lstData << str.split(","); //! 不忽略空白
    };


    m_pDiagnosisModel->removeRows( 0, m_pDiagnosisModel->items()->count(), QModelIndex() );
    foreach (QStringList lst, lstData) {

        int code = 0;
        QString strType;
        QString strTime;
        QString strAdditionInfo;
        int counter = 0;
        QString strMessage;

        if(lst.size() > 0){
            bool ok = false;
            code = lst.at(0).toInt(&ok);
            if(!ok) continue;
        }
        if(lst.size() > 1){
            strType      = lst.at(1);
        }
        if(lst.size() > 2){
            strTime = lst.at(2);
        }
        if(lst.size() > 3){
            strAdditionInfo = lst.at(3);
        }
        if(lst.size() > 4){
            bool ok = false;
            counter = lst.at(4).toInt(&ok);
            if(!ok) continue;
        }
        if(lst.size() > 5){
            strMessage   = lst.at(5);
        }
        m_pDiagnosisModel->appendOneItem(code,strType,strTime,strAdditionInfo,counter,strMessage);
    };


    ui->tvDiagnosis->resizeColumnsToContents();
    for(int i = 0; i < ui->tvDiagnosis->horizontalHeader()->count() - 1; i++){
        ui->tvDiagnosis->setColumnWidth(i, ui->tvDiagnosis->columnWidth(i) + 16);
    }
    ui->tvDiagnosis->horizontalHeader()->setStretchLastSection(true);

}

void H2Ops::on_btnDelete_clicked()
{
    MegaDeleteAffirmMessageBox msgBox;
    int ret = msgBox.exec();
    if ( ret == QMessageBox::Ok )
    {
        m_pDiagnosisModel->removeRows( 0, m_pDiagnosisModel->items()->count(), QModelIndex() );
    }
}

void H2Ops::on_btnExport_2_clicked()
{
    QFileDialog fDlg;

    fDlg.setAcceptMode( QFileDialog::AcceptSave );
    fDlg.setNameFilter( "Diagnosis (*.xml)" );

    if ( QDialog::Accepted != fDlg.exec() )
    { return; }

    m_pDiagnosisModel->save( fDlg.selectedFiles().first() );
}

////////////////////////////////////// 点击发送指令
void H2Ops::slot_starting_home_over(int ret)
{
    if(ret < 0){
        QMessageBox::critical(this,tr("error"),tr("failure"));
    }
    ui->h2Status->setActionStatus(H2Status::ACTION_MC);
}

void H2Ops::on_pushButton_starting_home_clicked()
{
    if(mViHandle <= 0) return;

    double velocity = m_Data["SearchVelocity"].toDouble();

    auto func = [=](int &result)
    {
        //不等待
        int ret = mrgRobotGoHomeWithParam(mViHandle, mRobotName, velocity, -1);
        qDebug() << "mrgRobotGoHome" << ret;

        while(1)
        {
            if ( m_threadOpsHoming->isInterruptionRequested() ){
                result = 1;
                return;
            }
            ret = mrgRobotWaitHomeEnd(mViHandle, mRobotName, -1);
            if(ret == -3){//超时
                qDebug() << "mrgRobotWaitHomeEnd timeout";
                continue;
            }else if(ret != 0){
                qDebug() << "mrgRobotWaitHomeEnd error";
                result = -1;
                return;
            }
            else{
                qDebug() << "mrgRobotWaitHomeEnd ok";
                result = 0;
                return;
            }
        }
    }; //end func

    if(m_isHomgingRunFlag) //正在运行
    {
        m_isHomgingRunFlag = false;

        mrgRobotGoHomeStop(mViHandle, mRobotName);

        if( m_threadOpsHoming != NULL ){
            m_threadOpsHoming->requestInterruption();
            m_threadOpsHoming->wait();
        }
        ui->pushButton_starting_home->setText(tr("Start Go Home"));
    }else{
        m_isHomgingRunFlag = true;
        ui->pushButton_starting_home->setText(tr("Stop Go Home"));

        if(mViHandle <= 0) return;
        m_threadOpsHoming = new XThread(func);
        connect(m_threadOpsHoming,SIGNAL(signalFinishResult(int)),SLOT(slot_starting_home_over(int)));
        connect(m_threadOpsHoming,&XThread::finished,[&](){
            m_threadOpsHoming = NULL;
            ui->pushButton_starting_home->setText(tr("Start Go Home"));
            m_isHomgingRunFlag = false;
        });
        m_threadOpsHoming->start(QThread::LowestPriority);
    }
}

void H2Ops::setButtonDisableTime(QToolButton *btn, int msec)
{
    btn->setEnabled(false);
    //非阻塞延时
    QTime dieTime = QTime::currentTime().addMSecs(msec);
    while( QTime::currentTime() < dieTime )
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

    btn->setEnabled(true);
}

void H2Ops::setOpsMonitorTimerStop()
{
    if( m_timerOpsMonitor->isActive() )
    {
        m_timerOpsMonitor->stop();
        if(mViHandle <= 0) return;
        //关闭上报状态
        mrgMRQReportState(this->mViHandle,this->mDeviceName, 0, 0, 0);
        mrgMRQReportState(this->mViHandle,this->mDeviceName, 1, 0, 0);
    }
}

void H2Ops::setOpsMonitorTimerStart()
{
    if(mViHandle <= 0) return;
    if( !m_timerOpsMonitor->isActive() )
    {
        if(mViHandle <= 0) return;
        //打开上报状态
        mrgMRQReportState(this->mViHandle,this->mDeviceName, 0, 0, 1);
        mrgMRQReportState(this->mViHandle,this->mDeviceName, 1, 0, 1);
        m_timerOpsMonitor->start();
    }
}


//ax:0 表示x轴， 1表示y轴, 2表示z轴，
//direct: -1表示负方向，1表示正方向
void H2Ops::buttonClickedSingelMove(QToolButton *btn, int ax, int direct)
{
    if(mViHandle <= 0) return;

    double  SWLimitPositiveX = m_Data["SWLimitPositiveX"].toDouble() + 0.5;
    double  SWLimitPositiveY = m_Data["SWLimitPositiveY"].toDouble() + 0.5;
    double  SWLimitNegativeX = m_Data["SWLimitNegativeX"].toDouble() - 0.5;
    double  SWLimitNegativeY = m_Data["SWLimitNegativeY"].toDouble() - 0.5;

    double  ProjectZeroPointX = m_Data["ProjectZeroPointX"].toDouble();
    double  ProjectZeroPointY = m_Data["ProjectZeroPointY"].toDouble();

    double curPosX = ui->doubleSpinBox_currentPos_x->value() + ProjectZeroPointX;
    double curPosY = ui->doubleSpinBox_currentPos_y->value() + ProjectZeroPointY;

    double offset = ui->doubleSpinBox_Increament->value();
    double speed = ui->doubleSpinBox_Velocity->value();
    float time = offset/(m_speedRatio*speed); //把速度当成最大速度

    if(ax == 0){
        double move = curPosX + (offset*direct);
        if( (move > SWLimitPositiveX) || (move < SWLimitNegativeX)){
            QMessageBox::warning(this, tr("warning"), tr("Target position over the limit"));
            return;
        }
    }else if(ax == 1){
        double move = curPosY + (offset*direct);
        if((move > SWLimitPositiveY) || (move < SWLimitNegativeY)) {
            QMessageBox::warning(this, tr("warning"), tr("Target position over the limit"));
            return;
        }
    }

    auto funcRun = [=]()
    {
        int ret = -1;
        if(ax == 0){
            ret = mrgRobotRelMove(mViHandle, mRobotName, -1, offset*direct, 0, 0, time, 0);
        }else if(ax == 1){
            ret = mrgRobotRelMove(mViHandle, mRobotName, -1, 0, offset*direct, 0, time, 0);
        }else if(ax == 2){
            ret = mrgRobotRelMove(mViHandle, mRobotName, -1, 0, 0, offset*direct, time, 0);
        }
        qDebug() << "mrgRobotRelMove" << ret << "ax=" << ax << direct;
        if(ret < 0)
            sysError("mrgRobotRelMove", ret);
    };

    on_pushButton_stop_clicked();
    QThread::msleep(100);
    btn->setEnabled(false);

    XThread *thread = new XThread(funcRun);
    connect(thread,&XThread::finished,this,[=](){
        btn->setEnabled(true);
        ui->h2Status->setActionStatus(H2Status::ACTION_MC);
    });
    thread->start();
}

//! single move
void H2Ops::on_toolButton_singlestep_x_dec_clicked()
{
    buttonClickedSingelMove(ui->toolButton_singlestep_x_dec, 0, -1);
}

void H2Ops::on_toolButton_singlestep_x_inc_clicked()
{
    buttonClickedSingelMove(ui->toolButton_singlestep_x_inc, 0, 1);
}

void H2Ops::on_toolButton_singlestep_y_dec_clicked()
{
    buttonClickedSingelMove(ui->toolButton_singlestep_y_dec, 1, -1);
}

void H2Ops::on_toolButton_singlestep_y_inc_clicked()
{
    buttonClickedSingelMove(ui->toolButton_singlestep_y_inc, 1, 1);
}

//! jog move
void H2Ops::on_toolButton_jogmode_x_dec_pressed()
{
    if(mViHandle <= 0) return;
    float cr_speed = m_Data["CrawlingVelocity"].toFloat();
    float cr_time = m_Data["CrawlingTime"].toFloat() / 1000; //ms --> s
    float speed = m_Data["MaximumVelocity"].toFloat();

    int ret = mrgRobotMoveJog(mViHandle, mRobotName, -1, 0, cr_time, 0-cr_speed, speed);
    if(ret != 0)
        sysError("mrhtRobotMoveJog", ret);
}

void H2Ops::on_toolButton_jogmode_x_inc_pressed()
{
    if(mViHandle <= 0) return;
    float cr_speed = m_Data["CrawlingVelocity"].toFloat();
    float cr_time = m_Data["CrawlingTime"].toFloat() / 1000; //ms --> s
    float speed = m_Data["MaximumVelocity"].toFloat();

    int ret = mrgRobotMoveJog(mViHandle, mRobotName, -1, 0, cr_time, cr_speed, speed);
    if(ret != 0)
        sysError("mrhtRobotMoveJog", ret);
}

void H2Ops::on_toolButton_jogmode_y_dec_pressed()
{
    if(mViHandle <= 0) return;
    float cr_speed = m_Data["CrawlingVelocity"].toFloat();
    float cr_time = m_Data["CrawlingTime"].toFloat() / 1000; //ms --> s
    float speed = m_Data["MaximumVelocity"].toFloat();

    int ret = mrgRobotMoveJog(mViHandle, mRobotName, -1, 1, cr_time, 0-cr_speed, speed);
    if(ret != 0)
        sysError("mrhtRobotMoveJog", ret);
}

void H2Ops::on_toolButton_jogmode_y_inc_pressed()
{
    if(mViHandle <= 0) return;
    float cr_speed = m_Data["CrawlingVelocity"].toFloat();
    float cr_time = m_Data["CrawlingTime"].toFloat() / 1000; //ms --> s
    float speed = m_Data["MaximumVelocity"].toFloat();

    int ret = mrgRobotMoveJog(mViHandle, mRobotName, -1, 1, cr_time, cr_speed, speed);
    if(ret != 0)
        sysError("mrhtRobotMoveJog", ret);
}


void H2Ops::on_toolButton_jogmode_x_dec_released()
{   on_pushButton_stop_clicked();   }

void H2Ops::on_toolButton_jogmode_x_inc_released()
{   on_pushButton_stop_clicked();   }

void H2Ops::on_toolButton_jogmode_y_dec_released()
{   on_pushButton_stop_clicked();   }

void H2Ops::on_toolButton_jogmode_y_inc_released()
{   on_pushButton_stop_clicked();   }


void H2Ops::on_pushButton_stop_clicked()
{
    if(mViHandle <= 0) return;

    int ret = mrgRobotStop(mViHandle, mRobotName, -1);
    qDebug() << "mrgRobotStop" << ret;

    ui->toolButton_singlestep_x_dec->setEnabled(true);
    ui->toolButton_singlestep_x_inc->setEnabled(true);
    ui->toolButton_singlestep_y_dec->setEnabled(true);
    ui->toolButton_singlestep_y_inc->setEnabled(true);
}

void H2Ops::on_pushButton_apply_clicked()
{
    //将速度 加速度 坐标点设置到record_table的第x行
    double x = ui->doubleSpinBox_currentPos_x->value();
    double y = ui->doubleSpinBox_currentPos_y->value();
    double v = ui->doubleSpinBox_Velocity->value();

    emit signal_apply_point(m_recordNumber,"PA",x,y,v,-1);
}

void H2Ops::on_toolButton_debugRun_clicked()
{
    auto func = [this]()
    {
        int rowCount = ui->tvDebug->model()->rowCount();
        bool isCyclic = ui->chkCyclic->isChecked();
        bool noBreak = true;
        do
        {
            for(int i=0; i<rowCount; i++)
            {
                ui->tvDebug->selectRow(i);
                int recordNumber = ui->tvDebug->model()->index(i, 0).data().toInt();
                double time = ui->tvDebug->model()->index(i, 1).data().toDouble();
                int ret = -1;

                ret = mrgRobotFileResolve(mViHandle, mRobotName, 0, recordNumber, 0, 20000);
                if(ret != 0) {
                    sysError("mrgRobotFileResolve", ret);
                    return;
                }

                ret = mrgRobotRun(mViHandle, mRobotName, 0);
                if(ret != 0) {
                    sysError("mrgRobotRun", ret);
                    return;
                }

                ret = mrgRobotWaitEnd(mViHandle, mRobotName, 0, 0);
                if(ret != 0) {
                    sysError("mrgRobotWaitEnd", ret);
                    return;
                }

                if ( m_threadOpsDebug->isInterruptionRequested() ){
                    noBreak = false;
                    break;
                }

                QThread::msleep(time * 1000);
            }
        }while(isCyclic && noBreak);

        ui->toolButton_debugRun->setText(tr("Run  Sequence"));
        m_isDebugRunFlag = false;
    }; //end func


    if(m_isDebugRunFlag) //正在运行
    {
        m_isDebugRunFlag = false;

        if( m_threadOpsDebug != NULL ){
            m_threadOpsDebug->requestInterruption();
            m_threadOpsDebug->wait();
        }
        ui->toolButton_debugRun->setText(tr("Run  Sequence"));
    }else{
        m_isDebugRunFlag = true;
        ui->toolButton_debugRun->setText(tr("Stop Sequence"));

        if(mViHandle <= 0) return;
        m_threadOpsDebug = new XThread(func);
        connect(m_threadOpsDebug,&XThread::finished,[&](){ m_threadOpsDebug = NULL; });
        m_threadOpsDebug->start(QThread::LowestPriority);
    }

}

/////////////////////////////////////////////////////////////
//更新标签的实时数值
void H2Ops::updateBackgroundStatus()
{
    if(mViHandle <= 0) return;
    int homeVaild = mrgGetRobotHomeRequire(mViHandle, mRobotName);
    qDebug() << "mrgGetRobotHomeRequire" << homeVaild;
    if(homeVaild == 1)
    {//表示需要回零
        ui->radHome->setChecked(false);
        ui->radioButton_homing_valid->setChecked(false);

        ui->tab_Manual->setEnabled(false);
        ui->tab_Debug->setEnabled(false);
        ui->tab_Monitor->setEnabled(false);
//        ui->tabWidget->setCurrentIndex(3);

        return;
    }
    else if(homeVaild == 0)
    {//表示不需要回零
        ui->radHome->setChecked(true);
        ui->radioButton_homing_valid->setChecked(true);

        ui->tab_Manual->setEnabled(true);
        ui->tab_Debug->setEnabled(true);
        ui->tab_Monitor->setEnabled(true);
    }

#if 0
    //! 如果外部停止按钮被按下，禁止所有可用操作
    int isStop = 0;

    if(isStop == 1)
    {//表示外部停止按钮被按下
        ui->radES->setChecked(false);
        ui->tab_Homing->setEnabled(false);
        ui->tab_Manual->setEnabled(false);
        ui->tab_Debug->setEnabled(false);

        stopOpsAllUpdateShow();
        return;
    }
    else if(isStop == 0)
    {//表示外部停止按钮没有被按下
        ui->radES->setChecked(true);
        ui->tab_Homing->setEnabled(true);
        ui->tab_Manual->setEnabled(true);
        ui->tab_Debug->setEnabled(true);
    }
#endif


}

void H2Ops::updateOpsDeviceStatus()
{
    if(mViHandle <= 0) return;
    //! TODO

}

void H2Ops::updateTabOpreate()
{
    if(mViHandle <= 0) return;
    int ret = -1;

    //! 更新记录编号
    int record = -1;
    ret = mrgGetRobotCurrentRecord(mViHandle, mRobotName, &record);
    qDebug() << "mrgGetRobotCurrentRecord" << ret << record;
    ui->doubleSpinBox_RecordNumber->setValue( record );

    //! 更新目标位置
    float dx = -1, dy = -1, dz = -1;
    ret =  mrgGetRobotTargetPosition(mViHandle, mRobotName, &dx, &dy, &dz);
    qDebug() << "mrgGetRobotTargetPosition" << ret << dx << dy;
    ui->doubleSpinBox_target_position_x->setValue( dx );
    ui->doubleSpinBox_target_position_y->setValue( dy );

    //! 更新当前位置
    float fx = -1, fy = -1, fz = -1;
    ret = mrgGetRobotCurrentPosition(mViHandle, mRobotName, &fx, &fy, &fz);
    qDebug() << "mrgGetRobotCurrentPosition1:" << ret << fx << fy;
    ui->doubleSpinBox_actual_position_x->setValue(fx);
    ui->doubleSpinBox_actual_position_y->setValue(fy);

    //! 更新当前里程
    ret = mrgGetRobotCurrentMileage(mViHandle, mRobotName, &dx, &dy, &dz);
    qDebug() << "mrgGetRobotCurrentMileage:" << ret << dx << dy;
    ui->doubleSpinBox_Mileage_x->setValue( dx );
    ui->doubleSpinBox_Mileage_y->setValue( dy );

}

void H2Ops::updateTabIO()
{
    if(mViHandle <= 0) return;
    int ret = -1;

    //!TODO

    //! 更新记录编号
    int record = -1;
    ret = mrgGetRobotCurrentRecord(mViHandle, mRobotName, &record);
    qDebug() << "mrgGetRobotCurrentRecord" << ret << record;
    ui->label_recordNumber->setText(QString::number( record ));




}

void H2Ops::updateTabHoming()
{
    if(mViHandle <= 0) return;

    ui->label_homing_target->setText(m_Data["Target"]);
    ui->label_homing_direction->setText(m_Data["Direction"]);

    //! 更新当前位置
    float fx = -1, fy = -1, fz = -1;
    int ret = mrgGetRobotCurrentPosition(mViHandle, mRobotName, &fx, &fy, &fz);
    qDebug() << "mrgGetRobotCurrentPosition2:" << ret << fx << fy;
    ui->doubleSpinBox_homing_actual_pos_x->setValue(fx);
    ui->doubleSpinBox_homing_actual_pos_y->setValue(fy);

}

void H2Ops::updateTabManual()
{
    if(mViHandle <= 0) return;

    //! 更新当前位置
    float fx = -1, fy = -1, fz = -1;
    int ret = mrgGetRobotCurrentPosition(mViHandle, mRobotName, &fx, &fy, &fz);
    qDebug() << "mrgGetRobotCurrentPosition3:" << ret << fx << fy;
    ui->doubleSpinBox_currentPos_x->setValue(fx);
    ui->doubleSpinBox_currentPos_y->setValue(fy);
}

void H2Ops::updateTabMonitor()
{
    if(mViHandle <= 0) return;

    unsigned int array1[1024] = {0};
    unsigned int array2[1024] = {0};
    int count1 = -1, count2 = -1;

    //查询第一个电机
    memset(array1, '\0', sizeof(array1));
    count1 = mrgMRQReportQueue_Query(mViHandle, mDeviceName, 0, 0, array1);
    if(count1 <= 0)
        return;

    //查询第二个电机
    memset(array2, '\0', sizeof(array2));
    count2 = mrgMRQReportQueue_Query(mViHandle, mDeviceName, 1, 0, array2);
    if(count2 <= 0)
        goto LAB1;

    for(int i=0; i<count2; i++)
    {
        int v1 = (array2[i] >> 8) & 0xFF;
        int v2 = array2[i] & 0xFF;

        qDebug() << "m_splineChart2" << i << v1 << v2;
        if(v1>=0 && v1<=100 && v2>=0 && v2<=100)
            m_splineChart2->dataAppend(v1,v2);
    }

LAB1:
    for(int i=0; i<count1; i++)
    {
        int v1 = (array1[i] >> 8) & 0xFF;
        int v2 = array1[i] & 0xFF;
        qDebug() << "m_splineChart1" << i << v1 << v2;
        if(v1>=0 && v1<=100 && v2>=0 && v2<=100)
            m_splineChart1->dataAppend(v1,v2);
    }
}

void H2Ops::updateTabDebug()
{
    if(mViHandle <= 0) return;
    int ret = -1;

    //! 更新记录编号
    int record = -1;
    ret = mrgGetRobotCurrentRecord(mViHandle, mRobotName, &record);
    qDebug() << "mrgGetRobotCurrentRecord" << ret << record;
    ui->doubleSpinBox_debugRecord->setValue( record );

    //! 更新当前位置
    float fx = -1, fy = -1, fz = -1;
    ret = mrgGetRobotCurrentPosition(mViHandle, mRobotName, &fx, &fy, &fz);
    qDebug() << "mrgGetRobotCurrentPosition4:" << ret << fx << fy;
    ui->doubleSpinBox_debug_posX->setValue(fx);
    ui->doubleSpinBox_debug_posY->setValue(fy);
}

void H2Ops::updateTabDiagnosis()
{
    if(mViHandle <= 0) return;

    //!TODO
    qDebug() << "Diagnosis";
}

void H2Ops::on_pushButton_go_prjZero_clicked()
{
    if(mViHandle <= 0) return;

    double velocity = m_Data["SearchVelocity"].toDouble();

    auto func = [=](int &result)
    {
        float fx = -1, fy = -1, fz = -1;
        result = mrgGetRobotCurrentPosition(mViHandle, mRobotName, &fx, &fy, &fz);
        if(result < 0) {
            sysError("mrgGetRobotCurrentPosition", result);
            result = -1;
            return;
        }

        double time = sqrt( pow(0-fx,2) + pow(0-fy,2) ) / (m_speedRatio * velocity);
        result = mrgRobotRelMove(mViHandle, mRobotName, -1, 0-fx, 0-fy, 0, time, 0);
        if(result < 0) {
            sysError("mrgRobotRelMove", result);
            result = -2;
            return;
        }

        qDebug() << "Go project zero point ok";
        result = 0;
        return;

    }; //end func

    if(m_isPrjZeroRunFlag) //正在运行
    {
        m_isPrjZeroRunFlag = false;
        int ret = mrgRobotStop(mViHandle, mRobotName, -1);
        qDebug() << "mrgRobotStop" << ret;
        ui->pushButton_go_prjZero->setText(tr("Run to Project Zero Point"));
    }else{
        m_isPrjZeroRunFlag = true;
        ui->pushButton_go_prjZero->setText(tr("Stop"));

        m_threadOpsPrjZero = new XThread(func);
        connect(m_threadOpsPrjZero,&XThread::finished,
                [&](){
            m_threadOpsPrjZero = NULL;
            ui->pushButton_go_prjZero->setText(tr("Run to Project Zero Point"));
            m_isPrjZeroRunFlag = false;
        });
        connect(m_threadOpsPrjZero,SIGNAL(signalFinishResult(int)),this,SLOT(slot_starting_home_over(int)));
        m_threadOpsPrjZero->start(QThread::LowestPriority);
    }

}

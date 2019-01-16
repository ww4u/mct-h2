#include "roboconfig.h"
#include "ui_roboconfig.h"

#include "mystd.h"
#include "h2robo.h"
#include "xthread.h"

RoboConfig::RoboConfig(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RoboConfig)
{
    ui->setupUi(this);

    mIndex = -1;
    m_menu = NULL;
    m_megaSerachWidget = NULL;

    connect( ui->treeWidget, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
             this, SLOT(slot_current_changed(QTreeWidgetItem*,QTreeWidgetItem*)));

    ui->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeWidget, SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(slotShowContextmenu(const QPoint&)));

    buildUI();

#ifndef _WIN32
    //! 如果在MRH-T上运行自动添加本地
    addDeviceWithIP("127.0.0.1");
#endif

}

void RoboConfig::buildUI()
{
    m_pRootNode = new QTreeWidgetItem();
    m_pRootNode->setText( 0, tr("Project"));
    m_pRootNode->setIcon( 0, QIcon( ":/res/image/icon/201.png" ) );
    ui->treeWidget->addTopLevelItem( m_pRootNode );

    QWidget *pWidget = new QWidget;
    pWidget->setMinimumSize(600,200);
    QVBoxLayout *t_layout = new QVBoxLayout(pWidget);
    QLabel *t_label = new QLabel;
    t_label->setPixmap(QPixmap(":/res/image/m.png"));
    t_label->setAlignment(Qt::AlignHCenter);
    t_label->setMinimumSize(200,200);
    QLabel *t_label2 = new QLabel("www.megarobo.tech");
    t_label2->setAlignment(Qt::AlignHCenter);

    t_layout->addWidget(t_label);
    t_layout->addWidget(t_label2);
    m_pRootNode->setData( 0, Qt::UserRole, QVariant( QVariant::fromValue(pWidget) ) );
    pWidget->setEnabled(false);
    ui->stackedWidget->addWidget( pWidget );

    loadXmlConfig();
}

//void RoboConfig::updateUI()
//{
//    qDebug() << "updateUI";
//    for(int index=0; index<m_RobotList.size(); index++){
//        foreach (XConfig *pCfg, ((H2Robo *)m_RobotList[index].m_Robo)->subConfigs()){
//            pCfg->loadConfig();
//            pCfg->updateShow();
//        }
//    }
//}

RoboConfig::~RoboConfig()
{
    delete ui;
}

void RoboConfig::loadXmlConfig()
{
    //! load xml
    MegaXML mXML;
    QString fileName = QApplication::applicationDirPath();
    QDir dir(fileName);
    if(!dir.exists()){dir.mkdir(fileName);}

    fileName += "/config.xml";
    mXML.xmlCreate(fileName);

    QMap<QString,QString> mapItems = mXML.xmlRead(fileName);
    QMap<QString,QString>::iterator itMap;
    for ( itMap=mapItems.begin(); itMap != mapItems.end(); ++itMap ) {
        if( "RobotDevice_" == itMap.key().left(QString("RobotDevice_").length()) )
            createRobot(itMap.value());
    }
    ui->treeWidget->setCurrentItem(m_pRootNode);
}

void RoboConfig::createRobot(QString strDevInfo)
{
    RobotInfo robotInfo;
    robotInfo.m_Robo = new H2Robo( ui->stackedWidget, strDevInfo );
    robotInfo.m_strDevInfo = strDevInfo;
    robotInfo.m_Visa = 0;
    robotInfo.m_DeviceName = 0;
    robotInfo.m_RoboName = 0;
    m_RobotList.insert(m_RobotList.count(), robotInfo);

    m_pRootNode->addChild( ((H2Robo *)(robotInfo.m_Robo))->roboNode() );

    mIndex = m_RobotList.count()-1;
    ui->treeWidget->setCurrentItem(m_pRootNode->child( mIndex ) );
    m_pRootNode->setExpanded(true);
    m_pRootNode->child(mIndex)->setExpanded(true);

    foreach (XConfig *pCfg, ((H2Robo *)(robotInfo.m_Robo))->subConfigs()){
        QString configFileName = robotInfo.m_strDevInfo.split(',').at(3);
        pCfg->setConfigFileName(configFileName);
        pCfg->loadConfig();
        pCfg->updateShow();
        connect( pCfg, SIGNAL(signal_focus_in( const QString &)),
                 this, SIGNAL(signal_focus_in( const QString &)) );
    }

    foreach (XConfig *pCfg, ((H2Robo *)(robotInfo.m_Robo))->subConfigs()){
        pCfg->saveConfig();
    }

    connect((H2Robo *)(robotInfo.m_Robo),SIGNAL(signal_online_request(QString)),
            this,SLOT(slot_open_close(QString)));

    connect((H2Robo *)(robotInfo.m_Robo),SIGNAL(signal_action_selected(int)),
            this,SIGNAL(signal_record_selected(int)));

//    updateUI();
}

void RoboConfig::slotAddNewRobot(QString strDevInfo)
{
    m_megaSerachWidget = NULL;
    for(int i=0; i< m_RobotList.count(); i++)
    {
        if( m_RobotList[i].m_strDevInfo == strDevInfo)
        {
            if( !strDevInfo.contains("127.0.0.1") )
                QMessageBox::information(this,tr("tips"),tr("The device already exists in the project."));
            return;
        }else{
            //将之前的127.0.0.1从工程中删除
            if( m_RobotList[i].m_strDevInfo.contains("127.0.0.1") )
            {
                //! delete from xml file
                MegaXML mXML;
                QString fileName = QApplication::applicationDirPath() + "/config.xml";
                QMap<QString,QString> mapRead = mXML.xmlRead(fileName);
                QMap<QString,QString> mapWrite;
                for (QMap<QString,QString>::iterator itMap=mapRead.begin(); itMap != mapRead.end(); ++itMap )
                {
                    if( "RobotDevice_" == itMap.key().left(QString("RobotDevice_").length())
                            && m_RobotList[i].m_strDevInfo != itMap.value() ){
                        mapWrite.insert(itMap.key(),itMap.value());
                    }
                }
                mXML.xmlNodeRemove(fileName, "RobotConfigs");
                mXML.xmlNodeAppend(fileName, "RobotConfigs", mapWrite); //update config.xml
            }
        }
    }
    qDebug() << "slotAddNewRobot" << strDevInfo;

    createRobot(strDevInfo);

    //添加到config.xml中
    MegaXML mXML;
    QString fileName = QApplication::applicationDirPath() + "/config.xml";

    QMap<QString,QString> mapRead = mXML.xmlRead(fileName);
    QMap<QString,QString> mapWrite;
    for (QMap<QString,QString>::iterator itMap=mapRead.begin(); itMap != mapRead.end(); ++itMap ) {
        if( "RobotDevice_" == itMap.key().left(QString("RobotDevice_").length()) )
        {   mapWrite.insert(itMap.key(),itMap.value()); }
    }
    QString proName = QString("RobotDevice_%1").arg(mapWrite.size());
    mapWrite.insert(proName ,strDevInfo );
    mXML.xmlNodeRemove(fileName, "RobotConfigs");
    mXML.xmlNodeAppend(fileName, "RobotConfigs", mapWrite);

    QString strIP = strDevInfo.split(',').at(0);
    slot_open_close(strIP); //默认打开设备

    if(m_RobotList[mIndex].m_Visa != 0)
    {
        //第一次打开设备将数据同步到上位机
        foreach (XConfig *pCfg, ((H2Robo *)m_RobotList[mIndex].m_Robo)->subConfigs()){
            int ret = pCfg->readDeviceConfig();
            if(ret != 0){
                QMessageBox::critical(this,tr("error"), pCfg->focuHelpName() + "\n" +tr("From device upload config faiured"));
            }
            pCfg->updateShow();
            pCfg->saveConfig();
            emit signalDataChanged();
        }
    }
}

void RoboConfig::slotDownload()
{
    if(mIndex < 0) return;
    if( m_RobotList[mIndex].m_Visa == 0)
    {
        QMessageBox::warning(this,tr("warning"),tr("Current Device In Offline"));
        return;//offline
    }

    bool ok = true;
    foreach (XConfig *pCfg, ((H2Robo *)m_RobotList[mIndex].m_Robo)->subConfigs()){
        pCfg->saveConfig();
        int ret = pCfg->writeDeviceConfig();
        if(ret != 0){
            ok = false;
            QMessageBox::critical(this,tr("error"), pCfg->focuHelpName() + "\n" + tr("Download Failure"));
            continue;
        }

        {   //从设备再重新upload一遍
            int ret = pCfg->readDeviceConfig();
            if(ret != 0){
                ok = false;
                QMessageBox::critical(this,tr("error"), pCfg->focuHelpName() + "\n" + tr("Download Failure"));
            }
            pCfg->updateShow();
            pCfg->saveConfig();
        }
    }
    emit signalDataChanged();
    if(ok){
        QMessageBox::information(this,tr("tips"),tr("Download Success!"));
    }
    qDebug() << "slotDownload Finish";

    return;
}

void RoboConfig::slotUpload()
{
    if(mIndex < 0) return;

    if( m_RobotList[mIndex].m_Visa == 0)
    {
        QMessageBox::warning(this,tr("warning"),tr("Current Device In Offline"));
        return;
    }

    bool ok = true;
    foreach (XConfig *pCfg, ((H2Robo *)m_RobotList[mIndex].m_Robo)->subConfigs()){
        int ret = pCfg->readDeviceConfig();
        if(ret != 0){
            ok = false;
            QMessageBox::critical(this,tr("error"), pCfg->focuHelpName() + "\n" + tr("Upload Faiured"));
        }
        pCfg->updateShow();
        pCfg->saveConfig();
    }

    emit signalDataChanged();
    if(ok){
        QMessageBox::information(this,tr("tips"),tr("Upload Succeed!"));
    }
    qDebug() << "slotUpload Finish";
    return;
}

void RoboConfig::slotStore()
{
    if(mIndex < 0) return;
    int visa = m_RobotList[mIndex].m_Visa;
    if( visa == 0) {
        QMessageBox::warning(this,tr("warning"),tr("Current Device In Offline"));
        return;
    }

    auto func = [=](int &ret)
    {
        qDebug() << "mrgGetRobotConfigState1";
        if (mrgGetRobotConfigState(visa) == 1){
            qDebug() << "mrgGetRobotConfigState == 1";
            ret = -1;
            return;
        }
        qDebug() << "mrgGetRobotConfigState2";

        int timeout = 10000;
        int state = 0;
        qDebug() << "mrgExportRobotConfig";
        mrgExportRobotConfig(visa);
        while (timeout > 0)
        {
            QThread::msleep(500);
            qDebug() << "mrgGetRobotConfigState";
            state = mrgGetRobotConfigState(visa);
            if (state != 1) { break;}
            timeout -= 500;
        }

        if(timeout <= 0){
            ret = -2;
            return;
        }

        if (state == 0){
            ret = 0;
        }else{
            ret = -3;
        }
        return;
    };

    XThread *thread = new XThread(func);
    connect(thread,SIGNAL(signalFinishResult(int)),this,SLOT(slotStoreEnd(int)));
    thread->start();
}

void RoboConfig::slotStoreEnd(int val)
{
    switch (val) {
    case 0:
        QMessageBox::information(this,tr("tips"),tr("Store success!"));
        break;
    case -1:
        QMessageBox::warning(this,tr("warning"),tr("Operation in progress!"));
        break;
    case -2:
        QMessageBox::warning(this,tr("warning"),tr("Store timeout!"));
        break;
    case -3:
        QMessageBox::critical(this,tr("error"),tr("Store error!"));
        break;
    default:
        break;
    }
}

void RoboConfig::slotSync()
{
    //! TODO
    QMessageBox::warning(this,tr("warning"),tr("unable"));
}

void RoboConfig::slotSearch()
{
    if(m_megaSerachWidget != NULL)
    {
        delete m_megaSerachWidget;
        m_megaSerachWidget = NULL;
    }
    m_megaSerachWidget = new MegaInterface;
    m_megaSerachWidget->move( x()+100, y()+100);
    m_megaSerachWidget->show();
    connect(m_megaSerachWidget, SIGNAL(signalSelectedInfo(QString)), this, SLOT(slotAddNewRobot(QString)));
}

void RoboConfig::slotWifi()
{
    if(mIndex < 0) return;
    if( m_RobotList[mIndex].m_Visa == 0)
    {
        QMessageBox::warning(this,tr("warning"),tr("Current Device In Offline"));
        return;
    }

    int ret = -1;
    char wifiList[1024] = "";
    ret = mrgSysWifiSearch(m_RobotList[mIndex].m_Visa, wifiList);
    if(ret < 0)
    {
        QMessageBox::critical(this,tr("error"),tr("Wifi search error or empty!"));
        qDebug() << "mrgSysWifiSearch" << ret;
        return;
    }

    QStringList StrList = QString(wifiList).split(",", QString::SkipEmptyParts);

    //显示选择对话框
    QString wifiName = QInputDialog::getItem(this, tr("Wifi"), tr("Please choose wifi:"), StrList, -1, false);
    if(wifiName == "")
        return;

    //显示输入对话框获取输入的wifi密码
    QString password = QInputDialog::getText(this, tr("Input"), tr("Please input wifi password:"), QLineEdit::Password);
    if(password == "")
        return;

    ret = mrgSysWifiConnect(m_RobotList[mIndex].m_Visa, wifiName.toLocal8Bit().data(), password.toLocal8Bit().data());
    if(ret == 1){
        QMessageBox::information(this,tr("tips"),tr("Wifi Connect success!"));
    }
    else{
        QMessageBox::critical(this,tr("error"),tr("Wifi Connect error!"));
        qDebug() << "mrgSysWifiConnect" << ret;
    }
    return;
}

void RoboConfig::slotExit()
{
    foreach (RobotInfo robo, m_RobotList ){
        QString strIP = robo.m_strDevInfo.split(',').at(0);
        if(robo.m_Visa != 0){
            //如果没有关闭就关闭设备
            slot_open_close(strIP);
        }
    }

    if(m_megaSerachWidget != NULL){
        m_megaSerachWidget->close();
    }
}

void RoboConfig::slotConnect()
{
    if(mIndex<0) return;
    QString strIP = m_RobotList[mIndex].m_strDevInfo.split(',').at(0);
    qDebug() << "slotConnect" << strIP;
    slot_open_close(strIP);
}

void RoboConfig::slotShowContextmenu(const QPoint& pos)
{
    QTreeWidgetItem* curItem = ui->treeWidget->itemAt(pos);  //获取当前被点击的节点
    if(curItem == NULL) return; //即在空白位置右击

    QVariant var = curItem->data(0,Qt::UserRole);
    if( "H2Product*" == QString("%1").arg(var.typeName()) ){
        if(m_menu != NULL)
            delete m_menu;

        m_menu = new QMenu(ui->treeWidget);
        QAction *actionClose = m_menu->addAction(tr("close"));
        QAction *actionOpen = m_menu->addAction(tr("delete"));

        connect(actionClose, SIGNAL(triggered(bool)), this, SLOT(soltActionClose()));
        connect(actionOpen, SIGNAL(triggered(bool)), this, SLOT(soltActionDelete()));

        m_menu->exec(QCursor::pos());
        ui->treeWidget->selectionModel()->clear();
    }
}

void RoboConfig::soltActionClose()
{
    int index = mIndex;
    QString strIP = m_RobotList[index].m_strDevInfo.split(',').at(0);
    if(m_RobotList[index].m_Visa != 0){
        //如果没有关闭就关闭设备
        slot_open_close(strIP);
    }

    m_pRootNode->removeChild(((H2Robo *)m_RobotList[index].m_Robo)->roboNode());
    delete (H2Robo *)m_RobotList[index].m_Robo;
    m_RobotList.removeAt(index);

    ui->treeWidget->setCurrentItem(m_pRootNode);

//    qDebug() << "after close:" << m_RobotList.count() << mIndex;
    if(m_RobotList.count() == 0)
        mIndex = -1;
}

void RoboConfig::soltActionDelete()
{
    //! delete from xml file
    MegaXML mXML;
    QString fileName = QApplication::applicationDirPath() + "/config.xml";
    QMap<QString,QString> mapRead = mXML.xmlRead(fileName);
    QMap<QString,QString> mapWrite;
    for (QMap<QString,QString>::iterator itMap=mapRead.begin(); itMap != mapRead.end(); ++itMap )
    {
        if( "RobotDevice_" == itMap.key().left(QString("RobotDevice_").length())
                && m_RobotList[mIndex].m_strDevInfo != itMap.value() ){
            mapWrite.insert(itMap.key(),itMap.value());
        }
    }
    mXML.xmlNodeRemove(fileName, "RobotConfigs");
    mXML.xmlNodeAppend(fileName, "RobotConfigs", mapWrite); //update config.xml

    QString strIDn = m_RobotList[mIndex].m_strDevInfo.split(',').at(3);

    //delete device.xml
    fileName = QApplication::applicationDirPath() + "/robots/" + strIDn + ".xml";
    {
        QFile file(fileName);
        if(file.exists())
        {   file.remove();  }
    }
    //!

    fileName = QApplication::applicationDirPath() + "/dataset/" + strIDn + ".mrp";
    {
        QFile file(fileName);
        if(file.exists())
        {   file.remove();  }
    }

    fileName = QApplication::applicationDirPath() + "/dataset/" + strIDn + ".xml";
    {
        QFile file(fileName);
        if(file.exists())
        {   file.remove();  }
    }

    soltActionClose();
}

int RoboConfig::setReset()
{
    if(mIndex < 0) return -100;

    //将默认的配置文件替换掉对应的配置文件，更新界面，写入设备
    QString strIDN = m_RobotList[mIndex].m_strDevInfo.split(',').at(3);
    copyFileToPath(QApplication::applicationDirPath() + "/robots/default.xml",
                   QApplication::applicationDirPath() + "/robots/" + strIDN + ".xml",
                   true);

    copyFileToPath(QApplication::applicationDirPath() + "/dataset/action_default.mrp",
                   QApplication::applicationDirPath() + "/dataset/" + strIDN + ".mrp",
                   true);

    copyFileToPath(QApplication::applicationDirPath() + "/dataset/errmgr_default.xml",
                   QApplication::applicationDirPath() + "/dataset/" + strIDN + ".xml",
                   true);

    foreach (XConfig *pCfg, ((H2Robo *)m_RobotList[mIndex].m_Robo)->subConfigs())
    {
        pCfg->loadConfig();
        pCfg->updateShow();
    }

    bool ok = true;
    if( m_RobotList[mIndex].m_Visa == 0)
    {
        QMessageBox::warning(this,tr("warning"),tr("Current Device In Offline"));
        return -2;//offline
     }

    foreach (XConfig *pCfg, ((H2Robo *)m_RobotList[mIndex].m_Robo)->subConfigs())
    {
        int ret = pCfg->writeDeviceConfig();
        if(ret != 0){
            ok = false;
            QMessageBox::critical(this,tr("error"), pCfg->focuHelpName() + "\t" + tr("Reset Failure"));
        }
    }
    emit signalDataChanged();

    if(ok){
        QMessageBox::information(this,tr("tips"),tr("Reset Success!"));
        return 0;
    }else{
        return -1;
    }
}

void RoboConfig::on_buttonBox_clicked(QAbstractButton *button)
{
    QDialogButtonBox::ButtonRole role = ui->buttonBox->buttonRole( button );
    if ( QDialogButtonBox::ResetRole == role ){
        setReset();
    }
    else
    {    }
}

void RoboConfig::slot_current_changed( QTreeWidgetItem* cur,QTreeWidgetItem* prv )
{
    if ( cur == NULL )
    { return; }

    QVariant var = QVariant(cur->data( 0, Qt::UserRole) );
    if ( var.isValid() ){
        QObject *pObj = var.value<QObject*>();
        if ( NULL != pObj ){
            ui->stackedWidget->setCurrentWidget( (QWidget*)pObj );
        }
    }

    int index = -1, row = -1;
    if(NULL == cur->parent() ){// 根节点-1,0
        return;
    }
    else if( NULL == cur->parent()->parent() ) { //子节点 x,0
        index = cur->parent()->indexOfChild(cur);
        row = 0;
    } else { //孙节点 x,y
        index = cur->parent()->parent()->indexOfChild(cur->parent());
        row = cur->parent()->indexOfChild(cur) + 1;
    }

    mIndex = index;
    QString strHelpName = ((H2Robo *)(m_RobotList[mIndex].m_Robo))->subConfigs().at(row)->focuHelpName();

//    qDebug() << "slot_current_changed" << mIndex << row << cur->text(0) << strHelpName;

    emit signal_focus_in( strHelpName );

    emit signalCurrentRobotChanged(m_RobotList[mIndex].m_strDevInfo,
                                   m_RobotList[mIndex].m_Visa,
                                   m_RobotList[mIndex].m_DeviceName,
                                   m_RobotList[mIndex].m_RoboName);
}

void RoboConfig::slot_open_close(QString strIP)
{
    if(mIndex < 0) return;
    H2Robo *pRobo = (H2Robo *)(m_RobotList[mIndex].m_Robo);

    if(m_RobotList[mIndex].m_Visa == 0){
        int ret = deviceOpen(strIP);
        if(ret > 0){            
            pRobo->change_online_status(true);
            emit signalDeviceConnect(true);
        }
        else{
            m_RobotList[mIndex].m_Visa = 0;
            m_RobotList[mIndex].m_DeviceName = 0;
            m_RobotList[mIndex].m_RoboName = 0;
            QMessageBox::information(this,tr("tips"),tr("Device Open Failure!!!"));
        }
    }else{
        deviceClose();
        pRobo->change_online_status(false);
        emit signalDeviceConnect(true);
    }

    //通知OPS
    emit signalCurrentRobotChanged(m_RobotList[mIndex].m_strDevInfo,
                                   m_RobotList[mIndex].m_Visa,
                                   m_RobotList[mIndex].m_DeviceName,
                                   m_RobotList[mIndex].m_RoboName);
}

int RoboConfig::deviceOpen(QString strID)
{
    if(mIndex < 0) return -1;
    int ret = -1;

    QString strDesc;
    if(strID.left(3) != "USB")
    {
        strDesc = QString("TCPIP0::%1::inst0::INSTR").arg(strID);
    }
    else
    {
        //USB0::0xA1B2::0x5722::MRHT00000000000001::INSTR
        QStringList lst = strID.split('_', QString::SkipEmptyParts);
        strDesc = QString("%1::%2::%3::%4::INSTR")
                .arg(lst.at(0))
                .arg(lst.at(1))
                .arg(lst.at(2))
                .arg(lst.at(3));
    }

    int visa = mrgOpenGateWay(strDesc.toLocal8Bit().data(), 2000);
    if(visa <= 0){
        qDebug() << "mrgOpenGateWay error" << visa;
        sysError("mrgOpenGateWay error");
        return -1;
    }

//! 直接搜索机器人和驱控器名字
    int deviceNames[32] = {0};
    int roboNames[32] = {0};
    int deviceName = -1;
    int roboName = -1;

    {
        ret = mrgGetRobotName(visa, roboNames);
        if( (ret <= 0) || (roboNames[0] == 0) )
        {
            qDebug() << "mrgGetRobotName error" << ret;
            sysError("mrgGetRobotName error");
            goto BUILD;
        }
        roboName = roboNames[0];//默认选择第一个机器人

        ret = mrgGetRobotDevice(visa, roboName, deviceNames);
        if( (ret <= 0) || (deviceNames[0] == 0) )
        {
            qDebug() << "mrgGetRobotDevice error" << ret;
            sysError("mrgGetRobotDevice error");
            goto BUILD;
        }
        deviceName = deviceNames[0];//默认选择第一个驱控器
        goto END;
    }

BUILD:
    {
        //! 构建机器人
        ret = mrgFindDevice(visa, 2000);
        if(ret <= 0){
            qDebug() << "mrgFindDevice error" << ret;
            sysError("mrgFindDevice error", ret);
            return -4;
        }

        //读取设备名称
        ret = mrgGetDeviceName(visa, deviceNames);
        if ( (ret == 0) || (deviceNames[0] == 0) ){
            qDebug() << "mrgGetDeviceName error" << ret;
            sysError("mrgGetDeviceName error", ret);
            return -5;
        }
        deviceName = deviceNames[0];//默认选择第一个驱控器

        char deviceType[128] = "";
        if ( mrgGetDeviceType(visa, deviceName, deviceType) != 0 ){
            qDebug() << "mrgGetDeviceType error";
            sysError("mrgGetDeviceType error");
            return -6;
        }

        ret = mrgBuildRobot(visa, "MRX-H2", QString("0@%1,1@%1").arg(deviceName).toLocal8Bit().data(), roboNames);
        if( (ret < 0) || (roboNames[0] == 0)){
            qDebug() << "mrgBuildRobot error" << ret;
            sysError("mrgBuildRobot error", ret);
            return -7;
        }
        ret = mrgGetRobotName(visa, roboNames);
        if( (ret <= 0) || (roboNames[0] == 0) ){
            qDebug() << "mrgGetRobotName error" << ret;
            sysError("mrgGetRobotName error");
            return -8;
        }
        roboName = roboNames[0];//默认选择第一个机器人
    }

END:
    //判断机器人类型是否是H2
    if(2 != mrgGetRobotType(visa, roboName)){
        QMessageBox::critical(this,tr("error"),tr("Robot type not") + "MRX-H2");
        return -10;
    }

    foreach (XConfig *pCfg, ((H2Robo *)m_RobotList[mIndex].m_Robo)->subConfigs())
    {    pCfg->attachHandle( visa, deviceName, roboName);  }

    mrgIdentify(visa, 0); //关闭识别防止用户之前打开

    qDebug() << "device open" << strID << visa;
    sysInfo("Device Open", visa);

    m_RobotList[mIndex].m_Visa = visa;
    m_RobotList[mIndex].m_DeviceName = deviceName;
    m_RobotList[mIndex].m_RoboName = roboName;
    return visa;
}

int RoboConfig::deviceClose()
{
    if(mIndex < 0) return -1;

//    mrgIdentify(m_RobotList[mIndex].m_Visa, 0);

    //电机关闭
    int ret = mrgMRQDriverState(m_RobotList[mIndex].m_Visa, m_RobotList[mIndex].m_DeviceName, 0, 0);
    qDebug() << "mrgMRQDriverState0 OFF" << ret;

    ret = mrgMRQDriverState(m_RobotList[mIndex].m_Visa, m_RobotList[mIndex].m_DeviceName, 1, 0);
    qDebug() << "mrgMRQDriverState1 OFF" << ret;
    QThread::msleep(300);

    ret = mrgCloseGateWay(m_RobotList[mIndex].m_Visa);

    qDebug() << "device close" << m_RobotList[mIndex].m_Visa << ret;
    sysInfo("Device Close");

    foreach (XConfig *pCfg, ((H2Robo *)m_RobotList[mIndex].m_Robo)->subConfigs())
    {    pCfg->detachHandle();  }

    m_RobotList[mIndex].m_Visa = 0;
    m_RobotList[mIndex].m_DeviceName = 0;
    m_RobotList[mIndex].m_RoboName = 0;

    return ret;
}

void RoboConfig::changeLanguage(QString qmFile)
{
    for(int index=0; index<m_RobotList.size(); index++)
    {
        m_RobotList[index].m_Robo->changeLanguage(qmFile);
        foreach (XConfig *pCfg, ((H2Robo *)m_RobotList[index].m_Robo)->subConfigs()){
            pCfg->changeLanguage(qmFile);
        }
    }

    qApp->removeTranslator(&m_translator);
    m_translator.load(qmFile);
    qApp->installTranslator(&m_translator);

    translateUi();
}

void RoboConfig::translateUi()
{
//    ui->buttonBox->button(QDialogButtonBox::Reset)->setText(tr("Reset"));
    ui->retranslateUi(this);
    m_pRootNode->setText( 0, tr("Project"));
}

void RoboConfig::slotSetOneRecord(int row, QString type, double x, double y, double v, double a)
{
    if(mIndex < 0) return;
    if(row < 0) return;

    H2Robo *pRobo = (H2Robo *)(m_RobotList[mIndex].m_Robo);
    H2Action *pAction = (H2Action *)(pRobo->subConfigs().at(5));

    if(m_RobotList[mIndex].m_Visa > 0){
        pAction->modfiyOneRecord(row-1, type, x, y, v, a);
    }
}

void RoboConfig::addDeviceWithIP(QString strID)
{
    QString strDesc;
    if(strID.left(3) != "USB")
    {
        strDesc = QString("TCPIP0::%1::inst0::INSTR").arg(strID);
    }
    else
    {
        //USB0::0xA1B2::0x5722::MRHT00000000000001::INSTR
        QStringList lst = strID.split('_', QString::SkipEmptyParts);
        strDesc = QString("%1::%2::%3::%4::INSTR")
                .arg(lst.at(0))
                .arg(lst.at(1))
                .arg(lst.at(2))
                .arg(lst.at(3));
    }

    int visa =  mrgOpenGateWay(strDesc.toLocal8Bit().data(), 2000);
    if(visa <= 0) {
        return;
    }

    char IDN[1024] = "";
    int ret = mrgGateWayIDNQuery(visa,IDN);
    if(ret != 0)
    {
        mrgCloseGateWay(visa);
        return;
    }else{
        int len = strlen(IDN);
        IDN[len-1] = '\0';
    }
    mrgCloseGateWay(visa);

    QStringList lst = strDesc.split("::", QString::SkipEmptyParts);
    QString devInfo = lst.at(1) + QString(",%1").arg(IDN);
    slotAddNewRobot(devInfo);
}

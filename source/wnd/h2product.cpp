#include "h2product.h"
#include "ui_h2product.h"

H2Product::H2Product(QString strDevInfo, QWidget *parent) :
    XConfig(parent),
    ui(new Ui::H2Product)
{
    ui->setupUi(this);

    setFocuHelpName( "Product" );
    m_strDevInfo = strDevInfo;
    m_handCloseFlag = false;

    m_thread = new LanCheckedThread(this);

    QStringList strListDev = strDevInfo.split(',', QString::SkipEmptyParts);

    QString strIP;
    QString strID = strListDev.at(0);
    if(strID.left(3) == "USB")
    {
        QStringList lst = strID.split('_', QString::SkipEmptyParts);
        strIP = lst.at(1) + "_" + lst.at(2);
    }
    else{
        strIP = strID;
        connect(m_thread,SIGNAL(signalCheckConnect(bool)),this,SLOT(slotCheckLanConnect(bool)));
        m_thread->setIp(strIP);
        m_thread->start();
    }

    this->m_IP      = strIP;
    this->m_Type    = strListDev.at(2);
    this->m_SN      = strListDev.at(3);
    this->m_Version = strListDev.at(4);

    ui->label_ip->setText(this->m_IP);
    ui->label_sn->setText(this->m_SN);
    ui->label_type->setText(this->m_Type);
    ui->label_version->setText(this->m_Version);

    change_online_status(false);
}

void H2Product::slotCheckLanConnect(bool isconnect)
{
    if(mViHandle > 0) {
        //打开状态
        if(!isconnect){
            //表示网络断开,就自动关闭设备
            qDebug() << "Network Disconnect and device auto close";
            emit signal_online_clicked(m_IP);
        }
    }else{
        //关闭状态
        if(!m_handCloseFlag){
            if(isconnect){
                //表示网络接通,并且没有被手动关闭,就自动打开设备
                qDebug() << "Network connect and device auto open";
                emit signal_online_clicked(m_IP);
            }
        }
    }
}

bool H2Product::handCloseFlag() const
{
    return m_handCloseFlag;
}

void H2Product::setHandCloseFlag(bool handCloseFlag)
{
    m_handCloseFlag = handCloseFlag;
}

H2Product::~H2Product()
{
    m_thread->quit();
    m_thread->wait();
    delete m_thread;
    delete ui;
}

void H2Product::on_pushButton_status_clicked()
{
    if(mViHandle > 0){
        //关闭设备的动作
        m_handCloseFlag = true; //手动关闭标志
    }else{
        //打开设备的动作
        m_handCloseFlag = false;
    }
    emit signal_online_clicked(m_IP);
}

void H2Product::change_online_status(bool bl)
{
    if(bl){
        ui->pushButton_status->setIcon(QIcon(":/res/image/h2product/connect.png"));
    }else{
        ui->pushButton_status->setIcon(QIcon(":/res/image/h2product/disconnect.png"));
    }
}

int H2Product::readDeviceConfig()
{
    return 0;
}

int H2Product::writeDeviceConfig()
{

    return 0;
}

int H2Product::loadConfig()
{

    return 0;
}

int H2Product::saveConfig()
{

    return 0;
}

void H2Product::updateShow()
{

}

void H2Product::translateUI()
{
    ui->retranslateUi(this);
}



LanCheckedThread::LanCheckedThread(QObject *parent)
    : QThread(parent)
{
    m_timerInterval = 3000;
    m_timerTimeoutCounter = 0;
    m_ip = "";
}

LanCheckedThread::~LanCheckedThread()
{
    quit();
    wait();
}

void LanCheckedThread::run()
{
    QTimer *m_timer = new QTimer();//不能指定父对象
    m_timer->setInterval(m_timerInterval);
    connect(m_timer,SIGNAL(timeout()),this,SLOT(slotTimerTimeout()),Qt::DirectConnection);
    m_timer->start();
    exec();
}

void LanCheckedThread::slotTimerTimeout()
{
    bool isConnect = QtPing(m_ip);
    emit signalCheckConnect(isConnect);
}

void LanCheckedThread::setIp(const QString &ip)
{
    m_ip = ip;
}

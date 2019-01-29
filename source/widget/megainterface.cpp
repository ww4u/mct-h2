#include "megainterface.h"
#include "ui_megainterface.h"

#include <QByteArray>
#include <QDebug>
#include <QMessageBox>

#include "MegaGateway.h"
#include "sysapi.h"
#include "xthread.h"

MegaInterface::MegaInterface(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MegaInterface),
    m_devType(TYPE_LAN),
    m_menu(NULL)
{
    ui->setupUi(this);

    connect(ui->comboBox_DevType,SIGNAL(currentIndexChanged(int)),this,SLOT(slotChangeDeviceType(int)));
    connect(ui->pushButton_Scan,SIGNAL(clicked(bool)),this,SLOT(slotDeviceScan()));

    m_model = new QStandardItemModel(ui->tableView);
    ui->tableView->setModel(m_model);

    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tableView, SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(slotShowContextmenu(const QPoint&)));

    connect(ui->tableView, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(slotSelectDevices()));

    connect(ui->tableView, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(close()));

}

MegaInterface::~MegaInterface()
{
    delete ui;
}

void MegaInterface::slotChangeDeviceType(int index)
{
    if(index == TYPE_LAN)
    {
        //! LAN
        ui->label_icon->setPixmap(QPixmap(":/res/image/icon/network.ico"));
        m_devType = TYPE_LAN;
    }else if(index == TYPE_USB)
    {
        //! USB
        ui->label_icon->setPixmap(QPixmap(":/res/image/icon/usb.ico"));
        m_devType = TYPE_USB;
    }
}

void MegaInterface::slotDeviceScan()
{
    auto func = [this](QString &strRet)
    {
        QString strFindDevices = "";
        QStringList strAllDeviceList;
        char buff[4096] = "";
        if(m_devType == TYPE_LAN){
            mrgFindGateWay(0, buff, sizeof(buff), 1);
        }
        else if(m_devType == TYPE_USB){
            mrgFindGateWay(1, buff, sizeof(buff), 1);
        }

        strFindDevices = QString("%1").arg(buff);
        if(strFindDevices.length() == 0)
        {
            sysError("mrgFindGateWay error!");
            strRet = "";
            return;
        }
        qDebug() << "find devices:" << strFindDevices;

        foreach (QString strDevice, strFindDevices.split(',', QString::SkipEmptyParts)) {
            int visa =  mrgOpenGateWay(strDevice.toLocal8Bit().data(), 800);
            if(visa <= 0) {
                continue;
            }

            char IDN[1024] = "";
            int ret = mrgGateWayIDNQuery(visa,IDN);
            qDebug() << "*IDN:" << QString(IDN);
            if(ret != 0)
            {
                mrgCloseGateWay(visa);
                continue;
            }
            mrgCloseGateWay(visa);

            QStringList lst = strDevice.split("::", QString::SkipEmptyParts);
            if(m_devType == TYPE_LAN){
                strAllDeviceList << lst.at(1) + QString(",%1").arg(IDN);
            }
            if(m_devType == TYPE_USB){
                strAllDeviceList << lst.at(0) + "_" + lst.at(1) + "_"
                                    + lst.at(2) + "_" + lst.at(3)
                                    + QString(",%1").arg(IDN);
            }
        }
        strRet = strAllDeviceList.join("::");
        return;
    };

    clearListView();
    XThread *thread = new XThread(func);
    connect(thread, SIGNAL(finished()), this, SLOT(slotDeviceScanEnd()));
    connect(thread, SIGNAL(signalFinishResult(QString)), this, SLOT(slotShowSearchResult(QString)));
    ui->pushButton_Scan->setEnabled(false);
    ui->progressBar->setMaximum(0);
    thread->start();
}

void MegaInterface::slotDeviceScanEnd()
{
    ui->pushButton_Scan->setEnabled(true);
    ui->progressBar->setMaximum(100);
}

void MegaInterface::slotShowSearchResult(QString strDevices)
{
    QStringList strListHeader;
    if(m_devType == TYPE_LAN)
    {
        strListHeader << tr("IP") << tr("Manufacturer") << tr("Type") << tr("SN") << tr("Version");
    }else if(m_devType == TYPE_USB)
    {
        strListHeader << tr("USBID") << tr("Manufacturer") << tr("Type") << tr("SN") << tr("Version");
    }
    m_model->setHorizontalHeaderLabels(strListHeader);

    foreach (QString strDev, strDevices.split("::", QString::SkipEmptyParts)) {
        int maxRow = m_model->rowCount();
        QStringList strListInfo = strDev.split(',', QString::SkipEmptyParts);
        for(int index=0; index<strListInfo.count(); index++)
        {
            QStandardItem *t_item = new QStandardItem(strListInfo.at(index));
            m_itemList.append(t_item);
            m_model->setItem(maxRow, index, t_item);
        }
    }

    //自动调整列宽
    ui->tableView->resizeColumnsToContents();
    for(int i = 0; i < ui->tableView->horizontalHeader()->count() - 1; i++){
        ui->tableView->setColumnWidth(i, ui->tableView->columnWidth(i) + 10);
    }
    ui->tableView->horizontalHeader()->setStretchLastSection(true);
}

void MegaInterface::clearListView()
{
    for(int i=0; i<m_itemList.count(); i++)
    {
        delete m_itemList.at(i);
    }
    m_itemList.clear();
    m_model->clear();
}

void MegaInterface::slotShowContextmenu(const QPoint& pos)
{
    if(!((ui->tableView->selectionModel()->selectedIndexes()).empty()))
    {
        if(m_menu != NULL)
            delete m_menu;

        m_menu = new QMenu(ui->tableView);
        QAction *actionOpen = m_menu->addAction(tr("Identify ON"));
        QAction *actionClose = m_menu->addAction(tr("Identify OFF"));

        connect(actionOpen, SIGNAL(triggered(bool)), this, SLOT(soltActionOpen()));
        connect(actionClose, SIGNAL(triggered(bool)), this, SLOT(soltActionClose()));

        m_menu->exec(QCursor::pos());
        ui->tableView->selectionModel()->clear();
    }
}

void MegaInterface::soltActionOpen()
{
    int visa = deviceOpen();
    if(visa <= 0)
        return;

    mrgIdentify(visa, 1);
    mrgCloseGateWay(visa);
}

void MegaInterface::soltActionClose()
{
    int visa = deviceOpen();
    if(visa <= 0)
        return;

    mrgIdentify(visa, 0);
    mrgCloseGateWay(visa);
}

void MegaInterface::slotSelectDevices()
{
    QString strDevInfo = "";
    QList<QModelIndex> modelList =  ui->tableView->selectionModel()->selectedIndexes();
    for(int i=0;i<modelList.count(); i++)
    {
        strDevInfo += m_model->data(modelList.at(i),Qt::DisplayRole).toString();
        strDevInfo += ",";
    }
    if(strDevInfo.split(",").count() >= 5)
    {
        emit signalSelectedInfo(strDevInfo);
    }
}

int MegaInterface::deviceOpen()
{
    QModelIndex index = ui->tableView->selectionModel()->selectedIndexes().at(0);
    QString strID = m_model->data(index,Qt::DisplayRole).toString();

    QString strDesc;
    if(m_devType == TYPE_LAN)
    {
        strDesc = QString("TCPIP0::%1::inst0::INSTR").arg(strID);
    }
    else if(m_devType == TYPE_USB)
    {
        //USB0::0xA1B2::0x5722::MRHT00000000000001::INSTR
        QStringList lst = strID.split('_', QString::SkipEmptyParts);
        strDesc = QString("%1::%2::%3::%4::INSTR")
                .arg(lst.at(0))
                .arg(lst.at(1))
                .arg(lst.at(2))
                .arg(lst.at(3));
    }

    int visa =  mrgOpenGateWay(strDesc.toLocal8Bit().data(), 800);
    if(visa <= 0){
        QMessageBox::critical(this,tr("error"),tr("open device error"));
    }
    return visa;
}

void MegaInterface::on_buttonBox_clicked(QAbstractButton *button)
{
    QDialogButtonBox::ButtonRole role = ui->buttonBox->buttonRole( button );
    if ( QDialogButtonBox::AcceptRole == role )
    {
        slotSelectDevices();
    }
    close();
}

#include "h2jogmode.h"
#include "ui_h2jogmode.h"

H2JogMode::H2JogMode(QWidget *parent) :
    XConfig(parent),
    ui(new Ui::H2JogMode)
{
    ui->setupUi(this);

    setFocuHelpName( "Jogmode" );
}

H2JogMode::~H2JogMode()
{
    delete ui;
}

int H2JogMode::readDeviceConfig()
{
    return 0;
}

int H2JogMode::writeDeviceConfig()
{
    m_CrawlingVelocity = ui->doubleSpinBox_CrawlingVelocity->value();
    m_CrawlingTime     = ui->doubleSpinBox_CrawlingTime->value();
    m_MaximumVelocity  = ui->doubleSpinBox_MaximumVelocity->value();

    return 0;
}

int H2JogMode::loadConfig()
{
    //! load xml
    MegaXML mXML;
    QString fileName = MCTHomeDIR() + "/robots/" + mConfigFileName + ".xml";
    QFile file(fileName);
    if( !file.exists() )
        fileName = qApp->applicationDirPath() + "/robots/default.xml";

    QMap<QString,QString> map = mXML.xmlRead(fileName);
    if(map.isEmpty()) return -1;

    m_CrawlingVelocity = map["CrawlingVelocity"].toDouble();
    m_CrawlingTime     = map["CrawlingTime"].toDouble();
    m_MaximumVelocity  = map["MaximumVelocity"].toDouble();

    return 0;
}

int H2JogMode::saveConfig()
{
    MegaXML mXML;
    QString fileName = MCTHomeDIR() + "/robots/" + mConfigFileName + ".xml";
    QMap<QString,QString> map;

    map.insert("CrawlingVelocity",  QString::number(m_CrawlingVelocity,10,2));
    map.insert("CrawlingTime",      QString::number(m_CrawlingTime,10,2));
    map.insert("MaximumVelocity",   QString::number(m_MaximumVelocity,10,2));

    mXML.xmlNodeRemove(fileName,"H2JogMode");
    mXML.xmlNodeAppend(fileName, "H2JogMode", map);

    return 0;
}

void H2JogMode::updateShow()
{
    ui->doubleSpinBox_CrawlingVelocity->setValue(m_CrawlingVelocity);
    ui->doubleSpinBox_CrawlingTime->setValue(m_CrawlingTime);
    ui->doubleSpinBox_MaximumVelocity->setValue(m_MaximumVelocity);
}

void H2JogMode::translateUI()
{
    ui->retranslateUi(this);
}

#include "h2homing.h"
#include "ui_h2homing.h"

H2Homing::H2Homing(QWidget *parent) :
    XConfig(parent),
    ui(new Ui::H2Homing)
{
    ui->setupUi(this);

    setFocuHelpName( "Homing" );
    ui->comboBox_target->setEnabled(false);
    ui->comboBox_movement->setEnabled(false); //暂不可选,结构不支持，没有对应的图片
}

H2Homing::~H2Homing()
{
    delete ui;
}

int H2Homing::readDeviceConfig()
{
    int ret = mrgGetRobotHomeMode(mViHandle,mRobotName);
    qDebug() << "mrgGetRobotHomeMode" << ret;
    if(ret < 0){
        sysError("mrgGetRobotHomeMode", ret);
        return -1;
    }
    m_HomingMode = ret;

    return 0;
}

int H2Homing::writeDeviceConfig()
{
    m_Target = ui->comboBox_target->currentText();
    m_HomingMode = ui->comboBox_movement->currentIndex();
    m_SearchVelocity = ui->doubleSpinBox_SearchVelocity->value();
    m_ForceLimit = ui->doubleSpinBox_ForceLimit->value();

    int ret = mrgSetRobotHomeMode(mViHandle,mRobotName, m_HomingMode);
    qDebug() << "mrgSetRobotHomeMode" << ret;
    if(ret != 0){
        sysError("mrgSetRobotHomeMode", ret);
        return -1;
    }

    return 0;
}

int H2Homing::loadConfig()
{
    //! load xml
    MegaXML mXML;
    QString fileName = MCTHomeDIR() + "/robots/" + mConfigFileName + ".xml";
    QFile file(fileName);
    if( !file.exists() )
        fileName = qApp->applicationDirPath() + "/robots/default.xml";

    QMap<QString,QString> map = mXML.xmlRead(fileName);
    if(map.isEmpty()) return -1;

    m_Target        = map["Target"];
    m_HomingMode      = map["HomingMode"].toInt();
    m_SearchVelocity = map["SearchVelocity"].toDouble();
    m_ForceLimit     = map["ForceLimit"].toDouble();

    return 0;
}

int H2Homing::saveConfig()
{
    MegaXML mXML;
    QString fileName = MCTHomeDIR() + "/robots/" + mConfigFileName + ".xml";
    QMap<QString,QString> map;

    map.insert("Target", ui->comboBox_target->currentText() );
    map.insert("HomingMode", QString::number(m_HomingMode,10) );
    map.insert("SearchVelocity", QString::number(m_SearchVelocity,10,2));
    map.insert("ForceLimit", QString::number(m_ForceLimit,10,2));

    mXML.xmlNodeRemove(fileName,"H2Homing");
    mXML.xmlNodeAppend(fileName, "H2Homing", map);

    return 0;
}

void H2Homing::updateShow()
{
    ui->comboBox_target->setCurrentText(this->m_Target);
    ui->comboBox_movement->setCurrentIndex(m_HomingMode);
    ui->doubleSpinBox_SearchVelocity->setValue(m_SearchVelocity);
    ui->doubleSpinBox_ForceLimit->setValue(m_ForceLimit);
}

void H2Homing::slot_set_direction(QString text)
{
    ui->label_direction->setText(text);
}

void H2Homing::translateUI()
{
    ui->retranslateUi(this);
}

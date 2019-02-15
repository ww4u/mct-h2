#include "h2homing.h"
#include "ui_h2homing.h"

H2Homing::H2Homing(QWidget *parent) :
    XConfig(parent),
    ui(new Ui::H2Homing)
{
    ui->setupUi(this);

    setFocuHelpName( "Homing" );
}

H2Homing::~H2Homing()
{
    delete ui;
}

int H2Homing::readDeviceConfig()
{
    return 0;
}

int H2Homing::writeDeviceConfig()
{

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
    m_Movement      = map["Movement"];

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

    map.insert("Movement", ui->comboBox_movement->currentText() );
    map.insert("SearchVelocity", QString::number(m_SearchVelocity,10,2));
    map.insert("ForceLimit", QString::number(m_ForceLimit,10,2));

    mXML.xmlNodeRemove(fileName,"H2Homing");
    mXML.xmlNodeAppend(fileName, "H2Homing", map);

    return 0;
}

void H2Homing::updateShow()
{
    ui->comboBox_target->setCurrentText(this->m_Target);
    ui->comboBox_movement->setCurrentText(this->m_Movement);
    ui->doubleSpinBox_SearchVelocity->setValue(m_SearchVelocity);
    ui->doubleSpinBox_ForceLimit->setValue(m_ForceLimit);
}

void H2Homing::on_comboBox_target_currentIndexChanged(const QString &arg1)
{
    m_Target = arg1;
    emit signalModelDataChanged(true);
}

void H2Homing::slot_set_direction(QString text)
{
    ui->label_direction->setText(text);
    emit signalModelDataChanged(true);
}

void H2Homing::on_comboBox_movement_currentIndexChanged(const QString &arg1)
{
    m_Movement = arg1;
    emit signalModelDataChanged(true);
}

void H2Homing::on_doubleSpinBox_SearchVelocity_valueChanged(double arg1)
{
    m_SearchVelocity = arg1;
    emit signalModelDataChanged(true);
}

void H2Homing::on_doubleSpinBox_ForceLimit_valueChanged(double arg1)
{
    m_ForceLimit = arg1;
    emit signalModelDataChanged(true);
}

void H2Homing::translateUI()
{
    ui->retranslateUi(this);
}

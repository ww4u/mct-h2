#include "h2configuration.h"
#include "ui_h2configuration.h"

H2Configuration::H2Configuration(QWidget *parent) :
    XConfig(parent),
    ui(new Ui::H2Configuration)
{
    ui->setupUi(this);

    m_WorkStrokeX = 0;
    m_WorkStrokeY = 0;

    setFocuHelpName( "Configuration" );

    ui->label_family->setText("MRX-H2");

    ui->doubleSpinBox_X->setEnabled(false);
    ui->doubleSpinBox_Y->setEnabled(false);
    connect(ui->doubleSpinBox_X,SIGNAL(valueChanged(double)),this,SIGNAL(WorkStrokeXChanged(double)));
    connect(ui->doubleSpinBox_Y,SIGNAL(valueChanged(double)),this,SIGNAL(WorkStrokeYChanged(double)));

    ui->sizeComboBox->setEnabled(false); //! 只有H2M,暂时不可选
}

H2Configuration::~H2Configuration()
{
    delete ui;
}

int H2Configuration::readDeviceConfig()
{
    if(mViHandle <= 0)
        return 1;

    int ret = -1;
    int type = mrgGetRobotType(mViHandle, mRobotName);
    qDebug() << "mrgGetRobotType" << type;
    if (type == MRX_T4){
        m_Family = "MRX-T4";
    }
    else if (type == MRX_AS){
        m_Family = "MRX-AS";
    }
    else if (type == MRX_H2){
        m_Family = "MRX-H2";
    }
    else if (type == MRX_DELTA){
        m_Family = "MRX-DELTA";
    }
    else if (type == MRX_RAW){
        m_Family = "MRX-RAW";
    }else{
        sysError("mrgGetRobotType error", type);
        return -1;
    }

    ret = mrgGetRobotSubType(mViHandle, mRobotName);
    qDebug() << "mrgGetRobotSubType" << ret;
    if(ret < 0) {
        sysError("mrgGetRobotSubType", ret);
        return -1;
    }
    m_Size = ret;

    return 0;
}

int H2Configuration::writeDeviceConfig()
{
    int ret = -1;

    m_Size        = ui->sizeComboBox->currentIndex();
    m_WorkStrokeX = ui->doubleSpinBox_X->value();
    m_WorkStrokeY = ui->doubleSpinBox_Y->value();

    //type:0==>H2S, 1==>H2M, 2==>H2L
    ret = mrgSetRobotSubType(mViHandle, mRobotName, m_Size);
    qDebug() << "mrgSetRobotSubType" << ret;
    if(ret != 0){
        sysError("mrgSetRobotSubType", ret);
        return -1;
    }

    return ret;
}

int H2Configuration::loadConfig()
{
    //! load xml
    MegaXML mXML;
    QString fileName = MCTHomeDIR() + "/robots/" + mConfigFileName + ".xml";
    QFile file(fileName);
    if( !file.exists() )
        fileName = qApp->applicationDirPath() + "/robots/default.xml";

    QMap<QString,QString> map = mXML.xmlRead(fileName);
    if(map.isEmpty()) return -1;

    m_Family = map["Family"];
    m_Size = map["Size"].toInt();

    return 0;
}

int H2Configuration::saveConfig()
{
    MegaXML mXML;
    QString fileName = MCTHomeDIR() + "/robots/" + mConfigFileName + ".xml";
    QMap<QString,QString> map;

    map.insert( "Family", m_Family);
    map.insert( "Size", QString::number(m_Size));
    map.insert( "WorkStrokeX", QString::number(m_WorkStrokeX));
    map.insert( "WorkStrokeY", QString::number(m_WorkStrokeY));

    mXML.xmlNodeRemove(fileName,"H2Configuration");
    mXML.xmlNodeAppend(fileName, "H2Configuration", map);

    return 0;
}

void H2Configuration::updateShow()
{
    ui->label_family->setText(m_Family);
    ui->sizeComboBox->setCurrentIndex(m_Size);
    ui->doubleSpinBox_X->setValue(m_WorkStrokeX);
    ui->doubleSpinBox_Y->setValue(m_WorkStrokeY);

    on_sizeComboBox_currentIndexChanged(m_Size);
}

void H2Configuration::on_sizeComboBox_currentIndexChanged(int index)
{
    if(index < 0) return;
    m_Size = index;

    if(index == 0){
        //! MRX-H2-S
        ui->doubleSpinBox_X->setValue(0); //! TODO
        ui->doubleSpinBox_Y->setValue(0); //! TODO
    }
    else if(index == 1){
        //! MRX-H2-M 355 x 375
        ui->doubleSpinBox_X->setValue(355);
        ui->doubleSpinBox_Y->setValue(375);
    }
    else if(index == 2){
        //! MRX-H2-L
        ui->doubleSpinBox_X->setValue(0); //! TODO
        ui->doubleSpinBox_Y->setValue(0); //! TODO
    }
    else{
        //! 定制
    }

    QString model = "";
    m_Family = ui->label_family->text();
    model += m_Family;

    if(ui->sizeComboBox->currentIndex() == 0){
        model += "-S";
    }
    else if(ui->sizeComboBox->currentIndex() == 1){
        model += "-M";
    }
    else if(ui->sizeComboBox->currentIndex() == 2){
        model += "-L";
    }
    else{

    }
    ui->label_model->setText(model);
}

void H2Configuration::translateUI()
{
    ui->retranslateUi(this);
}

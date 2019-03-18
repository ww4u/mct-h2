#include "h2configuration.h"
#include "ui_h2configuration.h"

//! 标准的小型H2
#define STD_H2S_X 99
#define STD_H2S_Y 99

//! 标准的中型H2
#define STD_H2M_X 355
#define STD_H2M_Y 375

//! 标准的大型H2
#define STD_H2L_X 999
#define STD_H2L_Y 999

//! 定制的H2
#define CUSTOM_H2_X 9999
#define CUSTOM_H2_Y 9999

H2Configuration::H2Configuration(QWidget *parent) :
    XConfig(parent),
    ui(new Ui::H2Configuration)
{
    ui->setupUi(this);

    m_WorkStrokeX = 0;
    m_WorkStrokeY = 0;

    setFocuHelpName( "Configuration" );

    ui->label_family->setText("MRX-H2");
    connect(ui->doubleSpinBox_X,SIGNAL(valueChanged(double)),this,SIGNAL(WorkStrokeXChanged(double)));
    connect(ui->doubleSpinBox_Y,SIGNAL(valueChanged(double)),this,SIGNAL(WorkStrokeYChanged(double)));

    connect(ui->comboBox_control,SIGNAL(currentIndexChanged(int)),this,SLOT(set_control_interface_picture(int)));

//    ui->sizeComboBox->setEnabled(false); //! 只有H2M,暂时不可选
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
    if (type == MRX_TYPE_T4){
        m_Family = "MRX-T4";
    }
    else if (type == MRX_TYPE_AS){
        m_Family = "MRX-AS";
    }
    else if (type == MRX_TYPE_H2){
        m_Family = "MRX-H2";
    }
    else if (type == MRX_TYPE_DELTA){
        m_Family = "MRX-DELTA";
    }
    else if (type == MRX_TYPE_RAW){
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
    if(m_Size == 3){
        //! 定制产品，获取滑块长宽，获取模具类型，获取齿轮齿数
        float links[6] = {0.0};
        int count = -1;
        ret = mrgGetRobotLinks(mViHandle,mRobotName,links,&count);
        if(ret < 0 || count < 6){
            sysError("mrgGetRobotLinks" + QString::number(count), ret);
            return -1;
        }
        m_WorkStrokeX = links[0];
        m_WorkStrokeY = links[1];
        m_SliderWidth   = links[2];
        m_SliderHeight   = links[3];
        m_MouldType = links[4];
        m_TeethQty  = links[5];
    }

    int ifce = 0;
    ret = mrgSysGetInterface(mViHandle, &ifce);
    if(ret < 0 || ifce < 0 || ifce > 2){
        sysError("mrgSysGetInterface " + QString::number(ifce), ret);
        return -1;
    }
    m_ControlInterface = ifce;

    return 0;
}

int H2Configuration::writeDeviceConfig()
{
    int ret = -1;

    m_Size        = ui->sizeComboBox->currentIndex();
    m_WorkStrokeX = ui->doubleSpinBox_X->value();
    m_WorkStrokeY = ui->doubleSpinBox_Y->value();

    m_SliderWidth   = ui->doubleSpinBox_sliderX->value();
    m_SliderHeight   = ui->doubleSpinBox_sliderY->value();
    m_MouldType = ui->doubleSpinBox_mouldType->value();
    m_TeethQty  = ui->doubleSpinBox_teethQty->value();

    if(ui->comboBox_control->currentText() == "CVE"){
        m_ControlInterface = 0;
    }
    else if(ui->comboBox_control->currentText() == "USB"){
        m_ControlInterface = 1;
    }
    else if(ui->comboBox_control->currentText() == "IO"){
        m_ControlInterface = 2;
    }

    //type:0==>H2S, 1==>H2M, 2==>H2L, 4==>定制
    ret = mrgSetRobotSubType(mViHandle, mRobotName, m_Size);
    qDebug() << "mrgSetRobotSubType" << ret;
    if(ret != 0){
        sysError("mrgSetRobotSubType", ret);
        return -1;
    }

    if(m_Size == 3){
        //! 定制产品，设置滑块长宽，设置模具类型，设置齿轮齿数
        float links[6] = {m_WorkStrokeX,m_WorkStrokeY,
                           m_SliderWidth,m_SliderHeight,
                           m_MouldType,m_TeethQty};
        ret = mrgSetRobotLinks(mViHandle, mRobotName, links, 6);
        if(ret < 0){
            sysError("mrgSetRobotLinks", ret);
            return -1;
        }
    }

    ret = mrgSysSetInterface(mViHandle, m_ControlInterface);
    if(ret < 0){
        sysError("mrgSysSetInterface", ret);
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

    m_WorkStrokeX = map["WorkStrokeX"].toFloat();
    m_WorkStrokeY = map["WorkStrokeY"].toFloat();
    m_SliderWidth  = map["SliderWidth"].toFloat();
    m_SliderHeight = map["SliderHeight"].toFloat();
    m_MouldType   = map["MouldType"].toFloat();
    m_TeethQty    = map["TeethQty"].toFloat();
    m_ControlInterface = map["ControlInterface"].toInt();

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

    map.insert( "SliderWidth", QString::number(m_SliderWidth));
    map.insert( "SliderHeight", QString::number(m_SliderHeight));
    map.insert( "MouldType", QString::number(m_MouldType));
    map.insert( "TeethQty", QString::number(m_TeethQty));
    map.insert( "ControlInterface", QString::number(m_ControlInterface));

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

    ui->doubleSpinBox_sliderX->setValue(m_SliderWidth);
    ui->doubleSpinBox_sliderY->setValue(m_SliderHeight);
    ui->doubleSpinBox_mouldType->setValue(m_MouldType);
    ui->doubleSpinBox_teethQty->setValue(m_TeethQty);

    if(m_ControlInterface == 0){
        ui->comboBox_control->setCurrentIndex(0);
    }
    else if(m_ControlInterface == 1){
        ui->comboBox_control->setCurrentIndex(-1); //无USB
    }
    else if(m_ControlInterface == 2){
        ui->comboBox_control->setCurrentIndex(1);
    }

    on_sizeComboBox_currentIndexChanged(m_Size);
}

void H2Configuration::on_sizeComboBox_currentIndexChanged(int index)
{
    if(index < 0) return;
    m_Size = index;

    if(index == 0){
        //! MRX-H2-S
        ui->doubleSpinBox_X->setValue(STD_H2S_X); //! TODO
        ui->doubleSpinBox_Y->setValue(STD_H2S_Y); //! TODO
    }
    else if(index == 1){
        //! MRX-H2-M 355 x 375
        ui->doubleSpinBox_X->setValue(STD_H2M_X);
        ui->doubleSpinBox_Y->setValue(STD_H2M_Y);
    }
    else if(index == 2){
        //! MRX-H2-L
        ui->doubleSpinBox_X->setValue(STD_H2L_X); //! TODO
        ui->doubleSpinBox_Y->setValue(STD_H2L_Y); //! TODO
    }
    else{

    }


    if(index >= 0 && index <= 2){
        ui->doubleSpinBox_X->setEnabled(false);
        ui->doubleSpinBox_Y->setEnabled(false);

        ui->doubleSpinBox_sliderX->setVisible(false);
        ui->label_sliderX->setVisible(false);

        ui->doubleSpinBox_sliderY->setVisible(false);
        ui->label_sliderY->setVisible(false);

        ui->doubleSpinBox_mouldType->setVisible(false);
        ui->label_mouldtype->setVisible(false);

        ui->doubleSpinBox_teethQty->setVisible(false);
        ui->label_teethqty->setVisible(false);
    }
    else
    {
        //! 定制
        ui->doubleSpinBox_X->setEnabled(true);
        ui->doubleSpinBox_Y->setEnabled(true);

        ui->doubleSpinBox_sliderX->setVisible(true);
        ui->label_sliderX->setVisible(true);

        ui->doubleSpinBox_sliderY->setVisible(true);
        ui->label_sliderY->setVisible(true);

        ui->doubleSpinBox_mouldType->setVisible(true);
        ui->label_mouldtype->setVisible(true);

        ui->doubleSpinBox_teethQty->setVisible(true);
        ui->label_teethqty->setVisible(true);
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
        model += "-Custom";
    }
    ui->label_model->setText(model);
}

void H2Configuration::set_control_interface_picture(int index)
{
    if(index == 0){
        m_ControlInterface = 0;
        ui->label_control->setPixmap(QPixmap(":/res/image/h2configuration/IO.png"));
    }else{
        m_ControlInterface = 1;
        ui->label_control->setPixmap(QPixmap(":/res/image/h2configuration/CVE.png"));
    }

}

void H2Configuration::translateUI()
{
    ui->retranslateUi(this);
}

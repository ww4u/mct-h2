#include "h2homing.h"
#include "ui_h2homing.h"

#define     strDirXtoY      tr("First in direction of X,then Y")
#define     strDirYtoX      tr("First in direction of Y,then X")

H2Homing::H2Homing(QWidget *parent) :
    XConfig(parent),
    ui(new Ui::H2Homing)
{
    ui->setupUi(this);


    setName( "homing" );

    ui->comboBox_target->addItem("Block");
    ui->comboBox_movement->addItem(strDirXtoY);
    ui->comboBox_movement->addItem(strDirYtoX);

    connect(ui->comboBox_movement,SIGNAL(currentTextChanged(QString)),this,SLOT(slotChangePicture(QString)));

    ui->label_picture->setPixmap(QPixmap(":/res/image/h2homing/dir_X_Y.PNG"));

}

H2Homing::~H2Homing()
{
    delete ui;
}


void H2Homing::slotChangePicture(QString str)
{
    if(str == strDirXtoY)
        ui->label_picture->setPixmap(QPixmap(":/res/image/h2homing/dir_X_Y.PNG"));
    else if(str == strDirYtoX)
        ui->label_picture->setPixmap(QPixmap(":/res/image/h2homing/dir_Y_X.PNG"));

}



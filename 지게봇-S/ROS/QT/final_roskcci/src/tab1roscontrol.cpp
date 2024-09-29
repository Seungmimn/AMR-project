#include "../include/roskcci/tab1roscontrol.h"
#include "ui_tab1roscontrol.h"
Tab1RosControl::Tab1RosControl(int argc, char **argv, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Tab1RosControl)
{
    ui->setupUi(this);
    LINEAR_MAX = 0.22; //Max 0.22
    ANGULAR_MAX = 2.84; //Max 2.84

    LINEAR_INTERVAL = LINEAR_MAX/10;
    ANGULAR_INTERVAL = ANGULAR_MAX/10;

    base_cmd.linear.x = 0.0;
    base_cmd.linear.y = 0.0;
    base_cmd.linear.z = 0.0;
    base_cmd.angular.x = 0.0;
    base_cmd.angular.y = 0.0;
    base_cmd.angular.z = 0.0;

    lift_cmd.linear.x = 0.0;
    lift_cmd.linear.y = 0.0;
    lift_cmd.linear.z = 0.0;

    linearX = 0;
    angularZ = 0;
    linearZ = 0;
    baseCmdFlag = false;
    liftCmdFlag = false;
    prosNode = new RosNode(argc, argv);
    prosNode->pLcamView = ui->pLcamView;
    prosNode->start();

    connect(ui->pushButtonGo, SIGNAL(clicked()),this, SLOT(goal_Pub()));
    connect(ui->pPBFront, SIGNAL(clicked()),this, SLOT(goal_Pub_Front()));
    connect(ui->pPBLiving, SIGNAL(clicked()),this, SLOT(goal_Pub_Living()));
    connect(ui->pPBStudy, SIGNAL(clicked()),this, SLOT(goal_Pub_Study()));
    connect(ui->pPBedroom, SIGNAL(clicked()),this, SLOT(goal_Pub_Bedroom()));
    connect(prosNode, SIGNAL(sigLdsReceive(float *)),this, SLOT(slotLdsReceive(float *)));
    connect(prosNode, SIGNAL(rosShutdown()),qApp, SLOT(quit()));

}

Tab1RosControl::~Tab1RosControl()
{
    delete ui;
}

void Tab1RosControl::goal_Pub()
{
    prosNode->go_goal("map",ui->doubleSpinBox1->value(),ui->doubleSpinBox2->value(), ui->doubleSpinBox3->value(),ui->doubleSpinBox4->value());

}
void Tab1RosControl::goal_PubSlot(double x, double y, double z, double w)
{
   prosNode->go_goal("map",x,y,z,w);
}
void Tab1RosControl::goal_Pub_Front()
{
   prosNode->go_goal("map",0.0, 0.0, 0.0, 0.9);
}
void Tab1RosControl::goal_Pub_Living()
{
   prosNode->go_goal("map",0.077, 0.19, 0.0, 0.79);
}
void Tab1RosControl::goal_Pub_Study()
{
   prosNode->go_goal("map",4.96, 6.366, 0.0, 0.62);
}
void Tab1RosControl::goal_Pub_Bedroom()
{
   prosNode->go_goal("map",7.78, 1.05, 0.0, 0.59);
}
void Tab1RosControl::set_Pub()
{
   prosNode->set_goal("map",0.83, 2.92, 0.0, 0.72);
}
void Tab1RosControl::slotLdsReceive(float *pscanData)
{
    ui->lcdNumber1->display(pscanData[0]);
    ui->lcdNumber2->display(pscanData[1]);
    ui->lcdNumber3->display(pscanData[2]);
    ui->lcdNumber4->display(pscanData[3]);
}

void Tab1RosControl::on_pPBForward_clicked()
{
    if(linearX <= 9 )
        linearX++;
    cmdVelPubSlot(linearX,angularZ);

}

void Tab1RosControl::on_pPBleft_clicked()
{
    if(angularZ <= 9 )
        angularZ++;
    cmdVelPubSlot(linearX,angularZ);
}

void Tab1RosControl::on_pPBStop_clicked()
{
    linearX =0;
    angularZ = 0;
    cmdVelPubSlot(linearX,angularZ);

}

void Tab1RosControl::on_pPBLight_clicked()
{
    if(-9 <= angularZ)
        angularZ--;
    cmdVelPubSlot(linearX,angularZ);
}

void Tab1RosControl::on_pPBBackward_clicked()
{
    if(-9 <= linearX )
        linearX--;
     cmdVelPubSlot(linearX,angularZ);
}

void Tab1RosControl::on_pPBUp_clicked()
{
    if(-9 <= linearZ )
        linearZ--;
    cmdVelLiftPubSlot(linearZ);
}

void Tab1RosControl::on_pPBStopLift_clicked()
{
    linearZ = 0;
    cmdVelLiftPubSlot(linearZ);
}

void Tab1RosControl::on_pPBDown_clicked()
{  
    if(linearZ <= 9 )
        linearZ++;
    cmdVelLiftPubSlot(linearZ);
}

void Tab1RosControl::cmdVelPubSlot(int linear,int angular)
{
    QString strLinear;
    QString strAngular;

    base_cmd.linear.x = LINEAR_INTERVAL * linear;
    base_cmd.angular.z = ANGULAR_INTERVAL * angular;

    if(linear != 0 || angular !=0 )
    {
        prosNode->set_cmd_vel(base_cmd);
        prosNode->set_cmdBaseCmd(true);
    }
    else
        prosNode->set_cmdBaseCmd(false);

    strLinear = QString::number(linear * 10);
    strLinear+='%';
    strAngular = QString::number(angular * 10);
    strAngular+='%';
    ui->pLBlinear->setText(strLinear);
    ui->pLBangular->setText(strAngular);
}

void Tab1RosControl::cmdVelLiftPubSlot(int linear)
{
    QString strLinear;

    lift_cmd.linear.z = LINEAR_INTERVAL * linear;

    if(linear != 0)
    {
        prosNode->set_cmd_vel_lift(lift_cmd);
        prosNode->set_cmdLiftCmd(true);
    }
    else
        prosNode->set_cmdLiftCmd(false);

    strLinear = QString::number(linear * 10);
    strLinear+='%';
    ui->pLBlinear->setText(strLinear);
}

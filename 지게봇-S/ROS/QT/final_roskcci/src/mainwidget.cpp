#include "../include/roskcci/mainwidget.h"
#include "ui_mainwidget.h"

MainWidget::MainWidget(int argc, char** argv, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWidget)
{
    ui->setupUi(this);
    setWindowTitle("KCCI AIOT ROS");
    pTab1RosControl = new Tab1RosControl(argc, argv, ui->pTab1);
    ui->pTab1->setLayout(pTab1RosControl->layout());

    pTab2SocketClient = new Tab2SocketClient(ui->pTab2);
    ui->pTab2->setLayout(pTab2SocketClient->layout());

    connect(pTab2SocketClient,SIGNAL(goGoalSig(double, double, double, double)),pTab1RosControl,SLOT(goal_PubSlot(double, double, double, double)));
    connect(pTab2SocketClient,SIGNAL(cmdVelPubSig(int, int)),pTab1RosControl,SLOT(cmdVelPubSlot(int,int)));
    connect(pTab2SocketClient,SIGNAL(cmdVelLiftPubSig(int)),pTab1RosControl,SLOT(cmdVelLiftPubSlot(int)));
}

MainWidget::~MainWidget()
{
    delete ui;
}


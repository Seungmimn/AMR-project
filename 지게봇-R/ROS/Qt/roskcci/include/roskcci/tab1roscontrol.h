#ifndef TAB1ROSCONTROL_H
#define TAB1ROSCONTROL_H

#include <QWidget>
#include <QButtonGroup>
#include <QPushButton>
#include <QDebug>
#include <iostream>
#include <array>
#include "rosnode.h"

namespace Ui {
class Tab1RosControl;
}

class Tab1RosControl : public QWidget
{
    Q_OBJECT

public:
    explicit Tab1RosControl(int argc, char **argv, QWidget *parent = nullptr);
    ~Tab1RosControl();

private:
    Ui::Tab1RosControl *ui;
    RosNode *prosNode;
    QButtonGroup *bg;
    QWidget *widget;
    geometry_msgs::Twist base_cmd;
    geometry_msgs::Twist lift_cmd;
    bool baseCmdFlag;
    bool liftCmdFlag;
    int linearX;
    int angularZ;
    int linearZ;
    float LINEAR_MAX;
    float ANGULAR_MAX;

    float LINEAR_INTERVAL;
    float ANGULAR_INTERVAL;

private slots:
    void goal_Pub();
    void goal_PubSlot(double, double, double, double);
    void goal_Pub_Front();
    void goal_Pub_Living();
    void goal_Pub_Study();
    void goal_Pub_Bedroom();
    void set_Pub();
    void slotLdsReceive(float *);
    void on_pPBForward_clicked();
    void on_pPBleft_clicked();
    void on_pPBStop_clicked();
    void on_pPBLight_clicked();
    void on_pPBBackward_clicked();
    void cmdVelPubSlot(int,int);
    void cmdVelLiftPubSlot(int);

    void on_pPBUp_clicked();
    void on_pPBStopLift_clicked();
    void on_pPBDown_clicked();
};

#endif // TAB1ROSCONTROL_H

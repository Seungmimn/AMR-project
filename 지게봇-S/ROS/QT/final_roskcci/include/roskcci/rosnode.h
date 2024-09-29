/* kccistc embedded & iot by ksh */
#ifndef __ROSNODE__H_
#define __ROSNODE__H_


#include <QThread>
#include <QSettings>
#include <QLabel>
#include <QDebug>
#include <sstream>
#include <string>

#include <ros/ros.h>
#include <std_msgs/String.h>
#include <geometry_msgs/PoseStamped.h>
#include <geometry_msgs/PoseWithCovarianceStamped.h>
#include <geometry_msgs/Twist.h>
#include <sensor_msgs/LaserScan.h>
#include <image_transport/image_transport.h>
#include <opencv2/highgui/highgui.hpp>
#include <cv_bridge/cv_bridge.h>

class RosNode : public QThread{
    Q_OBJECT
public:
    RosNode(int argc, char **argv);
    ~RosNode();
    bool init();
    void run();
    void startHandle();
    void set_goal(QString frame, double x, double y, double z, double w);
    void go_goal(QString frame, double x, double y, double z, double w);
    void set_cmd_vel(geometry_msgs::Twist);
    void set_cmd_vel_lift(geometry_msgs::Twist);
    void LDSMsgCallback(const sensor_msgs::LaserScan::ConstPtr &msg);
    void imageCallback(const sensor_msgs::ImageConstPtr& msg);

private:
    int init_argc;
    char **init_argv;
    QString goal_topic;
    geometry_msgs::Twist base_cmd;
    geometry_msgs::Twist lift_cmd;
    bool baseCmdFlag;
    bool liftCmdFlag;
    ros::Publisher set_pub;
    ros::Publisher goal_pub;
    ros::Publisher cmd_vel_pub;
    ros::Publisher cmd_vel_lift_pub;
    ros::Subscriber lds_sub;

    ros::Subscriber image_sub;




public:
    QLabel* pLcamView;
    void set_cmdBaseCmd(bool);
    void set_cmdLiftCmd(bool);
signals:
    void rosShutdown();
    void sigLdsReceive(float *);
};

#endif

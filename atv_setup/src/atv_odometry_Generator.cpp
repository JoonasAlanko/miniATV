#include <ros/ros.h>
// publishing both a transform from the "odom" coordinate frame to the "base_link" coordinate frame and
// a nav_msgs/Odometry message, we need to include the relevant header files.
#include <tf/transform_broadcaster.h>
#include <nav_msgs/Odometry.h>

int main(int argc, char** argv){
  ros::init(argc, argv, "odometry_publisher");

  ros::NodeHandle n;
// Create both a ros::Publisher and a tf::TransformBroadcaster to be able to send messages out using ROS
// and tf respectively.
  ros::Publisher odom_pub = n.advertise<nav_msgs::Odometry>("odom", 50);
  tf::TransformBroadcaster odom_broadcaster;
// Assume that the robot starts at the origin of the "odom" coordinate frame initially.
  double x = 0.0;
  double y = 0.0;
  double th = 0.0;
// Speeds
  double vx = 0.1;
  double vy = 0.-1;
  double vth = 0.1;

  ros::Time current_time, last_time;
  current_time = ros::Time::now();
  last_time = ros::Time::now();
// Publish odometry information at a rate of 5Hz
  ros::Rate r(5.0);
  while(n.ok()){

    ros::spinOnce();               // check for incoming messages
    current_time = ros::Time::now();
// Update our odometry information based on the constant velocities we set.
    //compute odometry in a typical way given the velocities of the robot
    double dt = (current_time - last_time).toSec();
    double delta_x = (vx * cos(th) - vy * sin(th)) * dt;
    double delta_y = (vx * sin(th) + vy * cos(th)) * dt;
    double delta_th = vth * dt;

   x += delta_x;
   y += delta_y;
   th += delta_th;
// Try to use 3D versions of all messages in our system to allow 2D and 3D components to work together
// when appropriate, and to keep the number of messages we have to create to a minimum. As such,
// it is necessary to convert our yaw value for odometry into a Quaternion to send over the wire.
// Fortunately, tf provides functions allowing easy creation of Quaternions from yaw values and easy
// access to yaw values from Quaternions.
    //since all odometry is 6DOF we'll need a quaternion created from yaw
    geometry_msgs::Quaternion odom_quat = tf::createQuaternionMsgFromYaw(th);
// Create a TransformStamped message that we will send out over tf. We want to publish the transform from
// the "odom" frame to the "base_link" frame at current_time. Therefore, we'll set the header of
// the message and the child_frame_id accordingly, making sure to use "odom" as the parent coordinate
// frame and "base_link" as the child coordinate frame.
    //first, we'll publish the transform over tf
    geometry_msgs::TransformStamped odom_trans;
    odom_trans.header.stamp = current_time;
    odom_trans.header.frame_id = "odom";
//    odom_trans.child_frame_id = "base_link";
      odom_trans.child_frame_id = "base_footprint";
// Fill in the transform message from our odometry data, and then send the transform using our TransformBroadcaster.
    odom_trans.transform.translation.x = x;
    odom_trans.transform.translation.y = y;
    odom_trans.transform.translation.z = 0.0;
    odom_trans.transform.rotation = odom_quat;

    //send the transform
    odom_broadcaster.sendTransform(odom_trans);
// Need to publish a nav_msgs/Odometry message so that the navigation stack can get velocity information
// from it. Set the header of the message to the current_time and the "odom" coordinate frame.
    //next, we'll publish the odometry message over ROS
    nav_msgs::Odometry odom;
    odom.header.stamp = current_time;
    odom.header.frame_id = "odom";
// Populates the message with odometry data and sends it out over the wire. We'll set the child_frame_id
// of the message to be the "base_link" frame since that's the coordinate frame we're sending our velocity
// information in.
    //set the position
    odom.pose.pose.position.x = x;
    odom.pose.pose.position.y = y;
    odom.pose.pose.position.z = 0.0;
    odom.pose.pose.orientation = odom_quat;

    //set the velocity
//  odom.child_frame_id = "base_link"; 
    odom.child_frame_id = "base_footprint";
    odom.twist.twist.linear.x = vx;
    odom.twist.twist.linear.y = vy;
    odom.twist.twist.angular.z = vth;

    //publish the message
    odom_pub.publish(odom);

    last_time = current_time;
    r.sleep();
  }
}
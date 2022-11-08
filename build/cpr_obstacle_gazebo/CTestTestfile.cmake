# CMake generated Testfile for 
# Source directory: /home/jaime/Desktop/DynamicEntropyExplore/src/cpr_gazebo/cpr_obstacle_gazebo
# Build directory: /home/jaime/Desktop/DynamicEntropyExplore/build/cpr_obstacle_gazebo
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(_ctest_cpr_obstacle_gazebo_roslaunch-check_launch "/home/jaime/Desktop/DynamicEntropyExplore/build/cpr_obstacle_gazebo/catkin_generated/env_cached.sh" "/usr/bin/python2" "/opt/ros/melodic/share/catkin/cmake/test/run_tests.py" "/home/jaime/Desktop/DynamicEntropyExplore/build/cpr_obstacle_gazebo/test_results/cpr_obstacle_gazebo/roslaunch-check_launch.xml" "--return-code" "/usr/bin/cmake -E make_directory /home/jaime/Desktop/DynamicEntropyExplore/build/cpr_obstacle_gazebo/test_results/cpr_obstacle_gazebo" "/opt/ros/melodic/share/roslaunch/cmake/../scripts/roslaunch-check -o \"/home/jaime/Desktop/DynamicEntropyExplore/build/cpr_obstacle_gazebo/test_results/cpr_obstacle_gazebo/roslaunch-check_launch.xml\" \"/home/jaime/Desktop/DynamicEntropyExplore/src/cpr_gazebo/cpr_obstacle_gazebo/launch\" ")
subdirs("gtest")

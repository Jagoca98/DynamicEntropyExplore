# CMake generated Testfile for 
# Source directory: /home/jaime/Desktop/DynamicEntropyExplore/src/husky/husky_viz
# Build directory: /home/jaime/Desktop/DynamicEntropyExplore/build/husky_viz
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(_ctest_husky_viz_roslaunch-check_launch "/home/jaime/Desktop/DynamicEntropyExplore/build/husky_viz/catkin_generated/env_cached.sh" "/usr/bin/python2" "/opt/ros/melodic/share/catkin/cmake/test/run_tests.py" "/home/jaime/Desktop/DynamicEntropyExplore/build/husky_viz/test_results/husky_viz/roslaunch-check_launch.xml" "--return-code" "/usr/bin/cmake -E make_directory /home/jaime/Desktop/DynamicEntropyExplore/build/husky_viz/test_results/husky_viz" "/opt/ros/melodic/share/roslaunch/cmake/../scripts/roslaunch-check -o \"/home/jaime/Desktop/DynamicEntropyExplore/build/husky_viz/test_results/husky_viz/roslaunch-check_launch.xml\" \"/home/jaime/Desktop/DynamicEntropyExplore/src/husky/husky_viz/launch\" ")
subdirs("gtest")

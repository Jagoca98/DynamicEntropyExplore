#!/bin/sh

if [ -n "$DESTDIR" ] ; then
    case $DESTDIR in
        /*) # ok
            ;;
        *)
            /bin/echo "DESTDIR argument must be absolute... "
            /bin/echo "otherwise python's distutils will bork things."
            exit 1
    esac
fi

echo_and_run() { echo "+ $@" ; "$@" ; }

echo_and_run cd "/home/jaime/Desktop/DynamicEntropyExplore/src/navigation/base_local_planner"

# ensure that Python install destination exists
echo_and_run mkdir -p "$DESTDIR/home/jaime/Desktop/DynamicEntropyExplore/install/lib/python2.7/dist-packages"

# Note that PYTHONPATH is pulled from the environment to support installing
# into one location when some dependencies were installed in another
# location, #123.
echo_and_run /usr/bin/env \
    PYTHONPATH="/home/jaime/Desktop/DynamicEntropyExplore/install/lib/python2.7/dist-packages:/home/jaime/Desktop/DynamicEntropyExplore/build/base_local_planner/lib/python2.7/dist-packages:$PYTHONPATH" \
    CATKIN_BINARY_DIR="/home/jaime/Desktop/DynamicEntropyExplore/build/base_local_planner" \
    "/usr/bin/python2" \
    "/home/jaime/Desktop/DynamicEntropyExplore/src/navigation/base_local_planner/setup.py" \
     \
    build --build-base "/home/jaime/Desktop/DynamicEntropyExplore/build/base_local_planner" \
    install \
    --root="${DESTDIR-/}" \
    --install-layout=deb --prefix="/home/jaime/Desktop/DynamicEntropyExplore/install" --install-scripts="/home/jaime/Desktop/DynamicEntropyExplore/install/bin"

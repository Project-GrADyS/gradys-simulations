docker build --build-arg OMNETPP_VERSION=6.0.1 .

docker run --rm -ti -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix -t [IMAGE_ID] bash

omnetpp

Install INET
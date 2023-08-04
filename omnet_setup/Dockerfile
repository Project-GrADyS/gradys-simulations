# This dockerfile is based on the work from omnetpy 
# The original can be found here: https://github.com/mmodenesi/omnetpy 

FROM ubuntu:20.04 as base

# install basic packages
RUN apt-get update && apt-get dist-upgrade -y \
    && DEBIAN_FRONTEND=noninteractive apt-get -y install \
        locales sudo wget \
        build-essential clang lld gdb bison flex perl gcc g++ \
        python3 python3-pip qtbase5-dev qtchooser qt5-qmake qtbase5-dev-tools \
        libqt5opengl5-dev libxml2-dev zlib1g-dev doxygen graphviz libwebkit2gtk-4.0-37 \
        openscenegraph-plugin-osgearth libosgearth-dev libopenscenegraph-dev \
        mpi-default-dev \
        xdg-utils \
        && rm -rf /var/lib/apt/lists/*

RUN pip3 install numpy pandas matplotlib scipy seaborn posix_ipc

# setup locale
RUN locale-gen en_US.UTF-8
ENV LC_ALL=en_US.UTF-8 LANG=en_US.UTF-8

ENV USERNAME userpp
ENV UID 1000
ENV GID 1000
ENV HOME /home/${USERNAME}

# inspired by http://fabiorehm.com/blog/2014/09/11/running-gui-apps-with-docker/
RUN groupadd --gid $GID $USERNAME
RUN useradd -m -d $HOME -s /bin/bash --uid $UID --gid $GID -G sudo $USERNAME
RUN echo "${USERNAME} ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers
RUN mkdir -p $HOME/workspace && chown -R $USERNAME:$USERNAME $HOME
WORKDIR $HOME/workspace
VOLUME "${HOME}/workspace"

# =================================================================================================
# End base stage
# =================================================================================================

FROM base as omnetpp

ARG OMNETPP_VERSION
ENV OMNETPP_ROOT $HOME/omnetpp-${OMNETPP_VERSION}

USER $USERNAME
ADD bashrc $HOME/.bashrc
RUN echo '. ~/.bashrc' >> ~/.bash_profile

# get source code
RUN wget -P $HOME --progress=dot:giga \
    https://github.com/omnetpp/omnetpp/releases/download/omnetpp-${OMNETPP_VERSION}/omnetpp-${OMNETPP_VERSION}-linux-x86_64.tgz \
    && tar xzf $HOME/omnetpp-${OMNETPP_VERSION}-linux-x86_64.tgz --directory $HOME \
    && rm $HOME/omnetpp-${OMNETPP_VERSION}-linux-x86_64.tgz

# configure and compile
RUN export PATH=$OMNETPP_ROOT/bin:$PATH; cd $OMNETPP_ROOT && . ./setenv && ./configure && make -j4

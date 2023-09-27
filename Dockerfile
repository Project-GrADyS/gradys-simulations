FROM ubuntu:20.04 as base
MAINTAINER Marcos Modenesi <marcosmodenesi@gmail.com>

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
RUN mkdir -p $HOME/workspace/gradys-simulations && chown -R $USERNAME:$USERNAME $HOME

# Install python
RUN apt-get update && apt-get dist-upgrade -y \
        && rm -rf /var/lib/apt/lists/*

#Set of all dependencies needed for pyenv to work on Ubuntu
RUN apt-get update \ 
        && apt-get install -y --no-install-recommends make build-essential libssl-dev zlib1g-dev libbz2-dev libreadline-dev libsqlite3-dev wget ca-certificates curl llvm libncurses5-dev xz-utils tk-dev libxml2-dev libxmlsec1-dev libffi-dev liblzma-dev mecab-ipadic-utf8 git

USER $USERNAME
ADD bashrc $HOME/.bashrc
RUN echo '. ~/.bashrc' >> ~/.bash_profile

WORKDIR $HOME

USER $ROOT

RUN git clone https://github.com/pyenv/pyenv.git $HOME/.pyenv

RUN echo 'export PYENV_ROOT="$HOME/.pyenv"' >>  $HOME/.bashrc
RUN echo 'command -v pyenv >/dev/null || export PATH="$PYENV_ROOT/bin:$PATH"' >>  $HOME/.bashrc
RUN echo 'eval "$(pyenv init -)"' >>  $HOME/.bashrc

RUN bash -i -c "source ~/.bashrc && pyenv install 3.11-dev && pyenv global 3.11-dev"

RUN pip install numpy pandas matplotlib scipy seaborn posix_ipc

USER $USERNAME

WORKDIR $HOME/workspace
VOLUME "${HOME}/workspace"

ENV GRADYS_SIMULATIONS_ROOT $HOME/workspace/gradys-simulations

# Copy gradys simulation content
ADD . $GRADYS_SIMULATIONS_ROOT

# =================================================================================================
# End base stage
# =================================================================================================

FROM base as omnetpp

ARG OMNETPP_VERSION
ENV OMNETPP_ROOT $HOME/omnetpp-${OMNETPP_VERSION}

# get source code
RUN wget -P $HOME --progress=dot:giga \
    https://github.com/omnetpp/omnetpp/releases/download/omnetpp-${OMNETPP_VERSION}/omnetpp-${OMNETPP_VERSION}-linux-x86_64.tgz \
    && tar xzf $HOME/omnetpp-${OMNETPP_VERSION}-linux-x86_64.tgz --directory $HOME \
    && rm $HOME/omnetpp-${OMNETPP_VERSION}-linux-x86_64.tgz

RUN cp -f $GRADYS_SIMULATIONS_ROOT/docker_setup/configure.user $OMNETPP_ROOT/configure.user

# configure and compile
RUN export PATH=$OMNETPP_ROOT/bin:$PATH; cd $OMNETPP_ROOT && . ./setenv && ./configure && make -j4

# =================================================================================================
# End omnetpp stage
# =================================================================================================

FROM omnetpp as inet

ARG INET_VERSION
ENV INET_ROOT $HOME/workspace/inet${INET_VERSION}

USER $USERNAME

# get source code
RUN wget -P $HOME --progress=dot:giga \
    https://github.com/inet-framework/inet/releases/download/v${INET_VERSION}.0/inet-${INET_VERSION}.0-src.tgz \
    && tar xzf $HOME/inet-${INET_VERSION}.0-src.tgz --directory $HOME/workspace \
    && rm $HOME/inet-${INET_VERSION}.0-src.tgz

# configure and compile
RUN export PATH=$OMNETPP_ROOT/bin:$PATH; cd $INET_ROOT && . ./setenv && make makefiles && make -j4

# =================================================================================================
# End inet stage
# =================================================================================================

FROM inet as configuration

# Build and install gradys simulations as well as python package
RUN cd $GRADYS_SIMULATIONS_ROOT/src/gradys-sim-prototype && pip install .

# Copy omnet project settings
RUN mkdir -p $OMNETPP_ROOT/ide/configuration/.settings/
RUN cp $GRADYS_SIMULATIONS_ROOT/docker_setup/org.eclipse.core.net.prefs $OMNETPP_ROOT/ide/configuration/.settings/
RUN cp $GRADYS_SIMULATIONS_ROOT/docker_setup/org.eclipse.ui.ide.prefs $OMNETPP_ROOT/ide/configuration/.settings/

RUN cp -a $GRADYS_SIMULATIONS_ROOT/docker_setup/.metadata/. $HOME/workspace/.metadata

USER $ROOT

RUN chown -R $USERNAME:$USERNAME $HOME

# =================================================================================================
# End configuration stage
# =================================================================================================

CMD ["LIBGL_ALWAYS_SOFTWARE=1 omnetpp"]
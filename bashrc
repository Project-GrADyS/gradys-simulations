# source omnetpp's setenv script on login
pushd $OMNETPP_ROOT > /dev/null && source setenv && popd > /dev/null

PS1="\[\e]0;\u@\h: \w\a\]${debian_chroot:+($debian_chroot)}\u@\h (omnetpp-dev):\w\$ "

export PYTHONPATH=$PYTHONPATH:.
export LD_LIBRARY_PATH=$OMNETPP_ROOT/lib
export LD_LIBRARY_PATH=/home/lac/.pyenv/versions/3.11.4/lib
export MESA_GL_VERSION_OVERRIDE=3.3
export LIBGL_ALWAYS_SOFTWARE=1
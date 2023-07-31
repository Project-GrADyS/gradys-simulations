#
# OMNeT++/OMNEST Makefile for gradys_simulations
#
# This file was generated with the command:
#  opp_makemake -f --deep -KINET4_5_PROJ=../../inet-4.5.0 -DINET_IMPORT -I/mavlink -I. -I/gradys_simulations -I$$\(INET4_5_PROJ\)/src -L$$\(INET4_5_PROJ\)/src -lpthread -lINET$$\(D\)
#

# Name of target to be created (-o option)
TARGET_DIR = .
TARGET_NAME = gradys_simulations$(D)
TARGET = $(TARGET_NAME)$(EXE_SUFFIX)
TARGET_IMPLIB = $(TARGET_NAME)$(IMPLIB_SUFFIX)
TARGET_IMPDEF = $(TARGET_NAME)$(IMPDEF_SUFFIX)
TARGET_FILES = $(TARGET_DIR)/$(TARGET)

# User interface (uncomment one) (-u option)
USERIF_LIBS = $(ALL_ENV_LIBS) # that is, $(QTENV_LIBS) $(CMDENV_LIBS)
#USERIF_LIBS = $(CMDENV_LIBS)
#USERIF_LIBS = $(QTENV_LIBS)

# C++ include paths (with -I)
INCLUDE_PATH = -I/mavlink -I. -I/gradys_simulations -I$(INET4_5_PROJ)/src

# Additional object and library files to link with
EXTRA_OBJS =

# Additional libraries (-L, -l options)
LIBS = $(LDFLAG_LIBPATH)$(INET4_5_PROJ)/src  -lpthread -lINET$(D)

# Output directory
PROJECT_OUTPUT_DIR = ../out
PROJECTRELATIVE_PATH = src
O = $(PROJECT_OUTPUT_DIR)/$(CONFIGNAME)/$(PROJECTRELATIVE_PATH)

# Object files for local .cc, .msg and .sm files
OBJS = \
    $O/gradys_simulations/MobileNode.o \
    $O/gradys_simulations/MobileSensorNode.o \
    $O/gradys_simulations/antenna/DipoleAntennaCustom.o \
    $O/gradys_simulations/applications/mamapp/MamDataCollectorApp.o \
    $O/gradys_simulations/applications/mamapp/MamNodeApp.o \
    $O/gradys_simulations/applications/mamapp/Md5.o \
    $O/gradys_simulations/communication/modules/UdpCommunicationApp.o \
    $O/gradys_simulations/linklayer/blemesh/BleMeshMac.o \
    $O/gradys_simulations/linklayer/blemesh/BleMeshProtocolDissector.o \
    $O/gradys_simulations/linklayer/blemesh/NetworkInterfaceExt.o \
    $O/gradys_simulations/mobility/DroneMobility.o \
    $O/gradys_simulations/mobility/DroneMobilityMav.o \
    $O/gradys_simulations/mobility/MAVLinkFileMobility.o \
    $O/gradys_simulations/mobility/MAVLinkRandomWaypointMobility.o \
    $O/gradys_simulations/mobility/base/MAVLinkMobilityBase.o \
    $O/gradys_simulations/mobility/failures/RandomFailureGenerator.o \
    $O/gradys_simulations/mobility/failures/SimpleConsumptionEnergy.o \
    $O/gradys_simulations/mobility/failures/SimpleTimerEnergy.o \
    $O/gradys_simulations/mobility/failures/base/FailureGeneratorBase.o \
    $O/gradys_simulations/pathloss/TwoRayNewModel.o \
    $O/gradys_simulations/protocols/auxiliary/CentralizedQLearning.o \
    $O/gradys_simulations/protocols/base/CommunicationProtocolBase.o \
    $O/gradys_simulations/protocols/ground/CentralizedQProtocolGround.o \
    $O/gradys_simulations/protocols/ground/DadcaProtocolGround.o \
    $O/gradys_simulations/protocols/ground/SimpleGroundProtocol.o \
    $O/gradys_simulations/protocols/mobile/CentralizedQProtocol.o \
    $O/gradys_simulations/protocols/mobile/DadcaProtocol.o \
    $O/gradys_simulations/protocols/mobile/SimpleDroneProtocol.o \
    $O/gradys_simulations/protocols/mobile/ZigzagProtocol.o \
    $O/gradys_simulations/protocols/sensor/CentralizedQProtocolSensor.o \
    $O/gradys_simulations/protocols/sensor/DadcaProtocolSensor.o \
    $O/gradys_simulations/protocols/sensor/SimpleSensorProtocol.o \
    $O/gradys_simulations/protocols/sensor/ZigzagProtocolSensor.o \
    $O/gradys_simulations/utils/TelemetryConditions.o \
    $O/gradys_simulations/utils/VehicleRoutines.o \
    $O/gradys_simulations/utils/subprocess/process.o \
    $O/gradys_simulations/utils/subprocess/process_unix.o \
    $O/gradys_simulations/utils/subprocess/process_win.o \
    $O/gradys_simulations/visualization/CustomOsgStatisticVisualizer.o \
    $O/gradys_simulations/applications/mamapp/BMeshPacket_m.o \
    $O/gradys_simulations/linklayer/blemesh/BleMeshMacHeader_m.o \
    $O/gradys_simulations/protocols/messages/internal/CommunicationCommand_m.o \
    $O/gradys_simulations/protocols/messages/internal/MobilityCommand_m.o \
    $O/gradys_simulations/protocols/messages/internal/Telemetry_m.o \
    $O/gradys_simulations/protocols/messages/network/CentralizedQMessage_m.o \
    $O/gradys_simulations/protocols/messages/network/DadcaMessage_m.o \
    $O/gradys_simulations/protocols/messages/network/SimpleMessage_m.o \
    $O/gradys_simulations/protocols/messages/network/ZigzagMessage_m.o

# Message files
MSGFILES = \
    gradys_simulations/applications/mamapp/BMeshPacket.msg \
    gradys_simulations/linklayer/blemesh/BleMeshMacHeader.msg \
    gradys_simulations/protocols/messages/internal/CommunicationCommand.msg \
    gradys_simulations/protocols/messages/internal/MobilityCommand.msg \
    gradys_simulations/protocols/messages/internal/Telemetry.msg \
    gradys_simulations/protocols/messages/network/CentralizedQMessage.msg \
    gradys_simulations/protocols/messages/network/DadcaMessage.msg \
    gradys_simulations/protocols/messages/network/SimpleMessage.msg \
    gradys_simulations/protocols/messages/network/ZigzagMessage.msg

# SM files
SMFILES =

# Other makefile variables (-K)
INET4_5_PROJ=../../inet-4.5.0

#------------------------------------------------------------------------------

# Pull in OMNeT++ configuration (Makefile.inc)

ifneq ("$(OMNETPP_CONFIGFILE)","")
CONFIGFILE = $(OMNETPP_CONFIGFILE)
else
CONFIGFILE = $(shell opp_configfilepath)
endif

ifeq ("$(wildcard $(CONFIGFILE))","")
$(error Config file '$(CONFIGFILE)' does not exist -- add the OMNeT++ bin directory to the path so that opp_configfilepath can be found, or set the OMNETPP_CONFIGFILE variable to point to Makefile.inc)
endif

include $(CONFIGFILE)

# Simulation kernel and user interface libraries
OMNETPP_LIBS = $(OPPMAIN_LIB) $(USERIF_LIBS) $(KERNEL_LIBS) $(SYS_LIBS)
ifneq ($(PLATFORM),win32)
LIBS += -Wl,-rpath,$(abspath $(INET4_5_PROJ)/src)
endif

COPTS = $(CFLAGS) $(IMPORT_DEFINES) -DINET_IMPORT $(INCLUDE_PATH) -I$(OMNETPP_INCL_DIR)
MSGCOPTS = $(INCLUDE_PATH)
SMCOPTS =

# we want to recompile everything if COPTS changes,
# so we store COPTS into $COPTS_FILE (if COPTS has changed since last build)
# and make the object files depend on it
COPTS_FILE = $O/.last-copts
ifneq ("$(COPTS)","$(shell cat $(COPTS_FILE) 2>/dev/null || echo '')")
  $(shell $(MKPATH) "$O")
  $(file >$(COPTS_FILE),$(COPTS))
endif

#------------------------------------------------------------------------------
# User-supplied makefile fragment(s)
-include makefrag

#------------------------------------------------------------------------------

# Main target
all: $(TARGET_FILES)

$(TARGET_DIR)/% :: $O/%
	@mkdir -p $(TARGET_DIR)
	$(Q)$(LN) $< $@
ifeq ($(TOOLCHAIN_NAME),clang-msabi)
	-$(Q)-$(LN) $(<:%.dll=%.lib) $(@:%.dll=%.lib) 2>/dev/null

$O/$(TARGET_NAME).pdb: $O/$(TARGET)
endif

$O/$(TARGET): $(OBJS)  $(wildcard $(EXTRA_OBJS)) Makefile $(CONFIGFILE)
	@$(MKPATH) $O
	@echo Creating executable: $@
	$(Q)$(CXX) $(LDFLAGS) -o $O/$(TARGET) $(OBJS) $(EXTRA_OBJS) $(AS_NEEDED_OFF) $(WHOLE_ARCHIVE_ON) $(LIBS) $(WHOLE_ARCHIVE_OFF) $(OMNETPP_LIBS)

.PHONY: all clean cleanall depend msgheaders smheaders

# disabling all implicit rules
.SUFFIXES :

$O/%.o: %.cc $(COPTS_FILE) | msgheaders smheaders
	@$(MKPATH) $(dir $@)
	$(qecho) "$<"
	$(Q)$(CXX) -c $(CXXFLAGS) $(COPTS) -o $@ $<

%_m.cc %_m.h: %.msg
	$(qecho) MSGC: $<
	$(Q)$(MSGC) -s _m.cc -MD -MP -MF $O/$(basename $<)_m.h.d $(MSGCOPTS) $?

%_sm.cc %_sm.h: %.sm
	$(qecho) SMC: $<
	$(Q)$(SMC) -c++ -suffix cc $(SMCOPTS) $?

msgheaders: $(MSGFILES:.msg=_m.h)

smheaders: $(SMFILES:.sm=_sm.h)

clean:
	$(qecho) Cleaning $(TARGET)
	$(Q)-rm -rf $O
	$(Q)-rm -f $(TARGET_FILES)
	$(Q)-rm -f $(call opp_rwildcard, . , *_m.cc *_m.h *_sm.cc *_sm.h)

cleanall:
	$(Q)$(CLEANALL_COMMAND)
	$(Q)-rm -rf $(PROJECT_OUTPUT_DIR)

help:
	@echo "$$HELP_SYNOPSYS"
	@echo "$$HELP_TARGETS"
	@echo "$$HELP_VARIABLES"
	@echo "$$HELP_EXAMPLES"

# include all dependencies
-include $(OBJS:%=%.d) $(MSGFILES:%.msg=$O/%_m.h.d)

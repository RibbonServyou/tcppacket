DBUG=FALSE
SRC_GENERIC=LibEvent.cpp TcpPacketImp.cpp tcpPacket.cpp

HIK_LINUX_LIB=../../../public/lib/x64/linux 

GENERIC_SOURCES=$(SRC_GENERIC)
GENERIC_OBJECTS=$(SRC_GENERIC:.cpp=.o)

KERNAL_VER=$(shell uname -r)
KERNAL_ID=$(shell uname -s)

#����Linuxƽ̨
ifeq ($(KERNAL_ID), Linux)

PLATFORM_SPEC=-DLINUX
SYS_USRLIB_DIR=/usr/lib
SYS_LIB_DIR=/lib
LIB_DIR=../../../public/LinuxSys32Lib/
OUTPUT_LIB_DIR=../Release/
BOOST_LIB=/usr/local/lib
BOOST_INCLUDE=/usr/local/include/
LIBEVENT_LIB=/opt/libevent/lib
LIBEVENT_INC=/opt/libevent/include
COMMON_INCLUDE=../../../public/include/

#����debug�汾
ifeq ($(DBUG),TRUE)
LIB_DIR=
endif
#����ifeq ($(DBUG),TRUE)

BUILD_DIR=./
RM =rm -rf 
CC=g++ -m64 -std=c++11
#-g -rdynamic
#LDFLAGS =-lc -lm  -L$(SYS_USRLIB_DIR) -L$(SYS_LIB_DIR) -L$(BOOST_LIB) -L$(LIBEVENT_LIB)
LDFLAGS =-lc -lm  -L$(SYS_LIB_DIR) -L$(BOOST_LIB) -L$(LIBEVENT_LIB)
OBJECT_EXTENSION=o
OBJECT_EXE=libTcpPacket.so
SPECIFIC_CFLAGS=-shared -fPIC

CFLAGS=-w -O2 -fstrength-reduce -finline-functions -ffast-math -DNDEBUG -I$(LIBEVENT_INC)  -I$(DY_INCLUDE) -I./ -I$(COMMON_INCLUDE)  -I/usr/include/nptl -I/usr/local/include/boost $(PLATFORM_SPEC)

CDFLAGS=-DDEBUG -Wall -g -fstrength-reduce -finline-functions -ffast-math -I/usr/include/nptl -I$(BOOST_INCLUDE)  $(COMMON_INCLUDE)
#����debug�汾
ifeq ($(DBUG),TRUE)
CFLAGS =$(CDFLAGS)
endif
#����ifeq ($(DBUG),TRUE)

SPECIFIC_LDFLAGS= -L$(SYS_USRLIB_DIR)/ntpl -L$(LIB_DIR) -ldl -lpthread  -levent -levent_pthreads -lboost_system -Wl,-rpath,./  -Wl,-rpath-link,$(LIB_DIR)
endif
#����ifeq ($(KERNAL_ID), Linux)

OBJECTS  = $(GENERIC_OBJECTS)
VPATH     = $(BUILD_DIR)
#-----------------------------------------------------------------------------
# The default rule
#-----------------------------------------------------------------------------
.SUFFIXES: .$(OBJECT_EXTENSION) .cpp
all: $(OBJECT_EXE)
	@echo
	@echo "---------------------------------------------------------------"
	@echo " $(SHARED_LIB) has been successfully built."
	@echo
	@echo " * Binaries are currently located in the '$(OUTPUT_LIB_DIR)' directory"
	@echo "---------------------------------------------------------------"
	@echo "$(BOOST_LIB) "
	@echo "$(GENERIC_SOURCES)"
	@echo

$(OBJECTS): $(GENERIC_SOURCES) 

.cpp.$(OBJECT_EXTENSION):
	@echo "  C: $(@D)/$(<F)"
	@$(CC) -c $(SPECIFIC_CFLAGS) $(CFLAGS) $< -o $(BUILD_DIR)$@

$(OBJECT_EXE): $(BUILD_DIR)$(OBJECTS)
	@echo "	 All object file: $(OBJECTS)"
	@echo "  L: $(@F)"
	@$(CC) $(SPECIFIC_CFLAGS) $(OBJECTS) $(LDFLAGS) $(SPECIFIC_LDFLAGS) -o $(OUTPUT_LIB_DIR)$(OBJECT_EXE)
	
.PHONY: clean

clean:
	@echo "  Clean: object files"
	@find $(BUILD_DIR) -name "*.o" -exec $(RM) {} \;

copy:
	@cp $(OUTPUT_LIB_DIR)/*.so $(BOOST_LIB)
	@cp $(OUTPUT_LIB_DIR)/*.so $(HIK_LINUX_LIB) 




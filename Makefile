

INCLUDEDIRS=

LIBS=  

VPATH = 

DEBUG=-g


TOPDIR=${PWD}


ifdef DISABLE_READLINE
DEFINES+=DISABLE_READLINE
endif


#set CPP
ifdef  CXX   
	CPP=$(CXX)
else
	CPP=g++
DEFINES=PCBUILD
endif




APPVERSION:= $(shell scripts/gversion.sh)


SOSUBREV=0


#Clean/dirty state

PROGRAM=farm

C_SOURCES=

CPP_SOURCES= main.cpp  

C_OBJECTS=$(patsubst %.c,%.o, $(C_SOURCES))

CPP_OBJECTS=$(patsubst %.cpp,%.o, $(CPP_SOURCES))

CFLAGS=$(DEBUG)  -MMD $(INCPATH)  $(DEFINE_ARGS) -lpthread

CPPFLAGS=$(DEBUG) -MMD $(INCPATH) -Wno-pmf-conversions -std=c++11 $(DEFINE_ARGS) -lpthread



C_DEPS=$(C_SOURCES:.c=.d)

CPP_DEPS=$(CPP_SOURCES:.cpp=.d)

OBJECTS=$(C_OBJECTS) $(CPP_OBJECTS)

DEPS=$(C_DEPS) $(CPP_DEPS)


INCLUDEDIR_ARGS=$(patsubst %,-I%, $(INCLUDEDIRS))


%.o:%.c
	$(CC) $(CFLAGS) -c $(INCLUDEDIR_ARGS) $(DEFINE_ARGS) -o $@  $<

%.o:%.cpp
	$(CPP) $(CPPFLAGS) -c $(INCLUDEDIR_ARGS) $(DEFINE_ARGS) -o $@  $<

all: $(PROGRAM) 

$(PROGRAM):$(OBJECTS) 
	$(CPP) $(CPPFLAGS) -o $(PROGRAM)  $(OBJECTS)  $(LIBS) 


version.h: 
	@echo "const char *appversionstring=\"$(APPVERSION)\";" >$@


test:
	@echo $(OBJECTS)
	@echo $(CPP)

dox doc docs:
	@doxygen

clean: genfile_clean
	rm -f $(OBJECTS) $(PROGRAM) $(DEPS) version.h
	
genfile_clean:
	cd $(MESSAGEDIR) ; rm -f $(GENCPPFILES) $(GENHEADERFILES) $(GENERATED_FILES)
	rm -rf html 



-include $(DEPS)

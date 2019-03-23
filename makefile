## -*- Makefile -*-
##
## User: Matthew
## Time: 12-Jan-2019 09:16:44
##
##


#### Compiler and tool definitions shared by all build targets #####
CC = gcc
BASICOPTS = -g -Wall
CFLAGS = $(BASICOPTS)


# Define the target directories.
TARGETDIR_ALL=build


all:    $(TARGETDIR_ALL)/weatherReceiver 
	
# ----------------------------------------------------------------------------------------------------------------
#       Common Objects
# ----------------------------------------------------------------------------------------------------------------
OBJS_common = \
	$(TARGETDIR_ALL)/ioctl_spi_comms.o \
	$(TARGETDIR_ALL)/gpio_control.o \

# ----------------------------------------------------------------------------------------------------------------
##      Target: testProgram
# ----------------------------------------------------------------------------------------------------------------
OBJS_weatherReceiver =  \
	$(OBJS_common) \
	$(TARGETDIR_ALL)/weatherReceiver.o
USERLIBS_weatherReceiver =  
DEPLIBS_weatherReceiver =  
LDLIBS_weatherReceiver = $(USERLIBS_weatherReceiver)



# ----------------------------------------------------------------------------------------------------------------
#       Link or archive
# ----------------------------------------------------------------------------------------------------------------
$(TARGETDIR_ALL)/weatherReceiver: $(TARGETDIR_ALL) $(OBJS_weatherReceiver) $(DEPLIBS_weatherReceiver)
	$(LINK.c) $(CFLAGS)  -o $@ $(OBJS_weatherReceiver)  $(LDLIBS_weatherReceiver)
		
# ----------------------------------------------------------------------------------------------------------------
#       Compile source files into .o files
# ----------------------------------------------------------------------------------------------------------------
$(TARGETDIR_ALL)/gpio_control.o: $(TARGETDIR_ALL) Receiver/src/gpio_control.c
	$(COMPILE.c) $(CFLAGS)  -o $@ Receiver/src/gpio_control.c

$(TARGETDIR_ALL)/weatherReceiver.o: $(TARGETDIR_ALL) Receiver/src/weatherReceiver.c
	$(COMPILE.c) $(CFLAGS)  -o $@ Receiver/src/weatherReceiver.c

$(TARGETDIR_ALL)/ioctl_spi_comms.o: $(TARGETDIR_ALL) Receiver/src/ioctl_spi_comms.c
	$(COMPILE.c) $(CFLAGS)  -o $@ Receiver/src/ioctl_spi_comms.c


# ----------------------------------------------------------------------------------------------------------------
#    Clean target deletes all generated files ####
# ----------------------------------------------------------------------------------------------------------------
clean:
	rm -f	$(OBJS_weatherReceiver) \
		$(TARGETDIR_ALL)/weatherReceiver \


# Create the target directory (if needed)
$(TARGETDIR_ALL):
	mkdir -p $(TARGETDIR_ALL)


# Enable dependency checking
.KEEP_STATE:
.KEEP_STATE_FILE:.make.state.GNU-x86_64-Linux


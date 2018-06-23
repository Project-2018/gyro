

# Required src directories
ROLLSENSSRC = $(SUBMODULE)/gyro/rollsensor.c \
	          $(SUBMODULE)/gyro/rollsenlp.c \
			  $(CHIBIOS)/os/various/devices_lib/accel/lis302dl.c

# Required include directories
ROLLSENSINC = $(SUBMODULE)/gyro


include $(CHIBIOS)/os/ex/ST/lis3dsh.mk

# Required src directories
ROLLSENSSRC = $(SUBMODULE)/gyro/rollsensor.c \
	          $(SUBMODULE)/gyro/rollsenlp.c \
	          $(LIS3DSHSRC)

# Required include directories
ROLLSENSINC = $(SUBMODULE)/gyro \
			  $(LIS3DSHINC)

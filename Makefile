CC := gcc
MEX := /usr/local/MATLAB/R2014a/bin/mex
CFLAGS := -Wall -O3 -g -fPIC -std=c99
LDFLAGS += /usr/lib/x86_64-linux-gnu/libOpenCL.so
#LDFLAGS += /usr/lib/libOpenCL.so
LDLIBS += -lm

#------------------------------------------------------------------------------
# Define OBJECTS.
#------------------------------------------------------------------------------
OBJS += $(patsubst %.c,%.o,$(wildcard lib/*.c))

#------------------------------------------------------------------------------
# Define MEX EXECUTABLES
#------------------------------------------------------------------------------
MEX += $(wildcard mex/*.c)
#------------------------------------------------------------------------------

.PHONY: clean realclean

all: objects mex

#------------------------------------------------------------------------------
# object targets
#------------------------------------------------------------------------------
objects: $(OBJS)

$(OBJS): %.o: %.c
	@echo Building $@
	@$(CC) -MT $@ -MM $< > $(<:%.c=%.d)
	@$(CC) $(CFLAGS) -c $< -o $@

-include $(patsubst %.o,%.d,$(OBJS))

#------------------------------------------------------------------------------
# MEX files
#------------------------------------------------------------------------------
mex: $(MEX)

$(MEX): $(OBJS)
	@echo Building MEX-Executable $@
	@echo ""
	@$(MEX) CFLAGS='$$CFLAGS $(CFLAGS)' -output bin/$(notdir $@) $(LDFLAGS) $@ \
		$(OBJS) $(LDLIBS)
	@echo ""

#------------------------------------------------------------------------------

clean:
	@rm -f lib/*.{o,d}

realclean:
	@rm -f lib/*.{o,d}

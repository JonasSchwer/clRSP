CC := gcc
CFLAGS := -Wall -O3 -g -fPIC -std=c99
LDLIBS += -lOpenCL
LDLIBS += -lm

#------------------------------------------------------------------------------
# Define OBJECTS.
#------------------------------------------------------------------------------
OBJS += $(patsubst %.c,%.o,$(wildcard src/*.c))

#------------------------------------------------------------------------------
# Define executables for EXAMPLES.
#------------------------------------------------------------------------------
EXAMPLES += $(patsubst %.c,%,$(wildcard examples/*.c))

#------------------------------------------------------------------------------
# Define executables for BENCHMARKS.
#------------------------------------------------------------------------------
BENCHMARKS += $(patsubst %.c,%,$(wildcard benchmarks/*.c))

#------------------------------------------------------------------------------
# Define MEX EXECUTABLES
#------------------------------------------------------------------------------
MEX += $(wildcard mex/*.c)
#------------------------------------------------------------------------------

.PHONY: clean realclean

all: objects examples

#------------------------------------------------------------------------------
# example executable targets
#------------------------------------------------------------------------------
examples: $(EXAMPLES) $(OBJS)

$(EXAMPLES): %: %.c
	@echo Building $@
	@$(CC) $(CFLAGS) $(LDFLAGS) $< -o bin/$(notdir $@) \
		$(OBJS) $(LDLIBS)

#------------------------------------------------------------------------------
# benchmark executable targets
#------------------------------------------------------------------------------
benchmarks: objects $(BENCHMARKS) $(OBJS)

$(BENCHMARKS): %: %.c
	@echo Building $@
	@$(CC) $(CFLAGS) $(LDFLAGS) $< -o bin/$(notdir $@) \
		$(OBJS) $(LDLIBS)

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
	@echo $(MEX)

$(MEX): $(NON_DEP_OBJS)

$(MEX):
	@echo Building MEX-Executable $@
	@echo ""
	@mex CFLAGS='$$CFLAGS $(CFLAGS) -DAMD' \
		-output bin_mex/$(notdir $@) $@ $(AUX_OBJS) $(OCL_OBJS) $(STOR_OBJS) \
		$(AMD_OBJS) $(LDLIBS) $(CLFFTLIB)
	@echo ""
#------------------------------------------------------------------------------

clean:
	@rm -f src/*.{o,d}

realclean:
	@rm -f bin/*
	@rm -f src/*.{o,d}

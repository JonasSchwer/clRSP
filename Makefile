CC := gcc
CFLAGS := -Wall -O3 -g -fPIC -std=c99
LDLIBS += -lOpenCL
LDLIBS += -lm
LDLIBS += -lclFFT

#------------------------------------------------------------------------------
# Define OBJECTS.
#------------------------------------------------------------------------------
OBJS += $(patsubst %.c,%.o,$(wildcard lib/*.c))

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

all: objects mex

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

$(MEX): $(OBJS)
	@echo Building MEX-Executable $@
	@echo ""
	@mex CFLAGS='$$CFLAGS $(CFLAGS)' -output bin/$(notdir $@) $@ $(OBJS) \
		$(LDLIBS)
	@echo ""

#------------------------------------------------------------------------------

clean:
	@rm -f lib/*.{o,d}

realclean:
	@rm -f lib/*.{o,d}

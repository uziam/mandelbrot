# Comment the following line to print compilation commands
Q        ?= @

EXEC     ?= mandelbrot

SRC      ?=
SRC      += main.c
SRC      += mandelbrot.c
SRC      += render.c

SRC_H    ?=
SRC_H    += mandelbrot.h
SRC_H    += render.h
SRC_H    += setting.h

CC       ?= gcc

CC_FLAGS ?=
CC_FLAGS += -std=gnu99
CC_FLAGS += -Wall
CC_FLAGS += -O2
CC_FLAGS += -fopenmp
CC_FLAGS += -mavx2
CC_FLAGS += -march=native
CC_FLAGS += $(shell sdl2-config --cflags)

LD_FLAGS ?=
LD_FLAGS += $(shell sdl2-config --libs)

OBJ      := $(SRC:.c=.o)

.PHONY: all
all: $(EXEC)

$(EXEC): $(OBJ)
	@echo "    LD   $@"
	$(Q) $(CC) $(CC_FLAGS) $(LD_FLAGS) $^ -o $@

%.o: %.c
	@echo "    CC   $@"
	$(Q) $(CC) -c $(CC_FLAGS) $< -o $@

.PHONY: clean
CLEAN_FILES ?=
CLEAN_FILES += $(EXEC)
CLEAN_FILES += $(OBJ)

CLEAN_TARG  := $(patsubst %,clean_%,$(CLEAN_FILES))

clean: $(CLEAN_TARG)

clean_%:
	@echo "    RM   $*"
	$(Q) rm -f $*

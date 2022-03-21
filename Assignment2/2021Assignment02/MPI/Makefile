CC = mpicc
OPTFLAGS = -O3 -march=native
override CFLAGS += -Wall -Wextra -fopenmp -D DOUBLE_PRECISION
LDFLAGS = -lm 

SRC = kdtree_mpi.c
EXE = $(SRC:.c=.x)

.PHONY:	all default clean

all: 		$(EXE)

default:	$(EXE)

%.x: %.c
	$(CC) $< -o $@ $(CFLAGS) $(OPTFLAGS) $(LDFLAGS)

clean:
	rm -f $(EXE)

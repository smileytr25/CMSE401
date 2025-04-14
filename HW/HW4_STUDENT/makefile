all: revGOL gol

# Compiler settings
GCC := gcc
MPICC := mpicc
EXT := c
FLAGS := -lm -lpng

# Object files
reverseGOL.o: reverseGOL.$(EXT)
	$(GCC) $(FLAGS) -c reverseGOL.$(EXT)

png_util.o: png_util.c
	$(GCC) $(FLAGS) -c png_util.c

revGOL: reverseGOL.o png_util.o
	$(GCC) $(FLAGS) -o revGOL reverseGOL.o png_util.o

gameoflife.o: gameoflife.$(EXT)
	$(MPICC) $(FLAGS) -c gameoflife.$(EXT)

gol: gameoflife.o png_util.o
	$(MPICC) $(FLAGS) -o gol gameoflife.o png_util.o

test: revGOL cmse2.txt
	./revGOL cmse2.txt

clean:
	rm -f *.o
	rm -f gol revGOL


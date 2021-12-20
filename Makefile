EXEC = D_recompile

CC = gcc

default:
	$(CC) -o drecompile D_recompile.c

dynamic:
	$(CC) -o drecompile D_recompile.c -DRECOMPILE

clean:
	rm -rf D_recompile $(EXEC)

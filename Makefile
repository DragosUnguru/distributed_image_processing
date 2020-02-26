CC=mpicc
EXE=tema3

# Filters:	blur smooth sharpen mean emboss

build: tema3.c utils.c
	$(CC) $^ -lm -o $(EXE)

run1: $(EXE)
	time -p mpirun -np 1 $(EXE) ./in/PNM/landscape.pnm ./out/PNM/landscape.pnm blur smooth mean sharpen emboss blur smooth mean sharpen emboss blur smooth mean sharpen emboss blur smooth mean sharpen emboss blur smooth mean sharpen emboss blur smooth mean sharpen emboss blur smooth mean sharpen emboss blur smooth mean sharpen emboss blur smooth mean sharpen emboss blur smooth mean sharpen emboss blur smooth mean sharpen emboss blur smooth mean sharpen emboss blur smooth mean sharpen emboss blur smooth mean sharpen emboss blur smooth mean sharpen emboss blur smooth mean sharpen emboss blur smooth mean sharpen emboss blur smooth mean sharpen emboss blur smooth mean sharpen emboss blur smooth mean sharpen emboss blur smooth mean sharpen emboss blur smooth mean sharpen emboss blur smooth mean sharpen emboss blur smooth mean sharpen emboss blur smooth mean sharpen emboss

run2: $(EXE)
	@time -p mpirun -np 2 $(EXE) ./in/PNM/landscape.pnm ./out/PNM/landscape.pnm blur smooth mean sharpen emboss blur smooth mean sharpen emboss blur smooth mean sharpen emboss blur smooth mean sharpen emboss blur smooth mean sharpen emboss blur smooth mean sharpen emboss blur smooth mean sharpen emboss blur smooth mean sharpen emboss blur smooth mean sharpen emboss blur smooth mean sharpen emboss blur smooth mean sharpen emboss blur smooth mean sharpen emboss blur smooth mean sharpen emboss blur smooth mean sharpen emboss blur smooth mean sharpen emboss blur smooth mean sharpen emboss blur smooth mean sharpen emboss blur smooth mean sharpen emboss blur smooth mean sharpen emboss blur smooth mean sharpen emboss blur smooth mean sharpen emboss blur smooth mean sharpen emboss blur smooth mean sharpen emboss blur smooth mean sharpen emboss blur smooth mean sharpen emboss

run3: $(EXE)
	@time -p mpirun -np 3 $(EXE) ./in/PNM/landscape.pnm ./out/PNM/landscape.pnm blur smooth sharpen emboss mean blur smooth sharpen emboss mean
	
run4: $(EXE)
	@time -p mpirun -np 4 $(EXE) ./in/PNM/landscape.pnm ./out/PNM/landscape.pnm blur smooth sharpen emboss mean blur smooth sharpen emboss mean

run5: $(EXE)
	@time -p mpirun -np 5 $(EXE) ./in/PNM/landscape.pnm ./out/PNM/landscape.pnm blur smooth sharpen emboss mean blur smooth sharpen emboss mean

run6: $(EXE)
	@time -p mpirun -np 6 $(EXE) ./in/PNM/landscape.pnm ./out/PNM/landscape.pnm blur smooth sharpen emboss mean blur smooth sharpen emboss mean
	
run7: $(EXE)
	@time -p mpirun -np 7 $(EXE) ./in/PNM/landscape.pnm ./out/PNM/landscape.pnm blur smooth sharpen emboss mean blur smooth sharpen emboss mean

run8: $(EXE)
	@time -p mpirun -np 8 $(EXE) ./in/PNM/landscape.pnm ./out/PNM/landscape.pnm blur smooth sharpen emboss mean blur smooth sharpen emboss mean

clean:
	rm -f $(EXE)

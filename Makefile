FLAG=-O3

all: clean gpu

gpu: gpu.c sum.cl
	$(CC) $(FLAG) $< -o $@ -lOpenCL

clean:
	rm -f gpu


FLAG=-O3

all: gpu

gpu: gpu.c mul.cl
	$(CC) $(FLAG) $< -o $@ -lOpenCL

clean:
	rm -f gpu


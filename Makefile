FLAG=-O0 -g

all: gpu

gpu: gpu.c mul.cl
	$(CC) $(FLAG) $< -o $@ -lOpenCL

clean:
	rm -f gpu

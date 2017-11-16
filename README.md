# OpenCL Performance Test

This is a simple test to compare the performance between CPU and GPU computation.

There are two program `cpu.c` and `gpu.c`, both of them will calculate the summary of numbers from `0` to `100 million`. In the end, the program will display the elapsed time.

## How to run

First, you need to compile the programs. This method only works on Apple platform (macOS) for now.

Type the following command to your terminal:

```
make
```

When the compiling finished, you will have 2 binary files: `cpu` and `gpu`.

Run test on GPU:

```
./gpu
```

# Why fork

Original program seem to execute in only one thread: Not parallel.
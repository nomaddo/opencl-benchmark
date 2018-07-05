#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <assert.h>

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#define MAX_SOURCE_SIZE (0x100000)


double gettime()
{
  struct timeval t;
  gettimeofday(&t, NULL);
  return t.tv_sec + (double)t.tv_usec * 1e-6;
}

int main(int argc, char * argv[]) {
  if (! (argc == 5)) {
    printf ("usage: %s filename name arg_num data_num\n", argv[0]);
    exit(0);
  }
  char * fileName = argv[1];
  char * kernel_name = argv[2];
  int arg_num = atoi(argv[3]);
  int N = atoi (argv[4]);

  cl_device_id device_id = NULL;
  cl_context context = NULL;
  cl_command_queue command_queue = NULL;
  cl_program program = NULL;
  cl_kernel kernel = NULL;
  cl_platform_id platform_id = NULL;
  cl_uint ret_num_devices;
  cl_uint ret_num_platforms;
  cl_int ret;

  FILE *fp;

  char *source_str;
  size_t source_size;



  fp = fopen(fileName, "r");

  if (!fp) {
    fprintf(stderr, "Failed to load kernel\n");
    exit(1);
  }

  source_str = (char*)malloc(MAX_SOURCE_SIZE);
  source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
  fclose(fp);

  ret = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &device_id, &ret_num_devices);

  context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);
  command_queue = clCreateCommandQueue(context, device_id, 0, &ret);

  program = clCreateProgramWithSource(context, 1, (const char **)&source_str, (const size_t *)&source_size, &ret);
  assert (ret == CL_SUCCESS);

  ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
  kernel = clCreateKernel(program, kernel_name, &ret);

  float * val = malloc (sizeof(float) * N);

  for (int i = 0; i < N; i++) {
    val[i] = (float)i;
  }

  /* arguments */
  cl_mem args[arg_num];
  for (int i = 0; i < arg_num; i++) {
    args[i] = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(cl_float) * N, NULL, &ret);
    clEnqueueWriteBuffer(command_queue, args[i], CL_TRUE, 0, sizeof(cl_float) * N, val, 0, NULL, NULL);
    assert(ret == CL_SUCCESS);
    ret = clSetKernelArg (kernel, i, sizeof(cl_mem), (void*) &(args[i]));
    assert(ret == CL_SUCCESS);
  }

  size_t local;
  ret = clGetKernelWorkGroupInfo(kernel, device_id, CL_KERNEL_WORK_GROUP_SIZE, sizeof(local), &local, NULL);

  double begin = gettime();

  size_t global_item_size[] = { N };
  ret = clEnqueueNDRangeKernel (command_queue, kernel, 1, NULL,
				&global_item_size, NULL, 0, NULL, NULL);
  ret = clFinish (command_queue);
  if (! (ret == CL_SUCCESS)) {
    printf ("error code: %d\n", ret);
    assert (0);
  }

  double end = gettime();

  ret = clEnqueueReadBuffer(command_queue, args[0], CL_TRUE, 0, sizeof(cl_float) * N, val, 0, NULL, NULL);
  assert (ret == CL_SUCCESS);

  for (int i = 0; i < N; i++) {
    printf ("%lf\n", val[i]);
  }

  ret = clFlush(command_queue);
  assert (ret == CL_SUCCESS);

  double runtime = (end - begin);

  ret = clReleaseKernel(kernel);
  ret = clReleaseProgram(program);

  for (int i = 0; i < arg_num; i++) {
    ret = clReleaseMemObject(args[i]);
  }

  ret = clReleaseCommandQueue(command_queue);
  ret = clReleaseContext(context);

  printf("%lf\n", runtime);
  printf("%lf BGPS\n", N * sizeof(float) * 8 / runtime * 1e-9);
  free(source_str);
  return 0;
}

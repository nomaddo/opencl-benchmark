#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#define MAX_SOURCE_SIZE (0x100000)

#define N 650 * 1000 * 10

int main() {
  cl_device_id device_id = NULL;
  cl_context context = NULL;
  cl_command_queue command_queue = NULL;
  cl_mem memobj = NULL;
  cl_program program = NULL;
  cl_kernel kernel = NULL;
  cl_platform_id platform_id = NULL;
  cl_uint ret_num_devices;
  cl_uint ret_num_platforms;
  cl_int ret;

  cl_float * val;

  FILE *fp;
  char fileName[] = "./mul.cl";
  char *source_str;
  size_t source_size;

  val = malloc(sizeof(cl_float) * N);
  
  fp = fopen(fileName, "r");
  if (!fp) {
    fprintf(stderr, "Failed to load kernel\n");
    exit(1);
  }
  source_str = (char*)malloc(MAX_SOURCE_SIZE);
  source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
  fclose(fp);

  ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
  ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_DEFAULT, 1, &device_id, &ret_num_devices);

  context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);
  command_queue = clCreateCommandQueue(context, device_id, 0, &ret);

  for (int i = 0; i < N; i++)
    val[i] = 990.0;
  
  memobj = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(cl_float) * N, NULL, &ret);
  clEnqueueWriteBuffer(command_queue, memobj, CL_TRUE, 0, sizeof(cl_float) * N, val, 0, NULL, NULL);
  
  program = clCreateProgramWithSource(context, 1, (const char **)&source_str, (const size_t *)&source_size, &ret);
  ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
  kernel = clCreateKernel(program, "hello", &ret);

  ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void*)&memobj);

  clock_t begin = clock();

  size_t global_item_size = N / 100;
  size_t local_item_size = 1;
  ret = clEnqueueNDRangeKernel (command_queue, kernel, 1, NULL,
				&global_item_size, &local_item_size, 0, NULL, NULL);
  assert (ret == CL_SUCCESS);

  ret = clEnqueueReadBuffer(command_queue, memobj, CL_TRUE, 0, sizeof(cl_mem), val, 0, NULL, NULL);
  assert (ret == CL_SUCCESS);
  
  ret = clFlush(command_queue);
  assert (ret == CL_SUCCESS);
  
  ret = clFinish(command_queue);
  assert (ret == CL_SUCCESS);
  
  clock_t end = clock();
  double runtime = (double)(end - begin) / CLOCKS_PER_SEC;
  
  ret = clReleaseKernel(kernel);
  ret = clReleaseProgram(program);
  ret = clReleaseMemObject(memobj);
  ret = clReleaseCommandQueue(command_queue);
  ret = clReleaseContext(context);

  printf("Result: %lf\n", val[0]);
  printf("Runtime: %lfms\n", runtime);
  
  free(source_str);

  return 0;
}

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

int main(int argc, char * argv[]) {
  if (! argc == 5) {
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
  printf ("context %d\n", ret);
  command_queue = clCreateCommandQueue(context, device_id, 0, &ret);
  printf ("command queue %d\n", ret);

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
    printf ("%d\n", ret);
    assert(ret == CL_SUCCESS);
  }
  
  printf ("compile successful!\n");

  size_t local;
  ret = clGetKernelWorkGroupInfo(kernel, device_id, CL_KERNEL_WORK_GROUP_SIZE, sizeof(local), &local, NULL);

  clock_t begin = clock();

  size_t global_item_size[] = { N };
  ret = clEnqueueNDRangeKernel (command_queue, kernel, 1, NULL,
				&global_item_size, NULL, 0, NULL, NULL);
  printf ("NDrange %d\n", ret);
  ret = clFinish (command_queue);
  if (! (ret == CL_SUCCESS)) {
    printf ("error code: %d\n", ret);
    assert (0);
  }
  
  ret = clEnqueueReadBuffer(command_queue, args[0], CL_TRUE, 0, sizeof(cl_float) * N, val, 0, NULL, NULL);
  assert (ret == CL_SUCCESS);

  for (int i = 0; i < N; i++) {
    printf ("%lf\n", val[i]);
  }

  ret = clFlush(command_queue);
  assert (ret == CL_SUCCESS);

  clock_t end = clock();
  double runtime = (double)(end - begin) / CLOCKS_PER_SEC;


  ret = clReleaseKernel(kernel);
  ret = clReleaseProgram(program);

  for (int i = 0; i < arg_num; i++) {
    ret = clReleaseMemObject(args[i]);
  }
  
  ret = clReleaseCommandQueue(command_queue);
  ret = clReleaseContext(context);

  printf("Runtime: %lfms\n", runtime);

  free(source_str);

  printf("finished!\n");
  fflush(stdout);
  return 0;
}

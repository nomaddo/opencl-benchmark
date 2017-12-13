kernel void hello(global float * x, global float * y) {

  int ind = get_global_id(0);
  float16 v = vload16 (ind, x);
  v *= 2;
  vstore16 (v, ind, x);

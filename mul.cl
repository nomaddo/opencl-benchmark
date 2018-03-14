kernel void hello(global float * x){
  int ind = get_global_id(0);
  x[ind] = x[ind] * 2;
}

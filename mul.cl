kernel void hello(global float * val) {
  size_t i = get_global_id(0);
  for (int j = 0; j < 100; j++) {
    val[i * 100 + j] *= 2;
  }
}

#include <hip/hip_runtime.h>

extern "C" __global__ void vec_add(
	int* __restrict__ A, 
	int* __restrict__ B
){
	int index = threadIdx.x; // 0~3
	B[index] += A[index];
}

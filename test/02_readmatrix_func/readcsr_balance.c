// 行で分割したローカルな行列を、その要素数分のメモリ確保　2回ファイルの読み取りを行う必要あり

// mpicc -O3 test/readmatrix_func/readcsr_balance.c test/readmatrix_func/matrix.c test/readmatrix_func/mmio.c -I test/readmatrix_func
// mpirun -np 4 ./a.out data/small_test.mtx

#include <mpi.h>
#include <math.h>
#include <time.h> 

#include "matrix.h"

int main(int argc, char *argv[]) {
    
    MPI_Init(&argc, &argv);

    int numprocs, myid, namelen;
    char proc_name[MPI_MAX_PROCESSOR_NAME];
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);
    MPI_Get_processor_name(proc_name, &namelen);

    char *filename = argv[1];

    INFO_Matrix A_info;

	CSR_Matrix *A_loc = (CSR_Matrix *)malloc(sizeof(CSR_Matrix));
	csr_init_matrix(A_loc);

    MPI_csr_load_matrix(filename, A_loc, &A_info);

	printf("proc %d, rows = %d, nz = %d, last ptr = %d, A.nz = %d, A.rows = %d, A.cols = %d, A.code = %.4s\n", 
            myid, A_loc->rows, A_loc->nz, A_loc->ptr[A_loc->rows], A_info.nz, A_info.rows, A_info.cols, A_info.code);

	csr_free_matrix(A_loc); free(A_loc);

	MPI_Finalize();
	return 0;
}

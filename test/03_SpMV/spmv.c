// 行で分割したローカルな行列を、その要素数分のメモリ確保　2回ファイルの読み取りを行う必要あり

// mpicc -O3 test/03_SpMV/spmv.c test/03_SpMV/matrix.c test/03_SpMV/mmio.c -I test/03_SpMV
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

    A_info.recvcounts = (int *)malloc(numprocs * sizeof(int));
    A_info.displs = (int *)malloc(numprocs * sizeof(int));

	CSR_Matrix *A_loc = (CSR_Matrix *)malloc(sizeof(CSR_Matrix));
	csr_init_matrix(A_loc);

    MPI_csr_load_matrix(filename, A_loc, &A_info);

    double *x_loc, *y_loc, *x, *y;
    x_loc = (double *)malloc(A_loc->rows * sizeof(double));
    y_loc = (double *)malloc(A_loc->rows * sizeof(double));
    x = (double *)malloc(A_info.cols * sizeof(double));
    y = (double *)malloc(A_info.rows * sizeof(double));

    for (int i = 0; i < A_info.cols; i++) {
        x[i] = 1;
    }

    csr_mv(A_loc, x, y_loc);

    //MPI_Gatherv(y_loc, A_loc->rows, MPI_DOUBLE, y, recvcounts, displs, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    //MPI_Allgatherv(y_loc, A_loc->rows, MPI_DOUBLE, y, recvcounts, displs, MPI_DOUBLE, MPI_COMM_WORLD);

    MPI_Request request;
    MPI_Iallgatherv(y_loc, A_loc->rows, MPI_DOUBLE, y, A_info.recvcounts, A_info.displs, MPI_DOUBLE, MPI_COMM_WORLD, &request);
    MPI_Wait(&request, MPI_STATUS_IGNORE);

    for (int proc = 0; proc < numprocs; proc++) {
        if (myid == proc) {
            printf("proc %d, rows = %d, nz = %d, last ptr = %d, A.nz = %d, A.rows = %d, A.cols = %d, A.code = %.4s\n", 
                    myid, A_loc->rows, A_loc->nz, A_loc->ptr[A_loc->rows], A_info.nz, A_info.rows, A_info.cols, A_info.code);

            for (int i = 0; i < A_loc->rows; i++) {
                printf("%f ", y_loc[i]);
            } printf("\n");

            int start_idx = A_info.displs[myid];
            int num_elements = A_info.recvcounts[myid];
            for (int i = 0; i < num_elements; i++) {
                y_loc[i] = y[start_idx + i];
            }

            for (int i = 0; i < A_loc->rows; i++) {
                printf("%f ", y_loc[i]);
            } printf("\n");
            for (int i = 0; i < A_info.rows; i++) {
                printf("%f ", y[i]);
            } printf("\n");
        }
    }

	csr_free_matrix(A_loc); free(A_loc);
    free(x_loc); free(y_loc); free(x); free(y);
    free(A_info.recvcounts);  free(A_info.displs);

	MPI_Finalize();
	return 0;
}

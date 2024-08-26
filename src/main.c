// mpicc -O3 src/main.c src/matrix.c src/vector.c src/mmio.c -I src
// mpirun -np 4 ./a.out data/epb3.mtx

#include <mpi.h>
#include <math.h>
#include <time.h> 

#include "matrix.h"
#include "vector.h"

#define RANDOM

int main(int argc, char *argv[]) {
    
    MPI_Init(&argc, &argv);

    int numprocs, myid, namelen;
    char proc_name[MPI_MAX_PROCESSOR_NAME];
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);
    MPI_Get_processor_name(proc_name, &namelen);

    char *all_proc_names = NULL;
    if (myid == 0) {
        all_proc_names = (char *)malloc(numprocs * MPI_MAX_PROCESSOR_NAME * sizeof(char));
    }
    MPI_Gather(proc_name, MPI_MAX_PROCESSOR_NAME, MPI_CHAR, all_proc_names, MPI_MAX_PROCESSOR_NAME, MPI_CHAR, 0, MPI_COMM_WORLD);

    if (myid == 0) {
        int *proc_count_per_node = (int *)calloc(numprocs, sizeof(int));
        int unique_nodes = 0;

        for (int i = 0; i < numprocs; i++) {
            int found = 0;
            for (int j = 0; j < unique_nodes; j++) {
                if (strncmp(&all_proc_names[i * MPI_MAX_PROCESSOR_NAME], 
                            &all_proc_names[j * MPI_MAX_PROCESSOR_NAME], MPI_MAX_PROCESSOR_NAME) == 0) {
                    proc_count_per_node[j]++;
                    found = 1;
                    break;
                }
            }
            if (!found) {
                strncpy(&all_proc_names[unique_nodes * MPI_MAX_PROCESSOR_NAME], 
                        &all_proc_names[i * MPI_MAX_PROCESSOR_NAME], MPI_MAX_PROCESSOR_NAME);
                proc_count_per_node[unique_nodes]++;
                unique_nodes++;
            }
        }

        printf("Node: %d\n", unique_nodes);
        printf("Proc: %d\n", numprocs);
/*
        for (int i = 0; i < unique_nodes; i++) {
            printf("Node %s: %d\n", &all_proc_names[i * MPI_MAX_PROCESSOR_NAME], proc_count_per_node[i]);
        }
*/

        free(proc_count_per_node);
    }

    if (all_proc_names != NULL) {
        free(all_proc_names);
    }

    char *filename = argv[1];

    INFO_Matrix A_info;
    A_info.recvcounts = (int *)malloc(numprocs * sizeof(int));
    A_info.displs = (int *)malloc(numprocs * sizeof(int));
	CSR_Matrix *A_loc = (CSR_Matrix *)malloc(sizeof(CSR_Matrix));
	csr_init_matrix(A_loc);
    MPI_csr_load_matrix(filename, A_loc, &A_info);

    int loc_vec_size = A_loc->rows;
    int vec_size = A_info.rows;
    double *x_loc, *y_loc, *x, *y;
    x_loc   = (double *)malloc(loc_vec_size * sizeof(double));
    y_loc   = (double *)malloc(loc_vec_size * sizeof(double));
    x       = (double *)malloc(vec_size * sizeof(double));
    y       = (double *)malloc(vec_size * sizeof(double));

#ifdef RANDOM
    srand(time(NULL) + myid);
    for (int i = 0; i < loc_vec_size; i++) {
        x_loc[i] = (double)rand() / RAND_MAX;
    }
    double alpha = (double)rand() / RAND_MAX;
#else
    for (int i = 0; i < loc_vec_size; i++) {
        x_loc[i] = 1.0;
    }
    double alpha = 1.0;
#endif

    double total_time_allgath = 0.0;
    double total_time_mv = 0.0;
    double total_time_dscal = 0.0;
    double total_time_daxpy = 0.0;
    double total_time_ddot = 0.0;
    double total_time_allred = 0.0;
    MPI_Request x_req, xTy_req;
    double xTy;
    double start_time, end_time;

    for (int i = 0; i < 100; i++) {
        MPI_Barrier(MPI_COMM_WORLD); start_time = MPI_Wtime();
        MPI_Iallgatherv(x_loc, loc_vec_size, MPI_DOUBLE, x, A_info.recvcounts, A_info.displs, MPI_DOUBLE, MPI_COMM_WORLD, &x_req);
        MPI_Wait(&x_req, MPI_STATUS_IGNORE);
        MPI_Barrier(MPI_COMM_WORLD); end_time = MPI_Wtime();
        total_time_allgath += (end_time - start_time);

        MPI_Barrier(MPI_COMM_WORLD); start_time = MPI_Wtime();
        csr_mv(A_loc, x, y_loc);
        MPI_Barrier(MPI_COMM_WORLD); end_time = MPI_Wtime();
        total_time_mv += (end_time - start_time);

        MPI_Barrier(MPI_COMM_WORLD); start_time = MPI_Wtime();
        my_dscal(loc_vec_size, alpha, y_loc);
        MPI_Barrier(MPI_COMM_WORLD); end_time = MPI_Wtime();
        total_time_dscal += (end_time - start_time);

        MPI_Barrier(MPI_COMM_WORLD); start_time = MPI_Wtime();
        my_daxpy(loc_vec_size, alpha, x_loc, y_loc);
        MPI_Barrier(MPI_COMM_WORLD); end_time = MPI_Wtime();
        total_time_daxpy += (end_time - start_time);

        MPI_Barrier(MPI_COMM_WORLD); start_time = MPI_Wtime();
        xTy = my_ddot(loc_vec_size, x_loc, y_loc);
        MPI_Barrier(MPI_COMM_WORLD); end_time = MPI_Wtime();
        total_time_ddot += (end_time - start_time);

        MPI_Barrier(MPI_COMM_WORLD); start_time = MPI_Wtime();
        MPI_Iallreduce(MPI_IN_PLACE, &xTy, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD, &xTy_req);
        MPI_Wait(&xTy_req, MPI_STATUS_IGNORE);
        MPI_Barrier(MPI_COMM_WORLD); end_time = MPI_Wtime();
        total_time_allred += (end_time - start_time);
    }

    double avg_time_iall    = total_time_allgath / 100.0;
    double avg_time_mv      = total_time_mv / 100.0;
    double avg_time_dscal   = total_time_dscal / 100.0;
    double avg_time_daxpy   = total_time_daxpy / 100.0;
    double avg_time_ddot    = total_time_ddot / 100.0;
    double avg_time_allred  = total_time_allred / 100.0;

    if (myid == 0) {
        printf("avg gatherv: %e [sec.]\n", avg_time_iall);
        printf("avg csr_mv : %e [sec.]\n", avg_time_mv);
        printf("avg dscal  : %e [sec.]\n", avg_time_dscal);
        printf("avg daxpy  : %e [sec.]\n", avg_time_daxpy);
        printf("avg ddot   : %e [sec.]\n", avg_time_ddot);
        printf("avg allred : %e [sec.]\n", avg_time_allred);
    }

#ifndef RANDOM
    for (int proc = 0; proc < numprocs; proc++) {
        if (myid == proc) {
            printf("proc %d, rows = %d, nz = %d, last ptr = %d, A.nz = %d, A.rows = %d, A.cols = %d, A.code = %.4s\n", 
                    myid, A_loc->rows, A_loc->nz, A_loc->ptr[A_loc->rows], A_info.nz, A_info.rows, A_info.cols, A_info.code);

            for (int i = 0; i < A_loc->rows; i++) {
                printf("%f ", y_loc[i]);
            } printf("\n");

        }
    }
#endif

	csr_free_matrix(A_loc); free(A_loc);
    free(x_loc); free(y_loc); free(x); free(y);
    free(A_info.recvcounts);  free(A_info.displs);

	MPI_Finalize();
	return 0;
}

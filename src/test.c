// mpicc -O3 src/test.c src/matrix.c src/vector.c src/mmio.c -I src
// mpirun -np 4 ./a.out data/small_test.mtx

#include <mpi.h>
#include <math.h>
#include <time.h> 

#include "matrix.h"
#include "vector.h"

//#define RANDOM

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

    double start_time, end_time;

    char *filename = argv[1];

    INFO_Matrix A_info;
    A_info.recvcounts = (int *)malloc(numprocs * sizeof(int));
    A_info.displs = (int *)malloc(numprocs * sizeof(int));
	CSR_Matrix *A_loc = (CSR_Matrix *)malloc(sizeof(CSR_Matrix)); csr_init_matrix(A_loc);
    CSR_Matrix *A_loc_diag = (CSR_Matrix *)malloc(sizeof(CSR_Matrix)); csr_init_matrix(A_loc_diag);
    CSR_Matrix *A_loc_offd = (CSR_Matrix *)malloc(sizeof(CSR_Matrix)); csr_init_matrix(A_loc_offd);

    MPI_Barrier(MPI_COMM_WORLD); start_time = MPI_Wtime();
    MPI_csr_load_matrix(filename, A_loc, &A_info);
    MPI_Barrier(MPI_COMM_WORLD); end_time = MPI_Wtime();

    if (myid == 0) {
        printf("IO(all) time  : %e [sec.]\n", end_time - start_time);
    }

    MPI_Barrier(MPI_COMM_WORLD); start_time = MPI_Wtime();
    MPI_csr_load_matrix_block(filename, A_loc_diag, A_loc_offd, &A_info);
    MPI_Barrier(MPI_COMM_WORLD); end_time = MPI_Wtime();

    if (myid == 0) {
        printf("IO(d&o) time  : %e [sec.]\n", end_time - start_time);
    }

    int loc_vec_size = A_loc->rows;
    int vec_size = A_info.rows;
    double *x_loc, *y_loc, *x, *y, *y_loc_ans;
    x_loc   = (double *)malloc(loc_vec_size * sizeof(double));
    y_loc   = (double *)malloc(loc_vec_size * sizeof(double));
    y_loc_ans = (double *)malloc(loc_vec_size * sizeof(double));
    x       = (double *)malloc(vec_size * sizeof(double));
    //y       = (double *)malloc(vec_size * sizeof(double));

#ifdef RANDOM
    srand(time(NULL) + myid);
    for (int i = 0; i < loc_vec_size; i++) {
        x_loc[i] = (double)rand() / RAND_MAX;
    }
    double alpha = (double)rand() / RAND_MAX;
#else
    for (int i = 0; i < loc_vec_size; i++) {
        x_loc[i] = myid + i;
    }
    double alpha = 1.0;
#endif

    int numsend = numprocs - 1;

    double **x_recv = (double **)malloc(numsend * sizeof(double *));
    int *recv_procs = (int *)malloc(numsend * sizeof(int));

    int proc_idx = 0;
    for (int i = 0; i < numprocs; i++) {
        if (i != myid) {
            recv_procs[proc_idx] = i;
            x_recv[proc_idx] = (double *)malloc(A_info.recvcounts[i] * sizeof(double));
            proc_idx++;
        }
    }

    MPI_Barrier(MPI_COMM_WORLD); start_time = MPI_Wtime();
    MPI_csr_spmv_async(A_loc_diag, A_loc_offd, &A_info, x_loc, x_recv, y_loc, numsend, myid, recv_procs);
    MPI_Barrier(MPI_COMM_WORLD); end_time = MPI_Wtime();

    MPI_Barrier(MPI_COMM_WORLD); start_time = MPI_Wtime();
    MPI_csr_spmv_ovlap(A_loc_diag, A_loc_offd, &A_info, x_loc, x, y_loc);
    MPI_Barrier(MPI_COMM_WORLD); end_time = MPI_Wtime();

    MPI_Barrier(MPI_COMM_WORLD); start_time = MPI_Wtime();
    MPI_csr_spmv(A_loc, &A_info, x_loc, x, y_loc_ans);
    MPI_Barrier(MPI_COMM_WORLD); end_time = MPI_Wtime();

    double local_sum = 0.0;
    double global_sum = 0.0;
    for (int i = 0; i < loc_vec_size; i++) {
        double diff = y_loc[i] - y_loc_ans[i];
        local_sum += diff * diff;
    }
    MPI_Allreduce(&local_sum, &global_sum, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
    double norm_error = sqrt(global_sum);
    if (myid == 0) {
        printf("Norm error    : %e\n", norm_error);
    }

	csr_free_matrix(A_loc); free(A_loc);
    csr_free_matrix(A_loc_diag); free(A_loc_diag);
    csr_free_matrix(A_loc_offd); free(A_loc_offd);
    free(x_loc); free(y_loc); free(y_loc_ans); free(x); //free(y); 
    free(A_info.recvcounts);  free(A_info.displs);

	MPI_Finalize();
	return 0;
}

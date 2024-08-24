// 行で分割したローカルな行列を、その要素数分のメモリ確保　2回ファイルの読み取りを行う必要あり

// mpicc -O3 test/readcsr.c test/coo.c test/csr.c test/mmio.c -I test
// mpirun -np 4 ./a.out data/small_test.mtx

#include <mpi.h>
#include <math.h>
#include <time.h> 

#include "matrix.h"

int main(int argc, char *argv[]) {
    
    int     numprocs, myid, namelen;

	char proc_name[MPI_MAX_PROCESSOR_NAME];
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &myid);
	MPI_Get_processor_name(proc_name, &namelen);

    int     m, n, nz;
    char    *filename = argv[1];

    FILE *file = fopen(filename, "r");
    MM_typecode code;

	if (mm_read_banner(file, &code) != 0){
		fprintf(stderr, "ERROR: Could not process Matrix Market banner.\n");
		exit(EXIT_FAILURE);
	}
	if ((mm_read_mtx_crd_size(file, &m, &n, &nz)) != 0){
		fprintf(stderr, "ERROR: Could not read matrix size.\n");
		exit(EXIT_FAILURE);
	}

    COO_Matrix *temp = (COO_Matrix *)malloc(sizeof(COO_Matrix));
    coo_init_matrix(temp);

    int rows_per_proc = m / numprocs;
    int extra_rows = m % numprocs;
    int start_row = myid * rows_per_proc + (myid < extra_rows ? myid : extra_rows);
    int end_row = start_row + rows_per_proc + (myid < extra_rows ? 1 : 0);

    int row, col;
    double val; int ival;
    int nz_loc = 0;

    for (int i = 0; i < nz; i++) {
		if(mm_is_pattern(code)){
			if(fscanf(file, "%d %d\n", &row, &col) < 2){
				fprintf(stderr, "ERROR: reading matrix data.\n");
				exit(EXIT_FAILURE);
			}
			val = 1.0;
		} else if (mm_is_real(code)){
			if(fscanf(file, "%d %d %lg\n", &row, &col, &val) < 3){
				fprintf(stderr, "ERROR: reading matrix data.\n");
				exit(EXIT_FAILURE);
			}
		} else if (mm_is_integer(code)){
			if(fscanf(file, "%d %d %d\n", &row, &col, &ival) < 3){
				fprintf(stderr, "ERROR: reading matrix data.\n");
				exit(EXIT_FAILURE);
			}
			val = (double)ival;
		}
        row--; //col--;
        if (row >= start_row && row < end_row) nz_loc++;   
    }

    temp->rows = end_row - start_row;
    temp->cols = n;
    temp->nz   = nz_loc;
    temp->val  = (double *)malloc(nz_loc * sizeof(double));
    temp->row  = (unsigned int *)malloc(nz_loc * sizeof(unsigned int));
    temp->col  = (unsigned int *)malloc(nz_loc * sizeof(unsigned int));

    // ファイルポインタを再度先頭に戻す
    fseek(file, 0, SEEK_SET);
    mm_read_banner(file, &code); // バナーを再度読み込み
    mm_read_mtx_crd_size(file, &m, &n, &nz); // 行列サイズを再度読み込み

    nz_loc = 0;
    for (int i = 0; i < nz; i++) {
		if(mm_is_pattern(code)){
			if(fscanf(file, "%d %d\n", &row, &col) < 2){
				fprintf(stderr, "ERROR: reading matrix data.\n");
				exit(EXIT_FAILURE);
			}
			val = 1.0;
		} else if (mm_is_real(code)){
			if(fscanf(file, "%d %d %lg\n", &row, &col, &val) < 3){
				fprintf(stderr, "ERROR: reading matrix data.\n");
				exit(EXIT_FAILURE);
			}
		} else if (mm_is_integer(code)){
			if(fscanf(file, "%d %d %d\n", &row, &col, &ival) < 3){
				fprintf(stderr, "ERROR: reading matrix data.\n");
				exit(EXIT_FAILURE);
			}
			val = (double)ival;
		}
        row--; col--;
        if (row >= start_row && row < end_row) {
            temp->row[nz_loc] = row - start_row;
            temp->col[nz_loc] = col;
            temp->val[nz_loc] = val;
            nz_loc++;
        }        
    }

    fclose(file);

    //printf("Hello World from processor %d out of %d, executing on %s, temp.nz = %d\n", myid, numprocs, proc_name, temp->nz);

	CSR_Matrix *A_loc = (CSR_Matrix *)malloc(sizeof(CSR_Matrix));
	csr_init_matrix(A_loc);
	coo2csr(temp, A_loc);
    coo_free_matrix(temp); free(temp);

	printf("proc %d, nz = %d, last ptr = %d\n", myid, A_loc->nz, A_loc->ptr[A_loc->rows]);

	csr_free_matrix(A_loc); free(A_loc);

	MPI_Finalize();
	return 0;
}

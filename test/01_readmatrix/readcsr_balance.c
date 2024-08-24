// 行で分割したローカルな行列を、その要素数分のメモリ確保　2回ファイルの読み取りを行う必要あり

// mpicc -O3 test/readmatrix/readcsr_balance.c test/readmatrix/coo.c test/readmatrix/csr.c test/readmatrix/mmio.c -I test/readmatrix
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

    int m, n, nz;
    char *filename = argv[1];

    FILE *file = fopen(filename, "r");
    MM_typecode code;

    if (mm_read_banner(file, &code) != 0) {
        fprintf(stderr, "ERROR: Could not process Matrix Market banner.\n");
        exit(EXIT_FAILURE);
    }
    if ((mm_read_mtx_crd_size(file, &m, &n, &nz)) != 0) {
        fprintf(stderr, "ERROR: Could not read matrix size.\n");
        exit(EXIT_FAILURE);
    }

    COO_Matrix *temp = (COO_Matrix *)malloc(sizeof(COO_Matrix));
    coo_init_matrix(temp);

    // 1. 各行の非ゼロ要素数をカウント
    int *nz_per_row = (int *)calloc(m, sizeof(int));
    int row, col, ival;
    double val;
    
    for (int i = 0; i < nz; i++) {
        if (mm_is_pattern(code)) {
            fscanf(file, "%d %d\n", &row, &col);
            val = 1.0;
        } else if (mm_is_real(code)) {
            fscanf(file, "%d %d %lg\n", &row, &col, &val);
        } else if (mm_is_integer(code)) {
            fscanf(file, "%d %d %d\n", &row, &col, &ival);
            val = (double)ival;
        }
        row--; // 0ベースに修正
        nz_per_row[row]++;
    }

    // 各プロセスの担当行数を計算
    int *rows_per_proc = (int *)malloc(numprocs * sizeof(int));
	int *nz_per_proc = (int *)malloc(numprocs * sizeof(int));
    int target_nz_per_proc = nz / numprocs;
    int start_row = 0, end_row = 0;
    int cumulative_nz = 0;

	int my_start_row, my_end_row;
	start_row = 0; // 初期化
	for (int proc = 0; proc < numprocs; proc++) {
		cumulative_nz = 0;
		for (int i = start_row; i < m; i++) {
			cumulative_nz += nz_per_row[i];  // 各行の非ゼロ要素数をカウント
			if (cumulative_nz >= target_nz_per_proc && proc < numprocs - 1) {  // ここで調整
				end_row = i + 1;
				nz_per_proc[proc] = cumulative_nz;
				break;
			}
		}
		if (proc == numprocs - 1) {
			end_row = m; // 最後のプロセスは残り全ての行を担当
			nz_per_proc[proc] = cumulative_nz;
		}
		rows_per_proc[proc] = end_row - start_row;
		if (proc == myid) {
			my_start_row = start_row;
			my_end_row = end_row;
		}
		start_row = end_row;
	}
	start_row = my_start_row;
	end_row = my_end_row;

	int nz_loc = nz_per_proc[myid];

	free(nz_per_row); free(rows_per_proc); free(nz_per_proc);

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

	printf("proc %d, start_row = %d, end_row = %d, rows = %d, nz = %d, last ptr = %d\n", myid, start_row, end_row, A_loc->rows, A_loc->nz, A_loc->ptr[A_loc->rows]);

	csr_free_matrix(A_loc); free(A_loc);

	MPI_Finalize();
	return 0;
}

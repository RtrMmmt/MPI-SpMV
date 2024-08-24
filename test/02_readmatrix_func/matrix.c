#include "matrix.h"

void coo_init_matrix(COO_Matrix *m){
	m->val = NULL;
	m->row = NULL;
	m->col = NULL;
	m->nz = m->rows = m->cols = 0;
}

void coo_free_matrix(COO_Matrix *m){
	if(m->val != NULL) free(m->val);
	if(m->row != NULL) free(m->row);
	if(m->col != NULL) free(m->col);
	m->val = NULL;
	m->row = NULL;
	m->col = NULL;
	m->nz = m->rows = m->cols = 0;
}

int coo_load_matrix(char* filename, COO_Matrix *coo){
	FILE *file;
	MM_typecode code;
	int m, n, nz, i, ival;
	unsigned int dc = 0;
	
	/* ファイルを開く */
	file = fopen(filename, "r");
	if(file == NULL){
		fprintf(stderr, "ERROR: can't open file \"%s\"\n", filename);
		exit(EXIT_FAILURE);
	}
	
    /* codeにMatrix Marketフォーマットの情報を格納 */
	if (mm_read_banner(file, &code) != 0){
		fprintf(stderr, "ERROR: Could not process Matrix Market banner.\n");
		exit(EXIT_FAILURE);
	}
	
	/* 対象としていない行列の場合エラー */
	if(!mm_is_matrix(code) || !mm_is_sparse(code) || !mm_is_coordinate(code) ){
		fprintf(stderr, "Sorry, this application does not support ");
		fprintf(stderr, "Market Market type: [%s]\n", mm_typecode_to_str(code));
		exit(EXIT_FAILURE);
	}
	
	/* 行列サイズを読み込む */
	if ((mm_read_mtx_crd_size(file, &m, &n, &nz)) != 0){
		fprintf(stderr, "ERROR: Could not read matrix size.\n");
		exit(EXIT_FAILURE);
	}
	coo->rows = m;
	coo->cols = n;
	coo->nz = nz;
	
	/* メモリの確保 */
	coo->val = (double *) malloc(nz * sizeof(double));
	coo->row = (unsigned int *) malloc(nz * sizeof(unsigned int));
	coo->col = (unsigned int *) malloc(nz * sizeof(unsigned int));
	
	/* 行列の非ゼロ要素を読み込む */
	for(i = 0; i < nz; i++){
		if(mm_is_pattern(code)){
			if(fscanf(file, "%d %d\n", &(coo->row[i]), &(coo->col[i]) ) < 2){
				fprintf(stderr, "ERROR: reading matrix data.\n");
				exit(EXIT_FAILURE);
			}
			coo->val[i] = 1.0;
		} else if (mm_is_real(code)){
			if(fscanf(file, "%d %d %lg\n", &(coo->row[i]), &(coo->col[i]), &(coo->val[i])) < 3){
				fprintf(stderr, "ERROR: reading matrix data.\n");
				exit(EXIT_FAILURE);
			}
		} else if (mm_is_integer(code)){
			if(fscanf(file, "%d %d %d\n", &(coo->row[i]), &(coo->col[i]), &ival) < 3){
				fprintf(stderr, "ERROR: reading matrix data.\n");
				exit(EXIT_FAILURE);
			}
			coo->val[i] = (double)ival;
		}
		coo->row[i]--;
		coo->col[i]--;
		if(coo->row[i] == coo->col[i]) { ++dc; }
	}
	
	fclose(file);
	
	return mm_is_symmetric(code);
}

void coo_mv(COO_Matrix *m, double *x, double *y){
	unsigned int    i;
	double          *val = m->val;
	unsigned int    *row = m->row;
	unsigned int    *col = m->col;
	
	for(i = 0; i < m->rows; i++){
		y[i] = 0.0;
	}
	
	for(i = 0; i < m->nz; i++){
		y[row[i]] += val[i] * x[col[i]];
	}
}

void coo_mv_sym(COO_Matrix *m, double *x, double *y){
	unsigned int    i, r, c;
	double          *val = m->val;
	unsigned int    *row = m->row;
	unsigned int    *col = m->col;
	
	for(i = 0; i < m->rows; i++){
		y[i] = 0.0;
	}
	
	for(i = 0; i < m->nz; i++){
		r = row[i];
		c = col[i];
		y[r] += val[i] * x[c];
		if(r != c){
			y[c] += val[i] * x[r];
		}
	}
}

void coo_copy(COO_Matrix *in, COO_Matrix *out){
	unsigned int i;
	out->val = (double *) malloc(in->nz * sizeof(double));
	out->row = (unsigned int *) malloc(in->nz * sizeof(unsigned int));
	out->col = (unsigned int *) malloc(in->nz * sizeof(unsigned int));
	out->nz = in->nz;
	out->rows = in->rows;
	out->cols = in->cols;
	
	for(i = 0; i < in->nz; i++){
		out->val[i] = in->val[i]; 
	}
	for(i = 0; i < in->nz; i++){
		out->row[i] = in->row[i]; 
	}
	for(i = 0; i < in->nz; i++){
		out->col[i] = in->col[i]; 
	}
}

void coo_reorder_by_rows(COO_Matrix *m){
	cooMergeSort(m->row, m->col, m->val, m->nz);
}

void cooBottomUpMerge(
	unsigned int *A, unsigned int *A2, double *A3, 
	unsigned int iLeft, unsigned int iRight, unsigned int iEnd, 
	unsigned int *B, unsigned int *B2, double *B3 ) {
	
	unsigned int i0 = iLeft;
	unsigned int i1 = iRight;
	unsigned int j;
 
	/* While there are elements in the left or right runs */
	for (j = iLeft; j < iEnd; j++){
		/* If left run head exists and is <= existing right run head */
		if (i0 < iRight && (i1 >= iEnd || A[i0] <= A[i1])){
			B[j] = A[i0];
			B2[j] = A2[i0];
			B3[j] = A3[i0];
			i0++;
		} else {
			B[j] = A[i1];
			B2[j] = A2[i1];
			B3[j] = A3[i1];
			i1++;
		}
	}
}

void cooCopyArray(unsigned int *B, unsigned int *A, unsigned int n) {
	unsigned int i;
   	for(i = 0; i < n; i++){
        A[i] = B[i];
	}
}

void cooCopyArrayDouble(double *B, double *A, unsigned int n) {
	unsigned int i;
   	for(i = 0; i < n; i++){
        A[i] = B[i];
	}
}

inline unsigned int cooMergeMin(unsigned int a, unsigned int b){
	return (a<b)?a:b;
}

void cooMergeSort(unsigned int *A, unsigned int *A2, double *A3, unsigned int n) {
	/* array A[] has the items to sort; array B[] is a work array */
	unsigned int	width,i;
	unsigned int	*t;
	double			*td;
	unsigned int	swaps = 0;
	unsigned int	*B = (unsigned int *)malloc(n * sizeof(unsigned int));
	unsigned int	*B2 = (unsigned int *)malloc(n * sizeof(unsigned int));
	double 			*B3 = (double *)malloc(n * sizeof(double));
	/* Each 1-element run in A is already "sorted". */
	/* Make successively longer sorted runs of length 2, 4, 8, 16... until whole array is sorted. */
	for (width = 1; width < n; width = 2 * width) {
      	/* Array A is full of runs of length width. */
    	for (i = 0; i < n; i = i + 2 * width){
         	/* Merge two runs: A[i:i+width-1] and A[i+width:i+2*width-1] to B[] */
         	/* or copy A[i:n-1] to B[] ( if(i+width >= n) ) */
        	cooBottomUpMerge(A,A2,A3, i, cooMergeMin(i+width, n), cooMergeMin(i+2*width, n), B,B2,B3);
    	}
      	/* Now work array B is full of runs of length 2*width. */
      	/* Copy array B to array A for next iteration. */
      	/* A more efficient implementation would swap the roles of A and B */
      	//cooCopyArray(B, A, n);
		//cooCopyArray(B2, A2, n);
		//cooCopyArrayDouble(B3, A3, n);
		t = B; B = A; A = t;
		t = B2; B2 = A2; A2 = t;
		td = B3; B3 = A3; A3 = td;
		swaps++;
      	/* Now array A is full of runs of length 2*width. */
	}
	if(swaps%2 == 1) {
		//printf("Swap A and B\n");
		t = B; B = A; A = t;
		t = B2; B2 = A2; A2 = t;
		td = B3; B3 = A3; A3 = td;
		cooCopyArray(B, A, n);
		cooCopyArray(B2, A2, n);
		cooCopyArrayDouble(B3, A3, n);
	}
	
	free(B);
	free(B2);
	free(B3);
}

void csr_init_matrix(CSR_Matrix *m){
	m->val = NULL;
	m->col = NULL;
	m->ptr = NULL;
	m->nz = m->rows = m->cols = 0;
}

void csr_free_matrix(CSR_Matrix *m){
	if(m->val != NULL) free(m->val);
	if(m->col != NULL) free(m->col);
	if(m->ptr != NULL) free(m->ptr);
	m->val = NULL;
	m->col = NULL;
	m->ptr = NULL;
	m->nz = m->rows = m->cols = 0;
}

void coo2csr(COO_Matrix *in, CSR_Matrix *out){
	unsigned int i;
	unsigned int tot = 0;
	COO_Matrix coo;
	
	// Init mem for CSR matrix
	out->val = (double *) malloc(in->nz * sizeof(double));
	out->col = (unsigned int *) malloc(in->nz * sizeof(unsigned int));
	out->ptr = (unsigned int *) malloc(((in->rows)+1) * sizeof(unsigned int));
	out->nz = in->nz;
	out->rows = in->rows;
	out->cols = in->cols;
	
	// Copy in matrix
	coo_copy(in, &coo);
	coo_reorder_by_rows(&coo);
	
	// Copy elements row by row
	out->ptr[0] = tot;
	for(i = 0; i < coo.rows; i++){
		while(tot < coo.nz && coo.row[tot] == i){
			out->val[tot] = coo.val[tot];
			out->col[tot] = coo.col[tot];
			tot++;
		}
		out->ptr[i+1] = tot;
	}
	
	coo_free_matrix(&coo);
}

int csr_load_matrix(char* filename, CSR_Matrix *m){
	int sym;
	COO_Matrix temp;
	coo_init_matrix(&temp);
	sym = coo_load_matrix(filename, &temp);
	coo2csr(&temp, m);
	coo_free_matrix(&temp);
	return sym;
}

void csr_mv(CSR_Matrix *m, double *x, double *y){
	unsigned int    i, j, end;
	double          tempy;
	double          *val = m->val;
	unsigned int    *col = m->col;
	unsigned int    *ptr = m->ptr;
	end = 0;
	
	// Loop over rows.
	for(i = 0; i < m->rows; i++) {
		tempy = 0.0;
		j = end;
		end = ptr[i+1];
		// Loop over non-zero elements in row i.
		for( ; j < end; j++) {
			tempy += val[j] * x[col[j]];
		}
		y[i] = tempy;
	}
}

void csr_mv_sym(CSR_Matrix *m, double *x, double *y) {
	unsigned int    i, j, end, mcol;
	double          tempy;
	double          *val = m->val;
	unsigned int    *col = m->col;
	unsigned int    *ptr = m->ptr;
	end = 0;

    for (i = 0; i < m->rows; i++) {
        y[i] = 0.0;
    }

	for(i = 0; i < m->rows; i++){
		tempy = 0.0;
		j = end;
		end = ptr[i+1];
		for( ; j < end; j++){
			mcol = col[j];
			tempy += val[j] * x[mcol];
			if(i != mcol){
				y[mcol] += val[j] * x[i];
			}
		}
		y[i] += tempy;
	}
}

void MPI_coo_load_matrix(char *filename, COO_Matrix *matrix_loc, INFO_Matrix *matrix_info) {
	int numprocs, myid;
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);

	int m, n, nz;

	FILE *file;
	MM_typecode code;
	
	file = fopen(filename, "r");

    if (mm_read_banner(file, &code) != 0) {
        fprintf(stderr, "ERROR: Could not process Matrix Market banner.\n");
        exit(EXIT_FAILURE);
    }
    if ((mm_read_mtx_crd_size(file, &m, &n, &nz)) != 0) {
        fprintf(stderr, "ERROR: Could not read matrix size.\n");
        exit(EXIT_FAILURE);
    }

	matrix_info->nz = nz; matrix_info->rows = m; matrix_info->cols = n;
	strncpy(matrix_info->code, code, sizeof(MM_typecode));

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

    matrix_loc->rows = end_row - start_row;
    matrix_loc->cols = n;
    matrix_loc->nz   = nz_loc;
    matrix_loc->val  = (double *)malloc(nz_loc * sizeof(double));
    matrix_loc->row  = (unsigned int *)malloc(nz_loc * sizeof(unsigned int));
    matrix_loc->col  = (unsigned int *)malloc(nz_loc * sizeof(unsigned int));

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
            matrix_loc->row[nz_loc] = row - start_row;
            matrix_loc->col[nz_loc] = col;
            matrix_loc->val[nz_loc] = val;
            nz_loc++;
        }
    }

    fclose(file);
}

void MPI_csr_load_matrix(char *filename, CSR_Matrix *matrix_loc, INFO_Matrix *matrix_info) {
    COO_Matrix *temp = (COO_Matrix *)malloc(sizeof(COO_Matrix));
    coo_init_matrix(temp);
	MPI_coo_load_matrix(filename, temp, matrix_info);
	csr_init_matrix(matrix_loc);
	coo2csr(temp, matrix_loc);
    coo_free_matrix(temp); free(temp);
}
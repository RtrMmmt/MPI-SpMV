#ifndef MATRIX_H
#define MATRIX_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include "mmio.h"

typedef struct {
	double          *val;  //非ゼロ要素の値を格納する配列
	unsigned int    *row;  //行のインデックスを格納する配列
	unsigned int    *col;  //列のインデックスを格納する配列
	unsigned int    nz;    //非ゼロ要素の数
	unsigned int    rows;  //行数
	unsigned int    cols;  //列数
} COO_Matrix;

typedef struct {
	double          *val; //非ゼロ要素の値を格納する配列
	unsigned int    *col; //列のインデックスを格納する配列
	unsigned int    *ptr; //各行の開始ポインタを格納する配列
	unsigned int    nz;   //非ゼロ要素の数
	unsigned int    rows; //行数
	unsigned int    cols; //列数
} CSR_Matrix;

typedef struct {
	unsigned int 	nz, rows, cols;
	MM_typecode 	code;
    int 			*recvcounts;  // 各プロセスの受信カウント
    int 			*displs;      // 各プロセスのデータの開始位置
} INFO_Matrix;

void coo_init_matrix(COO_Matrix *m);
void coo_free_matrix(COO_Matrix *m);
int  coo_load_matrix(char* filename, COO_Matrix *coo);
void coo_mv(COO_Matrix *m, double *x, double *y);
void coo_mv_sym(COO_Matrix *m, double *x, double *y);
void coo_copy(COO_Matrix *in, COO_Matrix *out);
void coo_reorder_by_rows(COO_Matrix *m);

void cooMerge(unsigned int *A, unsigned int *A2, double *A3, unsigned int *B, unsigned int *B2, double *B3, int left, int mid, int right);
void cooMergeSort(unsigned int *A, unsigned int *A2, double *A3, unsigned int *B, unsigned int *B2, double *B3, int left, int right);

void csr_init_matrix(CSR_Matrix *m);
void csr_free_matrix(CSR_Matrix *m);
void coo2csr(COO_Matrix *in, CSR_Matrix *out);
int  csr_load_matrix(char* filename, CSR_Matrix *m);
void csr_mv(CSR_Matrix *m, double *x, double *y);
void csr_mv_sym(CSR_Matrix *m, double *x, double *y);

void MPI_coo_load_matrix(char *filename, COO_Matrix *matrix_loc, INFO_Matrix *matrix_info);
void MPI_csr_load_matrix(char *filename, CSR_Matrix *matrix_loc, INFO_Matrix *matrix_info);

void MPI_csr_spmv(CSR_Matrix *matrix_loc, INFO_Matrix *matrix_info, double *x_loc, double **x_recv, double *y_loc, int numprocs, int myid, int *recv_procs);
void mult(CSR_Matrix* A_loc, double* x_part, double* y_loc, int start_index, int end_index);

#endif // MATRIX_H

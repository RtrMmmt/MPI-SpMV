#ifndef CSR_H
#define CSR_H

#include "coo.h"

typedef struct {
	double          *val; //非ゼロ要素の値を格納する配列
	unsigned int    *col; //列のインデックスを格納する配列
	unsigned int    *ptr; //各行の開始ポインタを格納する配列
	unsigned int    nz;   //非ゼロ要素の数
	unsigned int    rows; //行数
	unsigned int    cols; //列数
} CSR_Matrix;

void csr_init_matrix(CSR_Matrix *m);
void csr_free_matrix(CSR_Matrix *m);
void coo2csr(COO_Matrix *in, CSR_Matrix *out);
int csr_load_matrix(char* filename, CSR_Matrix *m);
void csr_mv(CSR_Matrix *m, double *x, double *y);
void csr_mv_sym(CSR_Matrix *m, double *x, double *y);

#endif // CSR_H
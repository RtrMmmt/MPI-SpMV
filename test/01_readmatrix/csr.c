#include <stdio.h>
#include <stdlib.h>
#include "csr.h"

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
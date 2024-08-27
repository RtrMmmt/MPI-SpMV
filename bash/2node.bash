#!/bin/bash
#PJM -L "rscunit=fx"
#PJM -L "rscgrp=fx-debug"
#PJM -L "node=2"
#PJM --mpi "proc=64"
#PJM -L "elapse=30:00"
mpiexec -n 2 ./spmv data/Transport.mtx
mpiexec -n 4 ./spmv data/Transport.mtx
mpiexec -n 8 ./spmv data/Transport.mtx
mpiexec -n 16 ./spmv data/Transport.mtx
mpiexec -n 32 ./spmv data/Transport.mtx
mpiexec -n 64 ./spmv data/Transport.mtx
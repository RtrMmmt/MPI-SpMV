#!/bin/bash
#PJM -L "rscunit=fx"
#PJM -L "rscgrp=fx-debug"
#PJM -L "node=5"
#PJM --mpi "proc=192"
#PJM -L "elapse=30:00"
mpiexec -n 5 ./spmv data/Transport.mtx
mpiexec -n 10 ./spmv data/Transport.mtx
mpiexec -n 20 ./spmv data/Transport.mtx
mpiexec -n 40 ./spmv data/Transport.mtx
mpiexec -n 80 ./spmv data/Transport.mtx

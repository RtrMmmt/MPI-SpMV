#!/bin/bash
#PJM -L "rscunit=fx"
#PJM -L "rscgrp=fx-debug"
#PJM -L "node=3"
#PJM --mpi "proc=192"
#PJM -L "elapse=30:00"
mpiexec -n 3 ./spmv data/Transport.mtx
mpiexec -n 6 ./spmv data/Transport.mtx
mpiexec -n 12 ./spmv data/Transport.mtx
mpiexec -n 24 ./spmv data/Transport.mtx
mpiexec -n 48 ./spmv data/Transport.mtx

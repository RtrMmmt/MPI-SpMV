#!/bin/bash
#PJM -L "rscunit=fx"
#PJM -L "rscgrp=fx-debug"
#PJM -L "node=12"
#PJM --mpi "proc=48"
#PJM -L "elapse=10:00"
mpiexec -of 4-12 -n 48 ./spmv data/Transport.mtx
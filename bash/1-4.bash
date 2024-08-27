#!/bin/bash
#PJM -L "rscunit=fx"
#PJM -L "rscgrp=fx-debug"
#PJM -L "node=1"
#PJM --mpi "proc=4"
#PJM -L "elapse=10:00"
mpiexec -of 1-4 -n 4 ./spmv data/Transport.mtx
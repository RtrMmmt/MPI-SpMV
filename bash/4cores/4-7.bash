#!/bin/bash
#PJM -L "rscunit=fx"
#PJM -L "rscgrp=fx-debug"
#PJM -L "node=7"
#PJM --mpi "proc=28"
#PJM -L "elapse=10:00"
mpiexec -of 4-7 -n 28 ./spmv data/Transport.mtx
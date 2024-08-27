#!/bin/bash
#PJM -L "rscunit=fx"
#PJM -L "rscgrp=fx-debug"
#PJM -L "node=8"
#PJM --mpi "proc=32"
#PJM -L "elapse=10:00"
mpiexec -of 4-8 -n 32 ./spmv data/Transport.mtx
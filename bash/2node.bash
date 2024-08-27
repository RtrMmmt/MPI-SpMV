#!/bin/bash
#PJM -L "rscunit=fx"
#PJM -L "rscgrp=fx-debug"
#PJM -L "node=2"
#PJM --mpi "proc=24"
#PJM -L "elapse=10:00"
mpiexec -of 2node -n 8 ./spmv data/Transport.mtx
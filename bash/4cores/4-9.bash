#!/bin/bash
#PJM -L "rscunit=fx"
#PJM -L "rscgrp=fx-debug"
#PJM -L "node=9"
#PJM --mpi "proc=36"
#PJM -L "elapse=10:00"
mpiexec -of 4-9 -n 36 ./spmv data/Transport.mtx
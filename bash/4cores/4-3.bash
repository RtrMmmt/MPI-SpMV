#!/bin/bash
#PJM -L "rscunit=fx"
#PJM -L "rscgrp=fx-debug"
#PJM -L "node=3"
#PJM --mpi "proc=12"
#PJM -L "elapse=10:00"
mpiexec -of 4-3 -n 12 ./spmv data/Transport.mtx
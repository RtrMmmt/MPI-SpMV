#!/bin/bash
#PJM -L "rscunit=fx"
#PJM -L "rscgrp=fx-debug"
#PJM -L "node=1"
#PJM --mpi "proc=2"
#PJM -L "elapse=10:00"
mpiexec -of 2-1 ./spmv data/Transport.mtx
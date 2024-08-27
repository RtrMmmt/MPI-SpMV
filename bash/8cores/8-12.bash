#!/bin/bash
#PJM -L "rscunit=fx"
#PJM -L "rscgrp=fx-debug"
#PJM -L "node=12"
#PJM --mpi "proc=96"
#PJM -L "elapse=10:00"
mpiexec -of 8-12 ./spmv data/Transport.mtx
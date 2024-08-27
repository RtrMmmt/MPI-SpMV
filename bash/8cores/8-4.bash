#!/bin/bash
#PJM -L "rscunit=fx"
#PJM -L "rscgrp=fx-debug"
#PJM -L "node=4"
#PJM --mpi "proc=32"
#PJM -L "elapse=10:00"
mpiexec -of 8-4 ./spmv data/Transport.mtx
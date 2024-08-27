#!/bin/bash
#PJM -L "rscunit=fx"
#PJM -L "rscgrp=fx-debug"
#PJM -L "node=6"
#PJM --mpi "proc=24"
#PJM -L "elapse=10:00"
mpiexec -of 4-6 -n 24 ./spmv data/Transport.mtx
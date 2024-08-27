#!/bin/bash
#PJM -L "rscunit=fx"
#PJM -L "rscgrp=fx-debug"
#PJM -L "node=2"
#PJM --mpi "proc=24"
#PJM -L "elapse=10:00"
mpiexec -n 8 -npernode 4 ./spmv data/Transport.mtx
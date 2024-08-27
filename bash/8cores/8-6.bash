#!/bin/bash
#PJM -L "rscunit=fx"
#PJM -L "rscgrp=fx-debug"
#PJM -L "node=6"
#PJM --mpi "proc=48"
#PJM -L "elapse=10:00"
mpiexec -of 8-6 ./spmv data/Transport.mtx
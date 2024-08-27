#!/bin/bash
#PJM -L "rscunit=fx"
#PJM -L "rscgrp=fx-debug"
#PJM -L "node=10"
#PJM --mpi "proc=40"
#PJM -L "elapse=10:00"
mpiexec -of 4-10 -n 40 ./spmv data/Transport.mtx
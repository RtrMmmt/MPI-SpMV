#!/bin/bash
#PJM -L "rscunit=fx"
#PJM -L "rscgrp=fx-debug"
#PJM -L "node=10"
#PJM --mpi "proc=80"
#PJM -L "elapse=10:00"
mpiexec -of 8-10 ./spmv data/Transport.mtx
#!/bin/bash
#PJM -L "rscunit=fx"
#PJM -L "rscgrp=fx-debug"
#PJM -L "node=7"
#PJM --mpi "proc=14"
#PJM -L "elapse=10:00"
mpiexec -of 2-7 ./spmv data/Transport.mtx
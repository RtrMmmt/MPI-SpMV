#!/bin/bash
#PJM -L "rscunit=fx"
#PJM -L "rscgrp=fx-debug"
#PJM -L "node=8"
#PJM --mpi "proc=16"
#PJM -L "elapse=10:00"
mpiexec -of 2-8 ./spmv data/Transport.mtx
#!/bin/bash
#PJM -L "rscunit=fx"
#PJM -L "rscgrp=fx-debug"
#PJM -L "node=12"
#PJM --mpi "proc=24"
#PJM -L "elapse=10:00"
mpiexec -of 2-12 ./spmv data/Transport.mtx
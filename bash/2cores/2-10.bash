#!/bin/bash
#PJM -L "rscunit=fx"
#PJM -L "rscgrp=fx-debug"
#PJM -L "node=10"
#PJM --mpi "proc=20"
#PJM -L "elapse=10:00"
mpiexec -of 2-10 ./spmv data/Transport.mtx
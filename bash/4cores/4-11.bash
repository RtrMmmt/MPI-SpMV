#!/bin/bash
#PJM -L "rscunit=fx"
#PJM -L "rscgrp=fx-debug"
#PJM -L "node=11"
#PJM --mpi "proc=44"
#PJM -L "elapse=10:00"
mpiexec -of 4-11 -n 44 ./spmv data/Transport.mtx
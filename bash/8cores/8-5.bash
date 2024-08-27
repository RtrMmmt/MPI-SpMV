#!/bin/bash
#PJM -L "rscunit=fx"
#PJM -L "rscgrp=fx-debug"
#PJM -L "node=5"
#PJM --mpi "proc=40"
#PJM -L "elapse=10:00"
mpiexec -of 8-5 ./spmv data/Transport.mtx
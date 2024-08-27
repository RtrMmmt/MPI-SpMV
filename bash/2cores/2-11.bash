#!/bin/bash
#PJM -L "rscunit=fx"
#PJM -L "rscgrp=fx-debug"
#PJM -L "node=11"
#PJM --mpi "proc=22"
#PJM -L "elapse=10:00"
mpiexec -of 2-11 ./spmv data/Transport.mtx
#!/bin/bash
#PJM -L "rscunit=fx"
#PJM -L "rscgrp=fx-debug"
#PJM -L "node=4"
#PJM --mpi "proc=16"
#PJM -L "elapse=10:00"
mpirun -np 16 ./spmv data/Transport.mtx
#!/bin/bash
#PJM -L "rscunit=fx"
#PJM -L "rscgrp=fx-debug"
#PJM -L "node=5"
#PJM --mpi "proc=20"
#PJM -L "elapse=10:00"
mpirun -np 20 ./spmv data/Transport.mtx
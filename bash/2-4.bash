#!/bin/bash
#PJM -L "rscunit=fx"
#PJM -L "rscgrp=fx-debug"
#PJM -L "node=2"
#PJM --mpi "proc=8"
#PJM -L "elapse=10:00"
mpirun -np 8 ./spmv data/Transport.mtx
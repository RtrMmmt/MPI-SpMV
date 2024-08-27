#!/bin/bash
#PJM -L "rscunit=fx"
#PJM -L "rscgrp=fx-debug"
#PJM -L "node=11"
#PJM --mpi "proc=88"
#PJM -L "elapse=10:00"
mpiexec -of 8-11./spmv data/Transport.mtx
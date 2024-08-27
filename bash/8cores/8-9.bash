#!/bin/bash
#PJM -L "rscunit=fx"
#PJM -L "rscgrp=fx-debug"
#PJM -L "node=9"
#PJM --mpi "proc=72"
#PJM -L "elapse=10:00"
mpiexec -of 8-9./spmv data/Transport.mtx
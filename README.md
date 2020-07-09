[![Generic badge](https://img.shields.io/badge/Stage-Development-blue.svg)](https://shields.io/)

# SONAR

## A custom benchmark for I/O systems

The purpose of this benchmark is to simulate different I/O patterns with
[configurable parameters](#configurable-parameters) in order to test
the performance for I/O systems. Initial development of Sonar is intended for
to testing the [LABIOS](https://dl.acm.org/doi/abs/10.1145/3307681.3325405)
I/O system. By obtaining performance metrics, the optimal configurations of
different I/O systems can be discovered.

## Configurable parameters:
- Application Workload
  - Type of Workload (input as # of reads, # of writes)
    - Read Heavy
    - Write Heavy
    - Read Only
    - Write Only
    - Mixed
  - Access pattern
    - Sequential
    - Random
    - Strided
      - Stride length
  - Operation Count
    - Examples:
      - 1000 iterations, 10 requests, 1 access-per-request
      - 10 iterations, 1000 requests, 2 accesses-per-request
      - 505 iterations, 505 requests, 1 access-per-request
  - Size of I/O request (input as lower and upper bound)
    - 4 to 8K
    - 64K to 512K
    - 1M to 16M
    - Distribution (future work)
      - Uniform
      - Normal
      - Gamma
      - Random
  - Scaling
    - \# of nodes
      - 1, 2, 4, 8
    - \# of processors per node
      - 4, 8, 16, 32
  - Compute
    - Inter-Iteration compute
    - Inter-Operation compute
  - Compute Intensity
    - No compute
    - Sleep-simulated ("busy waiting")
      - Sleep duration
    - Traditional
      - Matrix Multiplication
- Output log file (.csv)

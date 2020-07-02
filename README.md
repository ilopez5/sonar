[![Generic badge](https://img.shields.io/badge/Stage-Development-blue.svg)](https://shields.io/)

# SONAR

## A custom benchmark for I/O systems

The purpose of this benchmark is to simulate different I/O patterns with
configurable parameters in order to test throughput for I/O systems. Initial
development of this Sonar is to test the
[LABIOS](https://dl.acm.org/doi/abs/10.1145/3307681.3325405) I/O system.

## Configurable parameters:
- \# of I/O phases
- Size of I/O request
- Number of I/O accesses (per request)
- Workload type:
  - Read-only
  - Write-only
  - Read with compute
  - Write with compute
  - Mixed w/o compute
  - Mixed with compute
- Access Pattern:
  - Sequential
  - Random
  - Strided
    - Stride length
- Compute Intensity
  - None
  - Sleep-simulated
    - Sleep duration
  - Simple
  - Intense
- Output log file
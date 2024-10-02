# Pi Calculation Program

## Overview

This program is part of **Practice 2** for Operating Systems and calculates an approximation of Pi using numerical integration with the function `f(x) = sqrt(1 - x^2)`. The calculation is divided among multiple processes to parallelize the work, utilizing shared memory and semaphores for synchronization.

## Requirements

- C standard library headers
  - `<stdlib.h>`
  - `<stdio.h>`
  - `<unistd.h>`
  - `<math.h>`
  - `<sys/time.h>`
  - `<sys/ipc.h>`
  - `<sys/shm.h>`
  - `<sys/wait.h>`
  - `<semaphore.h>`
  - `<fcntl.h>`

## How It Works

1. **Function `f(x)`**: This function calculates the height of the rectangle under the curve for a given x using `f(x) = sqrt(1 - x^2)`.
   
2. **Function `calculate_pi()`**: It calculates the area of each rectangle for a portion of the total subintervals, which is used to approximate Pi.

3. **Main Process**:
   - Accepts two arguments: the number of subintervals and the number of processes.
   - Creates shared memory to store the Pi approximation.
   - Forks multiple child processes to calculate portions of the total area in parallel.
   - Uses a semaphore to ensure only one process at a time updates the shared Pi value.
   - Combines the results from all processes and prints the final value of Pi multiplied by 4 (to account for the quarter circle).
   
4. **Timing**: The program measures and prints the total time taken for the calculation.

## Compilation

To compile the program, use the following command:

```bash
make
```

## Usage

To run the program, provide the number of subintervals and the number of processes:

```bash
./picalc <number_of_subintervals> <number_of_processes>
```

Example:

```bash
./picalc 10000000 4
```

This will calculate Pi using 10,000,000 subintervals and 4 processes.

## Output

The program outputs the calculated value of Pi. It also prints the time it took for the entire process to complete.

## Author

Abraham de León Gutiérrez
# SetaLab v1.1

A C++11 console prototype for simulated laboratory instrument readings. **All data is synthetic.**

## Run in Dev-C++

1. Extract this ZIP into a normal folder (do not run from inside the ZIP).
2. Open `main.cpp` in Dev-C++.
3. Select a compiler supporting C++11 or later and press **F11** (Compile & Run).
4. Read the built-in guide, enter an experiment ID such as `EXP001` and sample such as `Diesel`.
5. Select **1** and enter `10` to simulate readings; **2** to inspect; **3** for statistics; **4** for sorted view; **5** to export `results.csv`; **6** for help; **0** to exit.

CSV files are written relative to the program's working directory, which depends on how Dev-C++ launches the program. A matching existing file will be overwritten.

## Technical overview

C++11, STL vectors, object-oriented design, random-number simulation, basic descriptive statistics, alert thresholds, CSV export, and input validation.

The demonstration alert thresholds are not certified operating limits for any real sample. This project is not suitable for instrument control or safety decisions.

# SetaLab — Industrial Instrument Simulator

A C++ console prototype for simulated laboratory instrument monitoring. Designed as the first stage of a future Qt desktop application.

## Features
- Simulated temperature and density measurements (synthetic data)
- Out-of-range alerts with illustrative thresholds
- Average, minimum and maximum statistics
- Sorting by temperature
- CSV export with escaped text fields
- Input validation and menu-driven interaction

## Build
Requires a C++11-compatible compiler. Open `main.cpp` in Dev-C++, select a compiler supporting C++11 or later, and choose **Compile & Run**.

Or from a terminal:

```bash
g++ -std=c++11 -Wall -Wextra -pedantic main.cpp -o setalab
./setalab
```

On Windows, run `setalab.exe`.

## Notes
Measurements are randomly generated and are **not** obtained from physical sensors. Limits are demonstration values, not petroleum test specifications. Running a new simulation replaces the previous set of readings. CSV export saves data to the selected file path.

## Roadmap
- Separate core logic from the console UI
- Add Qt 6 Widgets desktop interface
- Add configurable sensor limits and live charts
- Add SQLite storage, tests, and mock serial communication

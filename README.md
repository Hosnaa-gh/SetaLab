# SetaLab — Industrial Instrument Simulator

A C++11 command-line application that simulates laboratory instrument readings for **temperature** and **density**. It provides experiment controls, monitoring, simulated alarms, summary statistics and CSV reporting. Built as a personal software engineering portfolio project.

> **Simulation only:** SetaLab is not connected to physical hardware. The measurement values and alert thresholds are illustrative and must not be used to assess real petroleum samples.

## Features

- Create an experiment with an ID and sample name.
- Start, pause, resume and stop measurement collection.
- Generate individual readings or an automatic batch of simulated readings.
- View a console dashboard, recent measurement and instrument status.
- Inspect readings, sort by descending temperature and calculate min/mean/max.
- Highlight readings outside demonstration thresholds: 20–30 °C and 842–858 kg/m³.
- Export CSV files and append snapshots to a separate history file.
- Confirm before discarding readings or overwriting existing CSV exports.
- Display a colour-coded user guide when supported by the terminal.

## Technology and design

- **Language:** C++11; standard library (STL).
- **Compiler:** GCC/MinGW (Dev-C++ on Windows); no Qt dependency.
- **`SensorSimulator`:** uses `std::mt19937` and normal distributions to generate readings.
- **`Experiment`:** manages readings, experiment state, thresholds, statistics and CSV export.
- **CLI:** validates user input, presents menus and provides colour-coded output.

The application deliberately retains the original single-file C++ structure for easy compilation in Dev-C++. It is a console prototype, not an embedded or real-time system.

## Build and run

**Windows / Dev-C++:** Open `main.cpp`, ensure C++11 is enabled and choose **Compile & Run (F11)**.

**Terminal (GCC):**

```bash
g++ -std=c++11 -Wall -Wextra -pedantic main.cpp -o setalab
./setalab
```

On Windows the resulting executable may be `setalab.exe`.

## Quick start

1. Enter experiment ID `EXP001` and sample `Diesel`.
2. Select **1** and choose a target of `10` readings.
3. Select **2** to take individual readings, or **3** to generate the remaining readings.
4. Select **6** for dashboard, **7** for readings, or **8** for summary.
5. Select **10** to export to `results.csv`; select **11** to append to `experiment_history.csv`.

## Menu reference

| Option | Action |
| --- | --- |
| 1 | Start/restart experiment |
| 2 | Take next measurement |
| 3 | Auto-run remaining measurements |
| 4 | Pause/resume experiment |
| 5 | Stop experiment |
| 6 | Instrument dashboard |
| 7 | View readings |
| 8 | Summary and alerts |
| 9 | Sort readings by temperature |
| 10 | Export CSV |
| 11 | Save snapshot to history |
| 12 | View saved history |
| 13 | Create new experiment |
| 14 | Help / user guide |
| 0 | Exit |

## Tests

Integration tests compile the C++ program and exercise its interactive CLI, CSV export, state transitions, history and overwrite protection. Requires **Python 3** and **g++**:

```bash
python -m unittest discover -s tests -v
```

## Screenshots

Add your own screenshots from a successful Dev-C++ run to the `screenshots/` directory, then reference them here. Suggested captures: main menu, instrument dashboard, measurement summary and CSV opened in Excel.

## Known limitations

- **Auto-run is blocking:** Pause/resume works between individual measurements, not while an automatic batch is running.
- **History is append-only:** Saving the same snapshot repeatedly creates duplicate entries. The viewer displays the last 15 raw CSV lines, not a grouped experiment browser.
- Randomly simulated measurements are not calibrated sensor data.
- CSV export stores measurements but not timestamp or instrument metadata.
- This is a console prototype without a Qt GUI, hardware connectivity or embedded-device control.

## Next steps

Potential future work: structured experiment history, automated unit tests of pure calculation functions, timestamps, asynchronous acquisition and a Qt interface.

## License

No license has been selected yet. Add one if you want to specify reuse permissions.

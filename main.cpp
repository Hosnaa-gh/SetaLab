#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <algorithm>
#include <random>
#include <fstream>
#include <sstream>
#include <limits>
#include <ctime>
#include <cmath>
using namespace std;

struct Measurement {
    int index;
    double temperature;
    double density;
};

struct Statistics {
    double average, minimum, maximum;
};

class SensorSimulator {
private:
    mt19937 engine;
    normal_distribution<double> temperatureNoise;
    normal_distribution<double> densityNoise;
public:
    SensorSimulator() : engine(random_device{}()),
        temperatureNoise(0.0, 2.5), densityNoise(0.0, 4.0) {}

    Measurement read(int index) {
        Measurement m;
        m.index = index;
        m.temperature = 25.0 + temperatureNoise(engine);
        m.density = 850.0 + densityNoise(engine);
        return m;
    }
};

class Experiment {
private:
    string id, sample;
    vector<Measurement> readings;
    double temperatureMin, temperatureMax, densityMin, densityMax;
public:
    Experiment(const string& experimentId, const string& sampleName)
        : id(experimentId), sample(sampleName),
          temperatureMin(20.0), temperatureMax(30.0),
          densityMin(842.0), densityMax(858.0) {}

    void run(int count, SensorSimulator& sensor) {
        readings.clear();
        for (int i = 1; i <= count; ++i) readings.push_back(sensor.read(i));
    }

    bool hasData() const { return !readings.empty(); }
    bool isAlert(const Measurement& m) const {
        return m.temperature < temperatureMin || m.temperature > temperatureMax ||
               m.density < densityMin || m.density > densityMax;
    }
    int alertCount() const {
        int count = 0;
        for (size_t i = 0; i < readings.size(); ++i)
            if (isAlert(readings[i])) ++count;
        return count;
    }
    Statistics temperatureStats() const {
        Statistics s = {0, 0, 0};
        if (!hasData()) return s;
        s.minimum = s.maximum = readings[0].temperature;
        for (size_t i = 0; i < readings.size(); ++i) {
            double v = readings[i].temperature;
            s.average += v;
            s.minimum = min(s.minimum, v);
            s.maximum = max(s.maximum, v);
        }
        s.average /= readings.size();
        return s;
    }
    Statistics densityStats() const {
        Statistics s = {0, 0, 0};
        if (!hasData()) return s;
        s.minimum = s.maximum = readings[0].density;
        for (size_t i = 0; i < readings.size(); ++i) {
            double v = readings[i].density;
            s.average += v;
            s.minimum = min(s.minimum, v);
            s.maximum = max(s.maximum, v);
        }
        s.average /= readings.size();
        return s;
    }
    void printReadings(bool sortByTemperature = false) const {
        if (!hasData()) { cout << "No measurements yet. Run an experiment first.\n"; return; }
        vector<Measurement> displayed = readings;
        if (sortByTemperature)
            sort(displayed.begin(), displayed.end(), [](const Measurement& a, const Measurement& b) {
                return a.temperature > b.temperature;
            });
        cout << "\nExperiment: " << id << " | Sample: " << sample << "\n";
        cout << left << setw(10) << "Reading" << setw(18) << "Temp (C)"
             << setw(22) << "Density (kg/m3)" << "Status\n";
        cout << string(62, '-') << "\n";
        for (size_t i = 0; i < displayed.size(); ++i) {
            const Measurement& m = displayed[i];
            cout << left << setw(10) << m.index << setw(18) << fixed << setprecision(2)
                 << m.temperature << setw(22) << m.density
                 << (isAlert(m) ? "ALERT" : "OK") << "\n";
        }
    }
    void printSummary() const {
        if (!hasData()) { cout << "No measurements yet. Run an experiment first.\n"; return; }
        Statistics t = temperatureStats(), d = densityStats();
        cout << fixed << setprecision(2)
             << "\n--- Experiment Summary ---\n"
             << "ID: " << id << " | Sample: " << sample << "\n"
             << "Measurements: " << readings.size() << "\n"
             << "Temperature (C): average=" << t.average << ", min=" << t.minimum << ", max=" << t.maximum << "\n"
             << "Density (kg/m3): average=" << d.average << ", min=" << d.minimum << ", max=" << d.maximum << "\n"
             << "Allowed temperature: " << temperatureMin << " to " << temperatureMax << " C\n"
             << "Allowed density: " << densityMin << " to " << densityMax << " kg/m3\n"
             << "Out-of-range readings: " << alertCount() << "\n";
    }
    static string csvEscape(const string& text) {
        string escaped = "\"";
        for (size_t i = 0; i < text.size(); ++i) {
            if (text[i] == '"') escaped += "\"\"";
            else escaped += text[i];
        }
        return escaped + "\"";
    }
    bool saveCsv(const string& path) const {
        if (!hasData()) return false;
        ofstream out(path.c_str());
        if (!out) return false;
        out << "experiment_id,sample,reading,temperature_c,density_kg_m3,status\n";
        out << fixed << setprecision(2);
        for (size_t i = 0; i < readings.size(); ++i) {
            const Measurement& m = readings[i];
            out << csvEscape(id) << ',' << csvEscape(sample) << ',' << m.index << ','
                << m.temperature << ',' << m.density << ',' << (isAlert(m) ? "ALERT" : "OK") << '\n';
        }
        out.close();
        return out.good();
    }
};

int readInt(const string& prompt, int minimum, int maximum) {
    while (true) {
        cout << prompt;
        string line;
        if (!getline(cin, line)) return -1;
        istringstream input(line);
        int value;
        char extra;
        if ((input >> value) && !(input >> extra) && value >= minimum && value <= maximum)
            return value;
        cout << "Please enter a whole number from " << minimum << " to " << maximum << ".\n";
    }
}

string readText(const string& prompt) {
    string value;
    while (true) {
        cout << prompt;
        if (!getline(cin, value)) return "";
        if (value.find_first_not_of(" \t\r\n") != string::npos) return value;
        cout << "This field cannot be empty.\n";
    }
}

void showGuide() {
    cout << "\n========== USER GUIDE ==========\n"
         << "SetaLab simulates laboratory temperature and density readings.\n"
         << "No physical instrument is connected.\n\n"
         << "QUICK START\n"
         << "  1. Enter an experiment ID, e.g. EXP001.\n"
         << "  2. Enter a sample name, e.g. Diesel.\n"
         << "  3. Select option 1 and enter 10 for ten readings.\n"
         << "  4. Select option 2 to inspect individual readings.\n"
         << "  5. Select option 3 to view statistics and alerts.\n"
         << "  6. Select option 4 to view readings sorted by temperature.\n"
         << "  7. Select option 5 and enter results.csv to export.\n\n"
         << "MENU REFERENCE\n"
         << "  1: Generate 1-1000 simulated readings (replaces previous run).\n"
         << "  2: Show reading number, temperature, density, and status.\n"
         << "  3: Show averages, minimums, maximums, and alert count.\n"
         << "  4: Display descending temperature order without changing stored data.\n"
         << "  5: Save the current readings as a CSV file.\n"
         << "  6: Show this guide again.\n"
         << "  0: Exit the program.\n\n"
         << "ALERT RULES (SIMULATION ONLY)\n"
         << "  Temperature outside 20-30 C or density outside 842-858 kg/m3\n"
         << "  is marked ALERT. These are demonstration thresholds, not\n"
         << "  validated limits for any real petroleum sample.\n\n"
         << "CSV TIPS\n"
         << "  Enter a filename ending in .csv, for example results.csv.\n"
         << "  The file is written to the current working folder unless\n"
         << "  you provide a path. Existing files with that name are replaced.\n"
         << "  Open the exported file with Excel or a text editor.\n"
         << "================================\n";
}

int main() {
    cout << "==============================================\n"
         << " SetaLab | Industrial Instrument Simulator\n"
         << " C++ Console Prototype v1.1\n"
         << " SIMULATED DATA ONLY - NOT A REAL INSTRUMENT\n"
         << "==============================================\n";
    cout << "Tip: Enter 6 in the menu at any time to open the user guide.\n";
    showGuide();
    cout << "\n--- NEW EXPERIMENT ---\n";
    string id = readText("Experiment ID (e.g. EXP001): ");
    if (id.empty()) return 0;
    string sample = readText("Sample name (e.g. Diesel): ");
    if (sample.empty()) return 0;
    Experiment experiment(id, sample);
    SensorSimulator sensor;
    while (true) {
        cout << "\n--- MAIN MENU ---\n"
             << "1. Run simulated measurements\n"
             << "2. View readings\n"
             << "3. View summary and alerts\n"
             << "4. Sort readings by temperature (high to low)\n"
             << "5. Export results to CSV\n"
             << "6. Help / User Guide\n"
             << "0. Exit\n";
        int choice = readInt("Choose an option (6 = Help): ", 0, 6);
        if (choice == -1 || choice == 0) { cout << "Goodbye!\n"; break; }
        if (choice == 6) { showGuide(); continue; }
        if (choice == 1) {
            int count = readInt("Number of measurements (1-1000): ", 1, 1000);
            if (count == -1) break;
            experiment.run(count, sensor);
            cout << "Completed " << count << " simulated measurements.\n"
                 << "Next: choose 2 to view readings or 3 for summary and alerts.\n";
        } else if (choice == 2) experiment.printReadings();
        else if (choice == 3) experiment.printSummary();
        else if (choice == 4) experiment.printReadings(true);
        else if (choice == 5) {
            if (!experiment.hasData()) { cout << "Run an experiment before exporting.\n"; continue; }
            cout << "Tip: use results.csv; existing files with the same name will be overwritten.\n";
            string path = readText("CSV filename (e.g. results.csv): ");
            if (path.empty()) break;
            if (path.size() < 5 || path.substr(path.size() - 4) != ".csv") {
                cout << "Please use a filename ending in .csv. Export cancelled.\n";
                continue;
            }
            cout << (experiment.saveCsv(path) ? "CSV saved successfully.\n" : "Could not save CSV. Check the path and permissions.\n");
        }
    }
    return 0;
}

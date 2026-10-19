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
#include <chrono>
#include <thread>
#include <cctype>
#ifdef _WIN32
#include <windows.h>
#endif
using namespace std;

namespace Color {
    bool enabled = true;
    const char* reset = "\033[0m";
    const char* cyan = "\033[36m";
    const char* green = "\033[32m";
    const char* magenta = "\033[35m";
    const char* blue = "\033[34m";
    const char* yellow = "\033[33m";
    const char* red = "\033[31m";
    const char* bold = "\033[1m";
    void initialize() {
#ifdef _WIN32
        HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD mode = 0;
        if (handle == INVALID_HANDLE_VALUE || !GetConsoleMode(handle, &mode) ||
            !SetConsoleMode(handle, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING))
            enabled = false;
#endif
    }
    const char* use(const char* code) { return enabled ? code : ""; }
}


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
    int targetCount;
    bool active, paused, finished;
public:
    Experiment(const string& experimentId, const string& sampleName)
        : id(experimentId), sample(sampleName),
          temperatureMin(20.0), temperatureMax(30.0),
          densityMin(842.0), densityMax(858.0),
          targetCount(0), active(false), paused(false), finished(false) {}

    void run(int count, SensorSimulator& sensor) {
        start(count);
        while (active) takeReading(sensor);
    }

    void start(int count) {
        readings.clear();
        targetCount = count;
        active = true;
        paused = false;
        finished = false;
    }
    bool takeReading(SensorSimulator& sensor) {
        if (!active || paused) return false;
        readings.push_back(sensor.read(static_cast<int>(readings.size()) + 1));
        if (static_cast<int>(readings.size()) >= targetCount) {
            active = false;
            finished = true;
        }
        return true;
    }
    void pause() { if (active) paused = true; }
    void resume() { if (active) paused = false; }
    void stop() { active = false; paused = false; finished = true; }
    bool isActive() const { return active; }
    bool isPaused() const { return paused; }
    bool isFinished() const { return finished; }
    int progress() const { return static_cast<int>(readings.size()); }
    int target() const { return targetCount; }
    const Measurement* latest() const { return readings.empty() ? NULL : &readings.back(); }
    const string& getId() const { return id; }
    const string& getSample() const { return sample; }
    const vector<Measurement>& getReadings() const { return readings; }
    string alertReason(const Measurement& m) const {
        string reason;
        if (m.temperature < temperatureMin) reason += "Temperature LOW; ";
        if (m.temperature > temperatureMax) reason += "Temperature HIGH; ";
        if (m.density < densityMin) reason += "Density LOW; ";
        if (m.density > densityMax) reason += "Density HIGH; ";
        return reason.empty() ? "Within simulated limits" : reason;
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
        if (!hasData()) { cout << Color::use(Color::yellow) << "No measurements yet. Run an experiment first.\n" << Color::use(Color::reset); return; }
        vector<Measurement> displayed = readings;
        if (sortByTemperature)
            sort(displayed.begin(), displayed.end(), [](const Measurement& a, const Measurement& b) {
                return a.temperature > b.temperature;
            });
        cout << Color::use(Color::cyan) << "\nExperiment: " << id << " | Sample: " << sample << "\n";
        cout << left << setw(10) << "Reading" << setw(18) << "Temp (C)"
             << setw(22) << "Density (kg/m3)" << "Status\n" << Color::use(Color::reset);
        cout << string(62, '-') << "\n";
        for (size_t i = 0; i < displayed.size(); ++i) {
            const Measurement& m = displayed[i];
            cout << left << setw(10) << m.index << setw(18) << fixed << setprecision(2)
                 << m.temperature << setw(22) << m.density
                 << Color::use(isAlert(m) ? Color::red : Color::green)
                 << (isAlert(m) ? "ALERT" : "OK") << Color::use(Color::reset)
                  << "  " << alertReason(m) << "\n";
        }
    }
    void printSummary() const {
        if (!hasData()) { cout << Color::use(Color::yellow) << "No measurements yet. Run an experiment first.\n" << Color::use(Color::reset); return; }
        Statistics t = temperatureStats(), d = densityStats();
        cout << fixed << setprecision(2)
             << Color::use(Color::cyan) << "\n--- Experiment Summary ---\n" << Color::use(Color::reset)
             << "ID: " << id << " | Sample: " << sample << "\n"
             << "Measurements: " << readings.size() << "\n"
             << "Temperature (C): average=" << t.average << ", min=" << t.minimum << ", max=" << t.maximum << "\n"
             << "Density (kg/m3): average=" << d.average << ", min=" << d.minimum << ", max=" << d.maximum << "\n"
             << "Allowed temperature: " << temperatureMin << " to " << temperatureMax << " C\n"
             << "Allowed density: " << densityMin << " to " << densityMax << " kg/m3\n"
             << "Out-of-range readings: " << Color::use(alertCount() ? Color::red : Color::green)
             << alertCount() << Color::use(Color::reset) << "\n";
    }
public:
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
        cout << Color::use(Color::red) << "Please enter a whole number from " << minimum << " to " << maximum << ".\n" << Color::use(Color::reset);
    }
}

string readText(const string& prompt) {
    string value;
    while (true) {
        cout << prompt;
        if (!getline(cin, value)) return "";
        if (value.find_first_not_of(" \t\r\n") != string::npos) return value;
        cout << Color::use(Color::red) << "This field cannot be empty.\n" << Color::use(Color::reset);
    }
}

void showGuide() {
    cout << Color::use(Color::cyan) << "\n========== USER GUIDE ==========\n" << Color::use(Color::reset)
         << "Simulated temperature and density; no hardware connected.\n\n"
         << Color::use(Color::bold) << Color::use(Color::green) << "QUICK START\n" << Color::use(Color::reset)
         << "  1. Create an experiment (EXP001, Diesel).\n"
         << "  2. Choose 1 to set a measurement target.\n"
         << "  3. Choose 2 to take one reading, or 3 for an automatic batch.\n"
         << "  4. Choose 4 to pause/resume; 5 to stop.\n"
         << "  5. Choose 6 for the dashboard, 7 for readings, 8 for summary.\n\n"
         << Color::use(Color::bold) << Color::use(Color::blue) << "MENU REFERENCE\n" << Color::use(Color::reset)
         << "  1 Start/restart | 2 Step | 3 Auto-run | 4 Pause/resume | 5 Stop\n"
         << "  6 Dashboard | 7 Readings | 8 Summary | 9 Sort | 10 Export CSV\n"
         << "  11 Save to history | 12 View history | 13 New experiment\n"
         << "  14 Help | 0 Exit\n\n"
         << Color::use(Color::bold) << Color::use(Color::red) << "ALERT RULES (DEMO ONLY)\n" << Color::use(Color::reset)
         << "  Temperature outside 20-30 C or density outside 842-858 kg/m3\n"
         << "  triggers an alert. These limits are NOT validated for real samples.\n\n"
         << Color::use(Color::bold) << Color::use(Color::magenta) << "CSV TIPS\n" << Color::use(Color::reset)
         << "  Enter a .csv filename such as results.csv. Existing files are\n"
         << "  overwritten. The file is saved in the working folder unless\n"
         << "  you enter an absolute path. History uses a separate CSV file.\n"
         << "================================\n";
}

void showDashboard(const Experiment& experiment) {
    cout << Color::use(Color::cyan) << "\n========== INSTRUMENT DASHBOARD ==========\n"
         << Color::use(Color::reset)
         << "Experiment: " << experiment.getId() << " | Sample: " << experiment.getSample() << "\n";
    string state = experiment.isPaused() ? "PAUSED" :
        experiment.isActive() ? "RUNNING" : experiment.isFinished() ? "STOPPED/COMPLETE" : "READY";
    cout << "State: " << state << " | Progress: " << experiment.progress()
         << "/" << experiment.target() << "\n";
    const Measurement* m = experiment.latest();
    if (m) {
        cout << fixed << setprecision(2) << "Latest: " << m->temperature << " C, "
             << m->density << " kg/m3\n";
        cout << Color::use(experiment.isAlert(*m) ? Color::red : Color::green)
             << experiment.alertReason(*m) << Color::use(Color::reset) << "\n";
    } else cout << "No measurements yet.\n";
    cout << "Alerts so far: " << experiment.alertCount() << "\n";
}

void showNewReading(const Experiment& experiment) {
    const Measurement* m = experiment.latest();
    if (!m) return;
    cout << fixed << setprecision(2) << "Reading #" << m->index << ": "
         << m->temperature << " C | " << m->density << " kg/m3 | "
         << Color::use(experiment.isAlert(*m) ? Color::red : Color::green)
         << experiment.alertReason(*m) << Color::use(Color::reset) << "\n";
}

// Append-only CSV history: each snapshot is identified by its experiment ID.
bool saveHistory(const Experiment& experiment, const string& filename) {
    if (!experiment.hasData()) return false;
    bool header = true;
    { ifstream existing(filename.c_str(), ios::binary | ios::ate);
      if (existing && existing.tellg() > 0) header = false; }
    ofstream out(filename.c_str(), ios::app);
    if (!out) return false;
    if (header) out << "experiment_id,sample,reading,temperature_c,density_kg_m3,status\n";
    out << fixed << setprecision(2);
    const vector<Measurement>& readings = experiment.getReadings();
    for (size_t i = 0; i < readings.size(); ++i) {
        const Measurement& m = readings[i];
        out << Experiment::csvEscape(experiment.getId()) << ','
            << Experiment::csvEscape(experiment.getSample()) << ','
            << m.index << ',' << m.temperature << ',' << m.density << ','
            << (experiment.isAlert(m) ? "ALERT" : "OK") << '\n';
    }
    out.flush();
    return out.good();
}

void viewHistory(const string& filename) {
    ifstream in(filename.c_str());
    if (!in) { cout << "No saved history yet.\n"; return; }
    string line;
    cout << Color::use(Color::cyan) << "\n--- SAVED HISTORY (latest 15 lines) ---\n"
         << Color::use(Color::reset);
    vector<string> lines;
    while (getline(in, line)) {
        if (lines.size() == 15) lines.erase(lines.begin());
        lines.push_back(line);
    }
    for (size_t i = 0; i < lines.size(); ++i) cout << lines[i] << '\n';
    if (lines.empty()) cout << "History file is empty.\n";
}

int main() {
    Color::initialize();
    cout << Color::use(Color::cyan)
         << "==============================================\n"
         << " SetaLab | Industrial Instrument Simulator\n"
         << " C++ Console Prototype v2.0\n"
         << " SIMULATED DATA ONLY - NOT A REAL INSTRUMENT\n"
         << "==============================================\n" << Color::use(Color::reset);
    showGuide();
    string id = readText("Experiment ID (e.g. EXP001): ");
    if (id.empty()) return 0;
    string sample = readText("Sample name (e.g. Diesel): ");
    if (sample.empty()) return 0;
    Experiment experiment(id, sample);
    SensorSimulator sensor;
    const string historyFile = "experiment_history.csv";
    while (true) {
        cout << Color::use(Color::cyan) << "\n--- MAIN MENU ---\n" << Color::use(Color::reset)
             << " 1. Start/restart experiment\n 2. Take next measurement\n"
             << " 3. Auto-run remaining measurements\n 4. Pause / Resume\n"
             << " 5. Stop experiment\n 6. Instrument dashboard\n"
             << " 7. View readings\n 8. Summary and alerts\n"
             << " 9. Sort by temperature\n10. Export CSV\n"
             << "11. Save snapshot to history\n12. View saved history\n"
             << "13. New experiment\n14. Help / User Guide\n 0. Exit\n";
        int choice = readInt("Choose an option: ", 0, 14);
        if (choice == -1 || choice == 0) { cout << "Goodbye!\n"; break; }
        if (choice == 14) { showGuide(); continue; }
        if (choice == 1) {
            if (experiment.hasData())
                cout << Color::use(Color::yellow)
                     << "WARNING: Restart clears current readings. Export/save first if needed.\n"
                     << Color::use(Color::reset);
            int count = readInt("Target number of measurements (1-1000): ", 1, 1000);
            if (count == -1) break;
            experiment.start(count);
            cout << Color::use(Color::green) << "Experiment ready. Choose 2 or 3.\n"
                 << Color::use(Color::reset);
        } else if (choice == 2) {
            if (experiment.takeReading(sensor)) showNewReading(experiment);
            else cout << "Cannot take a reading. Start or resume the experiment first.\n";
        } else if (choice == 3) {
            if (!experiment.isActive() || experiment.isPaused()) {
                cout << "Start or resume an experiment first.\n";
            } else {
                cout << "Running simulated readings...\n";
                while (experiment.isActive() && !experiment.isPaused()) {
                    experiment.takeReading(sensor);
                    showNewReading(experiment);
                    std::this_thread::sleep_for(std::chrono::milliseconds(120));
                }
                cout << "Batch completed.\n";
            }
        } else if (choice == 4) {
            if (!experiment.isActive()) cout << "No active experiment.\n";
            else if (experiment.isPaused()) { experiment.resume(); cout << "Resumed.\n"; }
            else { experiment.pause(); cout << "Paused.\n"; }
        } else if (choice == 5) {
            if (!experiment.isActive()) cout << "No active experiment.\n";
            else { experiment.stop(); cout << "Experiment stopped; collected data retained.\n"; }
        } else if (choice == 6) showDashboard(experiment);
        else if (choice == 7) experiment.printReadings();
        else if (choice == 8) experiment.printSummary();
        else if (choice == 9) experiment.printReadings(true);
        else if (choice == 10) {
            if (!experiment.hasData()) { cout << "No readings to export.\n"; continue; }
            string path = readText("CSV filename (e.g. results.csv): ");
            if (path.empty()) break;
            if (path.size() < 5 || path.substr(path.size()-4) != ".csv") {
                cout << "Filename must end with .csv.\n"; continue;
            }
            cout << (experiment.saveCsv(path) ? "CSV saved successfully.\n" : "CSV save failed.\n");
        } else if (choice == 11) {
            cout << (saveHistory(experiment, historyFile) ? "Snapshot appended to experiment_history.csv.\n"
                      : "No data or could not save history.\n");
        } else if (choice == 12) viewHistory(historyFile);
        else if (choice == 13) {
            if (experiment.hasData()) cout << "Remember to export or save your current experiment first.\n";
            string newId = readText("New experiment ID: ");
            if (newId.empty()) break;
            string newSample = readText("New sample name: ");
            if (newSample.empty()) break;
            experiment = Experiment(newId, newSample);
            cout << "New experiment created.\n";
        }
    }
    return 0;
}

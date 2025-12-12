#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <string>
#include <iomanip>
#include <atomic>
#include <sstream>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

using namespace std;

// COLORS
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define CYAN    "\033[36m"
#define MAGENTA "\033[35m"
#define WHITE   "\033[37m"

struct ProcessInfo {
    int pid;
    string name;
    double cpuUsage;
    double memUsage;
};

atomic<bool> running(true);
vector<ProcessInfo> globalProcesses;

// helper: trim spaces from both ends
static inline string trim(const string &s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

// Clear screen
void clearScreen() {
    cout << "\033[2J\033[1;1H";
}

// Header
void displayHeader() {
    cout << CYAN << "==================== "
         << MAGENTA << "Real-Time Monitoring Dashboard"
         << CYAN << " ====================" 
         << RESET << "\n\n";
}

// Progress bar
string progressBar(double percent) {
    int totalBars = 30;
    int filled = static_cast<int>((percent / 100.0) * totalBars);
    if (filled < 0) filled = 0;
    if (filled > totalBars) filled = totalBars;

    string bar = "[";
    bar.append(filled, '#');
    bar.append(totalBars - filled, '-');
    bar += "]";

    return bar;
}

void displayCPU(double cpuUsage) {
    cout << YELLOW << "CPU Usage: " << RESET 
         << fixed << setprecision(2) << cpuUsage << "% "
         << GREEN << progressBar(cpuUsage) << RESET << "\n";
}

void displayMemory(double memUsage) {
    cout << YELLOW << "Memory Usage: " << RESET 
         << fixed << setprecision(2) << memUsage << "% "
         << BLUE << progressBar(memUsage) << RESET << "\n\n";
}

// Process table
void displayProcesses(const vector<ProcessInfo>& processes) {
    cout << CYAN << left
         << setw(10) << "PID"
         << setw(20) << "Process Name"
         << setw(10) << "CPU%"
         << setw(10) << "MEM%"
         << setw(20) << "STATUS"
         << RESET << "\n";

    cout << WHITE << "---------------------------------------------------------------------\n" << RESET;

    for (auto &p : processes) {
        string status;
        string color = RESET;

        if (p.cpuUsage > 70.0 || p.memUsage > 70.0) {
            status = "⚠ HIGH USAGE";
            color = RED;
        } else {
            status = "OK";
            color = GREEN;
        }

        cout << color << left
             << setw(10) << p.pid
             << setw(20) << p.name
             << setw(10) << fixed << setprecision(2) << p.cpuUsage
             << setw(10) << fixed << setprecision(2) << p.memUsage
             << setw(20) << status
             << RESET << "\n";
    }
}

vector<ProcessInfo> getProcesses() {
    vector<ProcessInfo> processes;
    for (int i = 0; i < 5; i++) {
        processes.push_back({
            1000 + i,
            "Process_" + to_string(i + 1),
            10.0 + i * 20,   // create some high usage examples
            18.0 + i * 25
        });
    }
    globalProcesses = processes;
    return processes;
}

double getCPUUsage() { return 45.5; }
double getMemoryUsage() { return 63.7; }

void inputListener() {
    string line;
    while (running) {
        if (!std::getline(cin, line)) {
     
            running = false;
            break;
        }
        string cmdline = trim(line);
        if (cmdline.empty()) continue;

        string lower = cmdline;
        transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

        if (lower == "stop") {
            running = false;
            break;
        }

        stringstream ss(cmdline);
        string cmd;
        ss >> cmd;
        string cmdLower = cmd;
        transform(cmdLower.begin(), cmdLower.end(), cmdLower.begin(), ::tolower);

        if (cmdLower == "kill") {
            int pid;
            if (ss >> pid) {
                bool removed = false;
                for (auto it = globalProcesses.begin(); it != globalProcesses.end(); ++it) {
                    if (it->pid == pid) {
                        globalProcesses.erase(it);
                        removed = true;
                        break;
                    }
                }
                if (removed) {
                    cout << RED << "Simulated: Process " << pid << " terminated.\n" << RESET;
                } else {
                    cout << YELLOW << "Process " << pid << " not found.\n" << RESET;
                }
            } else {
                cout << YELLOW << "Usage: kill <pid>\n" << RESET;
            }
        } else if (cmdLower == "info") {
            int pid;
            if (ss >> pid) {
                bool found = false;
                for (auto &p : globalProcesses) {
                    if (p.pid == pid) {
                        cout << CYAN << "\nProcess Info:\n";
                        cout << " PID   : " << p.pid << "\n";
                        cout << " Name  : " << p.name << "\n";
                        cout << " CPU%  : " << fixed << setprecision(2) << p.cpuUsage << "%\n";
                        cout << " MEM%  : " << fixed << setprecision(2) << p.memUsage << "%\n" << RESET;
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    cout << YELLOW << "Process " << pid << " not found.\n" << RESET;
                }
            } else {
                cout << YELLOW << "Usage: info <pid>\n" << RESET;
            }
        } else {
            cout << YELLOW << "Unknown command: '" << cmdline << "'. Available: stop, kill <pid>, info <pid>\n" << RESET;
        }
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    thread listener(inputListener);

    while (running) {
        clearScreen();
        displayHeader();

        displayCPU(getCPUUsage());
        displayMemory(getMemoryUsage());

        vector<ProcessInfo> processes = getProcesses();
        
        if (!globalProcesses.empty()) {
            processes = globalProcesses;
        }

        displayProcesses(processes);

        cout << "\n" << MAGENTA
             << "Type 'stop' to exit. Other commands: kill <pid>, info <pid>\n"
             << RESET << endl;
        for (int i = 0; i < 10 && running; ++i) {
            this_thread::sleep_for(chrono::milliseconds(100));
        }
    }
    
    if (listener.joinable()) listener.join();

    cout << GREEN << "\nDashboard stopped successfully.\n" << RESET;

    return 0;
}

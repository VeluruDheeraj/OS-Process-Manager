
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <queue>
#include <stack>
#include <memory>
#include <algorithm> // <-- Needed for std::remove

using namespace std;

class Process {
public:
    int pid;
    string name;
    Process* parent;
    vector<Process*> children;
    stack<string> callStack;

    Process(int pid, const string& name, Process* parent = nullptr)
        : pid(pid), name(name), parent(parent) {}

    void addChild(Process* child) {
        children.push_back(child);
    }

    void displayHierarchy(const string& indent = "") {
        cout << indent << "PID: " << pid << ", Name: " << name << "\n";
        for (auto* child : children) {
            child->displayHierarchy(indent + "  ");
        }
    }
};

class ProcessManager {
private:
    int pidCounter = 1;
    Process* root = nullptr;
    map<int, unique_ptr<Process>> processMap;
    queue<Process*> readyQueue;
    queue<Process*> ioQueue;

public:
    void createProcess(const string& name, int parentPID = -1) {
        Process* parent = nullptr;
        if (parentPID != -1 && processMap.count(parentPID)) {
            parent = processMap[parentPID].get();
        }

        auto proc = make_unique<Process>(pidCounter++, name, parent);
        Process* rawPtr = proc.get();

        if (parent) {
            parent->addChild(rawPtr);
        } else if (!root) {
            root = rawPtr;
        }

        processMap[rawPtr->pid] = move(proc);
        readyQueue.push(rawPtr);

        cout << "Created process: PID=" << rawPtr->pid << "\n";
    }

    void callFunction(int pid, const string& funcName) {
        auto it = processMap.find(pid);
        if (it != processMap.end()) {
            it->second->callStack.push(funcName);
            cout << "Process " << pid << " called function: " << funcName << "\n";
        } else {
            cout << "Process not found.\n";
        }
    }

    void requestIO(int pid) {
        auto it = processMap.find(pid);
        if (it != processMap.end()) {
            Process* p = it->second.get();
            queue<Process*> newQueue;
            bool found = false;

            while (!readyQueue.empty()) {
                Process* front = readyQueue.front(); readyQueue.pop();
                if (front == p) {
                    ioQueue.push(front);
                    found = true;
                } else {
                    newQueue.push(front);
                }
            }

            readyQueue = newQueue;

            if (found)
                cout << "Process " << pid << " moved to I/O queue.\n";
            else
                cout << "Process not in ready queue.\n";
        } else {
            cout << "Process not found.\n";
        }
    }

    void completeIO(int pid) {
        queue<Process*> newIOQueue;
        bool found = false;

        while (!ioQueue.empty()) {
            Process* front = ioQueue.front(); ioQueue.pop();
            if (front->pid == pid) {
                readyQueue.push(front);
                found = true;
            } else {
                newIOQueue.push(front);
            }
        }

        ioQueue = newIOQueue;

        if (found)
            cout << "Process " << pid << " completed I/O and returned to ready queue.\n";
        else
            cout << "Process not found in I/O queue.\n";
    }

    void terminateProcess(int pid) {
        auto it = processMap.find(pid);
        if (it == processMap.end()) {
            cout << "Process not found.\n";
            return;
        }

        Process* p = it->second.get();

        // Remove from ready and IO queues
        removeFromQueue(readyQueue, p);
        removeFromQueue(ioQueue, p);

        // Remove from parent's children
        if (p->parent) {
            auto& siblings = p->parent->children;
            siblings.erase(std::remove(siblings.begin(), siblings.end(), p), siblings.end());
        }

        // If root, clear root
        if (p == root) root = nullptr;

        processMap.erase(pid);
        cout << "Process " << pid << " terminated.\n";
    }

    void showState() {
        cout << "\n--- Ready Queue ---\n";
        displayQueue(readyQueue);

        cout << "\n--- I/O Queue ---\n";
        displayQueue(ioQueue);

        cout << "\n--- Process Tree ---\n";
        if (root)
            root->displayHierarchy();
        else
            cout << "(No processes created yet)\n";

        cout << "--------------------\n";
    }

private:
    void displayQueue(queue<Process*> q) {
        while (!q.empty()) {
            Process* p = q.front(); q.pop();
            cout << "PID: " << p->pid << ", Name: " << p->name << "\n";
        }
    }

    void removeFromQueue(queue<Process*>& q, Process* p) {
        queue<Process*> temp;
        while (!q.empty()) {
            Process* current = q.front(); q.pop();
            if (current != p) temp.push(current);
        }
        q = temp;
    }
};

// ---------------------- Main Function ----------------------

int main() {
    ProcessManager manager;
    int choice, pid;
    string name, func;

    do {
        cout << "\n--- OS Process Manager (Console) ---\n";
        cout << "1. Create Process\n";
        cout << "2. Call Function\n";
        cout << "3. Request I/O\n";
        cout << "4. Complete I/O\n";
        cout << "5. Terminate Process\n";
        cout << "6. Show State\n";
        cout << "0. Exit\n";
        cout << "Enter choice: ";
        cin >> choice;

        switch (choice) {
            case 1:
                cout << "Enter process name: ";
                cin >> name;
                cout << "Enter parent PID (-1 if none): ";
                cin >> pid;
                manager.createProcess(name, pid);
                break;
            case 2:
                cout << "Enter PID: ";
                cin >> pid;
                cout << "Enter function name: ";
                cin >> func;
                manager.callFunction(pid, func);
                break;
            case 3:
                cout << "Enter PID: ";
                cin >> pid;
                manager.requestIO(pid);
                break;
            case 4:
                cout << "Enter PID: ";
                cin >> pid;
                manager.completeIO(pid);
                break;
            case 5:
                cout << "Enter PID: ";
                cin >> pid;
                manager.terminateProcess(pid);
                break;
            case 6:
                manager.showState();
                break;
            case 0:
                cout << "Exiting...\n";
                break;
            default:
                cout << "Invalid choice.\n";
        }
    } while (choice != 0);

    return 0;
}

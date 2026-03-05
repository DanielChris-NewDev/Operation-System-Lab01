#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

using namespace std;

struct Queue {
    string QID = "";
    int timeSlice = 0;
    string schedulingPolicy = "";
};

struct Process {
    string PID = "";
    int arrivalTime = 0, burstTime = 0, turnaroundTime = 0, waitingTime = 0, remainingTime = 0, completionTime = 0;
    string QueueID = "";
};

void readFile(string filename, vector<Queue>& queue, vector<Process>& processes) {

    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Unable to open file\n";
        return;
    }

    string line;
    getline(file, line);
    int queueAmount = stoi(line);

    for (int i = 0; i < queueAmount; i++) {
        getline(file, line);

        Queue q;
        stringstream ss(line);
        string temp;

        getline(ss, q.QID, ' ');
        getline(ss, temp, ' ');
        q.timeSlice = stoi(temp);
        getline(ss, q.schedulingPolicy);

        queue.push_back(q);
    }

    while (getline(file, line)) {
        Process p;
        stringstream ss(line);
        string temp;

        getline(ss, p.PID, ' ');
        getline(ss, temp, ' ');
        p.arrivalTime = stoi(temp);
        getline(ss, temp, ' ');
        p.burstTime = stoi(temp);
        p.remainingTime = p.burstTime;
        getline(ss, p.QueueID);

        processes.push_back(p);
    }

    file.close();
    return;
    
}

void SJF(vector<Process>& processes, string QueueID, int& currentTime, int timeSlice) {
    int processTime = 0; //Thời gian chạy process

    while (processTime < timeSlice) {
        int index = -1;
        int shortestBurst = INT_MAX;

        for (int i = 0; i < processes.size(); i++) { //Chạy qua từng process
            if (processes[i].QueueID == QueueID && processes[i].remainingTime > 0) { //Tìm process cùng queueID và cần CPU
                if (processes[i].arrivalTime <= currentTime) { //process phải arrive trước thời gian hiện tại
                    if (processes[i].burstTime < shortestBurst) { //Tìm vị trí process ngắn nhất hiện tại
                        shortestBurst = processes[i].burstTime;
                        index = i;
                    }
                }
            }
        }
        if (index == -1) break; //Không có process nào cả

        Process& p = processes[index]; //Process đang sử dụng CPU

        int extraTime = timeSlice - processTime; //Thời gian dư (xài cho process kế)
        int runTime = min(p.remainingTime, extraTime); //Thời gian chạy

        cout << "[" << currentTime << "-" << currentTime + runTime << "]" << '\n';
        cout << QueueID << '\n' 
            << p.PID << '\n';

        currentTime += runTime; //Update thời gian
        p.remainingTime -= runTime; //Update thời gian cần chạy
        processTime += runTime; //Update thời gian chạy của process hiện tại

        //Công thức tính các thời gian
        if (p.remainingTime == 0) {
            p.completionTime = currentTime;
            p.turnaroundTime = p.completionTime - p.arrivalTime;
            p.waitingTime = p.turnaroundTime - p.burstTime;
        }   
    }
}

void runQueue(vector<Process>& processes, vector<Queue>& queues) {
    int currentTime = 0;
    int finished = 0; //Số process hoàn thành

    while (finished < processes.size()) { //Còn process cần chạy
        for (int i = 0; i < queues.size(); i++) {
            cout << "Doing: " << queues[i].QID << '\n';
            if (queues[i].schedulingPolicy == "SJF") {
                SJF(processes, queues[i].QID, currentTime, queues[i].timeSlice);
            }
            //Thêm SRTN
        }

        //Đếm lại có bao nhiêu process đã hoàn thành
        finished = 0;
        for (int j = 0; j < processes.size(); j++) {
            if (processes[j].remainingTime == 0) {
                finished++;
            }
        }
    }
}


int main()
{
    vector<Queue> queues;
    vector<Process> processes;

    readFile("test.txt", queues, processes);
    runQueue(processes, queues);

    
    cout << "PID" << "  " << "Completion" << "  " << "Turnaround" << "  " << "Waiting" << '\n';
    for (int i = 0; i < processes.size(); i++) {
        Process& p = processes[i];
        cout << p.PID << "  "
            << p.completionTime << "    "
            << p.turnaroundTime << "    "
            << p.waitingTime << '\n';
    }

    return 0;
}


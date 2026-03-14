#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <queue>
#include <iomanip>

using namespace std;

struct Queue {
    string QID;
    int timeSlice;
    string schedulingPolicy;
};

struct Process {
    string PID;
    int arrivalTime, burstTime, turnaroundTime, waitingTime, remainingTime, completionTime;
    string QueueID;
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

void SJF(vector<Process>& processes, string QueueID, int& currentTime, int timeSlice, ofstream& outputFile) {
    int processTime = 0; //Thời gian chạy process

    while (processTime < timeSlice) {
        int index = -1;
        int shortestBurst = INT_MAX;

        for (int i = 0; i < processes.size(); i++) { //Chạy qua từng process
            if (processes[i].QueueID == QueueID && processes[i].remainingTime > 0) { //Tìm process cùng queueID và cần CPU
                if (processes[i].arrivalTime <= currentTime) { //process phải arrive trước thời gian hiện tại
                    if (processes[i].remainingTime < shortestBurst) { //Tìm vị trí process ngắn nhất hiện tại
                        shortestBurst = processes[i].remainingTime;
                        index = i;
                    }
                }
            }
        }
        if (index == -1) {
            break; //Không có process nào cả
        }

        Process& p = processes[index]; //Process đang sử dụng CPU

        int extraTime = timeSlice - processTime; //Thời gian dư (xài cho process kế)
        int runTime = min(p.remainingTime, extraTime); //Thời gian chạy

        outputFile << "[" << currentTime << "-" << currentTime + runTime << "]" << "                " << QueueID << "                " << p.PID << '\n';

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

void SRTN(vector<Process>& processes, string QueueID, int& currentTime, int timeSlice, ofstream& outputFile) {
    int processTime = 0; //Thời gian chạy process
    int currentIndex = -1; // index của process đang chạy hiện tại
    int start = 0;      // thời gian bắt đầu của đoạn hiện tại

    //Vòng lặp chạy đến khi hết time slice
    while (processTime < timeSlice) {
        int index = -1;
        int shortestBurst = INT_MAX;
        //Qua mỗi vòng lặp, kiểm tra xem trong cùng một hàng đợi, có process nào đã đến và đang cần dùng CPU hay không
        for (int i = 0; i < processes.size(); i++) {
            if ((processes[i].QueueID == QueueID) && (processes[i].arrivalTime <= currentTime) && (processes[i].remainingTime > 0)) {
                //Kiểm tra xem trong hàng đợi, process nào có thời gian sử dụng CPU thấp nhất
                // Sau đó chọn process đó bằng cách cập nhật lại biến index và shortestBurst
                if (processes[i].remainingTime < shortestBurst) {
                    shortestBurst = processes[i].remainingTime;
                    index = i;
                }
            }
        }

        if (index == -1) {
            break;
        }

        // So sánh giá trị index mới với currentIndex, nếu process thay đổi thì in process cũ
        if (index != currentIndex) {
            if (currentIndex != -1) {
                outputFile << "[" << start << "-" << currentTime << "]" << "                " << QueueID << "                " << processes[currentIndex].PID << '\n';
            }

            //Cập nhật current index với index mới của process dang chạy
            // Thời gian bắt đầu chạy của process mới bằng với current time
            currentIndex = index;
            start = currentTime;
        }

        //Thời gian chạy của process được chọn giảm đi 1
        processes[index].remainingTime--;
        //Thời gian chạy hiện tại và thời gian cho 1 time slice cũng tăng lên 1
        currentTime++;
        processTime++;

        //Công thức tính các thời gian
        if (processes[index].remainingTime == 0) {
            processes[index].completionTime = currentTime;
            processes[index].turnaroundTime = processes[index].completionTime - processes[index].arrivalTime;
            processes[index].waitingTime = processes[index].turnaroundTime - processes[index].burstTime;
        }
    }

    //In ra thời gian chạy của process cuối
    if (currentIndex != -1) {
        outputFile << "[" << start << "-" << currentTime << "]" << "                " << QueueID << "                " << processes[currentIndex].PID << '\n';
    }
}

void runQueue(vector<Process>& processes, vector<Queue>& queues, ofstream& outputFile) {
    int currentTime = 0;
    int finished = 0; //Số process hoàn thành

    while (finished < processes.size()) { //Còn process cần chạy
        for (int i = 0; i < queues.size(); i++) {
            if (queues[i].schedulingPolicy == "SJF") {
                SJF(processes, queues[i].QID, currentTime, queues[i].timeSlice, outputFile);
            }
            if (queues[i].schedulingPolicy == "SRTN") {
                SRTN(processes, queues[i].QID, currentTime, queues[i].timeSlice, outputFile);
            }
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
    ofstream outputFile("Output.txt");

    readFile("Input.txt", queues, processes);
    outputFile << "\n================== CPU SCHEDULING DIAGRAM ==================\n";
    outputFile << "\n[Start - End]        Queue        Process\n";
    outputFile << "--------------------------------------------------------------\n";
    runQueue(processes, queues, outputFile);

    outputFile << "\n================ PROCESS STATISTICS ================\n";
    outputFile << left << setw(12) << "\nProcess" << setw(12) << "Arrival" << setw(10) << "Burst" << setw(15) << "Completion" << setw(15) << "Turnaround" << setw(10) << "Waiting" << '\n';
    outputFile << "---------------------------------------------------------------------------------\n";
    double averageTurnaroundTime = 0.0;
    double averageWaitingTime = 0.0;
    for (int i = 0; i < processes.size(); i++) {
        Process& p = processes[i];
        averageTurnaroundTime += p.turnaroundTime;
        averageWaitingTime += p.waitingTime;
        outputFile << left << setw(12) << p.PID << setw(12) << p.arrivalTime << setw(13) << p.burstTime << setw(15) << p.completionTime << setw(13) << p.turnaroundTime << setw(10) << p.waitingTime << '\n';
    }
    outputFile << "---------------------------------------------------------------------------------\n";
    outputFile << "\nAverage Turnaround Time : " << averageTurnaroundTime / processes.size();
    outputFile << "\nAverage Waiting Time : " << averageWaitingTime / processes.size() << '\n';
    outputFile << "\n============================================================\n";
    outputFile.close();
    return 0;
}


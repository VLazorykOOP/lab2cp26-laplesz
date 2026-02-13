#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <cmath>
#include <cstdlib>

using namespace std;

const int DELAY_MS = 200;
const int CHAOS_TIME = 2;

mutex printMutex;

class Insect {
public:
    double x, y;
    double startX, startY;
    double speed;

    Insect(double x, double y, double speed)
        : x(x), y(y), startX(x), startY(y), speed(speed) {
    }

    void runWorker() {
        bool returning = false;

        while (true) {
            double targetX = returning ? startX : 0.0;
            double targetY = returning ? startY : 0.0;

            double dx = targetX - x;
            double dy = targetY - y;
            double dist = sqrt(dx * dx + dy * dy);

            if (dist <= speed) {
                x = targetX;
                y = targetY;
                returning = !returning;

                lock_guard<mutex> lock(printMutex);
                cout << "[Worker] Reached the goal (" << x << ", " << y << "). I'm turning around." << endl;
            }
            else {
                x += (dx / dist) * speed;
                y += (dy / dist) * speed;
            }

            {
                lock_guard<mutex> lock(printMutex);
                cout << "[Worker] at (" << (int)x << ", " << (int)y << ")" << endl;
            }
            this_thread::sleep_for(chrono::milliseconds(DELAY_MS));
        }
    }

    void runDrone() {
        double dirX, dirY;
        int stepsLimit = (CHAOS_TIME * 1000) / DELAY_MS;
        int stepCount = 0;

        auto newDir = [&]() {
            double angle = (rand() % 360) * 3.14159 / 180.0;
            dirX = cos(angle);
            dirY = sin(angle);
            };
        newDir();

        while (true) {
            x += dirX * speed;
            y += dirY * speed;

            stepCount++;
            if (stepCount >= stepsLimit) {
                newDir();
                stepCount = 0;
                lock_guard<mutex> lock(printMutex);
                cout << "[Drone] Changed direction!" << endl;
            }

            {
                lock_guard<mutex> lock(printMutex);
                cout << "[Drone]  at (" << (int)x << ", " << (int)y << ")" << endl;
            }
            this_thread::sleep_for(chrono::milliseconds(DELAY_MS));
        }
    }
};

int main() {
    srand(time(0));

    cout << "Start simulation..." << endl;

    thread workerThread(&Insect::runWorker, Insect(50, 50, 5));

    thread droneThread(&Insect::runDrone, Insect(0, 0, 3));

    workerThread.join();
    droneThread.join();

    return 0;
}
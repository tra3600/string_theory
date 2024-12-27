#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

// Constants
const double PI = 3.14159265358979323846;
const int DIMENSIONS = 4; // 3 spatial dimensions + 1 time dimension
const int POINTS = 100; // Number of points on the string
const double AMPLITUDE = 1.0; // Amplitude of vibration
const double FREQUENCY = 1.0; // Frequency of vibration
const double TIME_STEP = 0.1; // Time step for simulation

// Function to initialize the string points
std::vector<std::vector<double>> initialize_string(int points, int dimensions) {
    std::vector<std::vector<double>> string_points(points, std::vector<double>(dimensions, 0.0));
    for (int i = 0; i < points; ++i) {
        string_points[i][0] = i; // Initial position in the first spatial dimension
    }
    return string_points;
}

// Function to update the string points based on harmonic oscillation
void update_string(std::vector<std::vector<double>>& string_points, double time) {
    for (auto& point : string_points) {
        for (int dim = 1; dim < DIMENSIONS; ++dim) { // Skip the first dimension (initial position)
            point[dim] = AMPLITUDE * std::sin(2 * PI * FREQUENCY * (point[0] / POINTS + time));
        }
    }
}

// Function to print the string points
void print_string(const std::vector<std::vector<double>>& string_points) {
    for (const auto& point : string_points) {
        for (double value : point) {
            std::cout << std::setw(10) << value << " ";
        }
        std::cout << std::endl;
    }
}

int main() {
    // Initialize the string points
    std::vector<std::vector<double>> string_points = initialize_string(POINTS, DIMENSIONS);

    // Simulate for a few time steps
    for (double time = 0; time < 10; time += TIME_STEP) {
        std::cout << "Time: " << time << std::endl;
        update_string(string_points, time);
        print_string(string_points);
        std::cout << std::endl;
    }

    return 0;
}
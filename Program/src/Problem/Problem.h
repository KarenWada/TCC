// *******************************************************************
//      file with specific functions to solve TD-TSP problem
// *******************************************************************
#ifndef _PROBLEM_H
#define _PROBLEM_H

#include <cstdio>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <numeric>

//----------------- DEFINITION OF PROBLEM SPECIFIC TYPES -----------------------

struct TProblemData
{
    int n;                                      // size of the RKO vector (number of customers)

    // TD-TSP data
    int totalNodes;                             // Total nodes including depot (indexed 0 to totalNodes - 1)
    int nIntervals;                             // Number of time intervals |H|
    double T;                                   // Length of each time interval T
    std::vector<double> serviceTime;            // Service time s at each node
    std::vector<std::vector<std::vector<double>>> travelTime; // travelTime[h][i][j]
};


//-------------------------- FUNCTIONS OF SPECIFIC PROBLEM --------------------------


/************************************************************************************
 Method: ReadData
 Description: read the input data for TD-TSP
*************************************************************************************/
void ReadData(char name[], TProblemData &data)
{ 
    FILE *arq = fopen(name, "r");

    if (arq == NULL)
    {
        printf("\nERROR: File (%s) not found!\n", name);
        getchar();
        exit(1);
    }

    // 1. Number of nodes, including the depot (indexed as 0) and 'n' customers.
    if (fscanf(arq, "%d", &data.totalNodes) != 1) {
        printf("\nERROR: Invalid format in (%s)!\n", name);
        fclose(arq);
        exit(1);
    }

    // 2. Number of time intervals |H|
    if (fscanf(arq, "%d", &data.nIntervals) != 1) {
        printf("\nERROR: Invalid format in (%s)!\n", name);
        fclose(arq);
        exit(1);
    }

    // 3. Length of each time interval T
    if (fscanf(arq, "%lf", &data.T) != 1) {
        printf("\nERROR: Invalid format in (%s)!\n", name);
        fclose(arq);
        exit(1);
    }

    // 4. List of service time s at each node
    data.serviceTime.resize(data.totalNodes);
    for (int i = 0; i < data.totalNodes; i++) {
        if (fscanf(arq, "%lf", &data.serviceTime[i]) != 1) {
            printf("\nERROR: Invalid format in (%s)!\n", name);
            fclose(arq);
            exit(1);
        }
    }

    // 5. Travel time matrices for each time interval h=0,1,2,...
    data.travelTime.resize(data.nIntervals, 
        std::vector<std::vector<double>>(data.totalNodes, std::vector<double>(data.totalNodes, 0.0)));

    for (int h = 0; h < data.nIntervals; h++) {
        for (int i = 0; i < data.totalNodes; i++) {
            for (int j = 0; j < data.totalNodes; j++) {
                if (fscanf(arq, "%lf", &data.travelTime[h][i][j]) != 1) {
                    printf("\nERROR: Invalid format in (%s)!\n", name);
                    fclose(arq);
                    exit(1);
                }
            }
        }
    }

    fclose(arq);

    // Number of customers to be ordered by Random Keys
    data.n = data.totalNodes - 1;
}

/************************************************************************************
 Method: EvaluateRoute
 Description: Simulates travel along a given customer sequence and computes total travel time
*************************************************************************************/
inline double EvaluateRoute(const std::vector<int> &route, const TProblemData &data)
{
    double currentTime = 0.0;
    double totalTravelTime = 0.0;
    int currentDepot = 0;

    int currentFrom = currentDepot;

    for (size_t k = 0; k < route.size(); k++) {
        int nextNode = route[k];
        
        int h = 0;
        if (data.T > 1e-9) {
            h = static_cast<int>(currentTime / data.T);
            if (h >= data.nIntervals) {
                h = data.nIntervals - 1;
            }
        }

        double tt = data.travelTime[h][currentFrom][nextNode];
        totalTravelTime += tt;
        currentTime += tt + data.serviceTime[nextNode];
        currentFrom = nextNode;
    }

    // Final leg back to depot
    int h = 0;
    if (data.T > 1e-9) {
        h = static_cast<int>(currentTime / data.T);
        if (h >= data.nIntervals) {
            h = data.nIntervals - 1;
        }
    }

    double finalTT = data.travelTime[h][currentFrom][currentDepot];
    totalTravelTime += finalTT;

    return totalTravelTime;
}

/************************************************************************************
 Method: Decoder 
 Description: mapping the random-key solution into a problem solution and applying 2-opt
*************************************************************************************/
double Decoder(TSol &s, const TProblemData &data)
{   
    if (data.n <= 0) return 0.0;

    // Create permutation based on random keys
    std::vector<int> route(data.n);
    std::iota(route.begin(), route.end(), 1); // Customers 1 to n

    std::sort(route.begin(), route.end(), [&s](int a, int b) {
        return s.rk[a - 1] < s.rk[b - 1];
    });

    // Evaluate initial route cost
    double bestCost = EvaluateRoute(route, data);

    // Efficient 2-Opt local search
    bool improved = true;
    int maxPasses = 5; 
    int pass = 0;

    while (improved && pass < maxPasses) {
        improved = false;
        pass++;

        for (int i = 0; i < data.n - 1; i++) {
            for (int j = i + 1; j < data.n; j++) {
                std::reverse(route.begin() + i, route.begin() + j + 1);
                double newCost = EvaluateRoute(route, data);

                if (newCost < bestCost - 1e-6) {
                    bestCost = newCost;
                    improved = true;
                } else {
                    // Revert move
                    std::reverse(route.begin() + i, route.begin() + j + 1);
                }
            }
        }
    }

    return bestCost;
}


/************************************************************************************
 Method: FreeMemoryProblem
 Description: Free local memory allocated by Problem
*************************************************************************************/
void FreeMemoryProblem(TProblemData &data){
    data.serviceTime.clear();
    data.travelTime.clear();
}

#endif
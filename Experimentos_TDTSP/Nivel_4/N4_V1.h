// *******************************************************************
//      file with specific functions to solve TDTSP
// *******************************************************************
#ifndef _PROBLEM_H
#define _PROBLEM_H

#include <vector>
#include <algorithm>
#include <numeric>
#include <cstdio>
#include <cstdlib>

//----------------- DEFINITION OF PROBLEM SPECIFIC TYPES -----------------------

struct TProblemData
{
    int n;                                      // size of the RKO vector (number of customers)

    // other variables of the problem at hand
    int nNodes;                                 // number of nodes (depot + customers)
    int nIntervals;                             // number of time intervals
    double intervalLength;                      // length of each time interval
    std::vector<double> serviceTimes;           // service time at each node
    // travelTimes[h][i][j]
    std::vector<std::vector<std::vector<double>>> travelTimes; 
};

//-------------------------- FUNCTIONS OF SPECIFIC PROBLEM --------------------------

/************************************************************************************
 Method: ReadData
 Description: read the input data
*************************************************************************************/
void ReadData(char name[], TProblemData &data)
{ 
    FILE *arq;
    arq = fopen(name,"r");

    if (arq == NULL)
    {
        printf("\nERROR: File (%s) not found!\n",name);
        getchar();
        exit(1);
    }

    // read data according to readme.txt
    fscanf(arq, "%d", &data.nNodes);
    fscanf(arq, "%d", &data.nIntervals);
    fscanf(arq, "%lf", &data.intervalLength);
    
    data.serviceTimes.resize(data.nNodes);
    for (int i = 0; i < data.nNodes; i++)
    {
        fscanf(arq, "%lf", &data.serviceTimes[i]);
    }

    data.travelTimes.resize(data.nIntervals, std::vector<std::vector<double>>(data.nNodes, std::vector<double>(data.nNodes)));
    for (int h = 0; h < data.nIntervals; h++)
    {
        for (int i = 0; i < data.nNodes; i++)
        {
            for (int j = 0; j < data.nNodes; j++)
            {
                fscanf(arq, "%lf", &data.travelTimes[h][i][j]);
            }
        }
    }
    
    // define the random-key vector size as the number of customers
    data.n = data.nNodes - 1;
    fclose(arq);
}

/************************************************************************************
 Method: EvaluateRoute
 Description: simulates the route and calculates total travel time
*************************************************************************************/
double EvaluateRoute(const std::vector<int>& route, const TProblemData& data)
{
    double currentTime = 0.0;
    double totalTravelTime = 0.0;
    int currentNode = 0; // Starts at depot

    for (size_t i = 0; i < route.size(); i++)
    {
        int nextNode = route[i];
        int h = (int)(currentTime / data.intervalLength);
        if (h >= data.nIntervals) h = data.nIntervals - 1;

        double travel = data.travelTimes[h][currentNode][nextNode];
        totalTravelTime += travel;
        currentTime += travel + data.serviceTimes[nextNode];
        currentNode = nextNode;
    }

    // Return to depot
    int h = (int)(currentTime / data.intervalLength);
    if (h >= data.nIntervals) h = data.nIntervals - 1;
    double travelToDepot = data.travelTimes[h][currentNode][0];
    totalTravelTime += travelToDepot;

    return totalTravelTime;
}

/************************************************************************************
 Method: Decoder 
 Description: mapping the random-key solution into a problem solution and 2-opt
*************************************************************************************/
double Decoder(TSol &s, const TProblemData &data)
{   
    std::vector<int> route(data.n);
    std::iota(route.begin(), route.end(), 1); // Fill with 1, 2, ..., nNodes-1

    // Sort based on random keys
    std::sort(route.begin(), route.end(), [&](int a, int b) {
        return s.rk[a - 1] < s.rk[b - 1];
    });

    double bestCost = EvaluateRoute(route, data);
    bool improved = true;

    // 2-opt local search
    while (improved) {
        improved = false;
        for (int i = 0; i < data.n - 1; i++) {
            for (int j = i + 1; j < data.n; j++) {
                std::vector<int> newRoute = route;
                std::reverse(newRoute.begin() + i, newRoute.begin() + j + 1);
                
                double newCost = EvaluateRoute(newRoute, data);
                if (newCost < bestCost) {
                    bestCost = newCost;
                    route = newRoute;
                    improved = true;
                }
            }
        }
    }
    
    return bestCost;
}

/************************************************************************************
 Method: FreeMemoryProblem
 Description: Free local memory allocate by Problem
*************************************************************************************/
void FreeMemoryProblem(TProblemData &data){
    data.serviceTimes.clear();
    data.travelTimes.clear();
}

#endif
// *******************************************************************
//      file with specific functions to solve TDTSP problem
// *******************************************************************
#ifndef _PROBLEM_H
#define _PROBLEM_H

#include <vector>
#include <algorithm>
#include <utility>
#include <cstdio>
#include <cstdlib>

//----------------- DEFINITION OF PROBLEM SPECIFIC TYPES -----------------------

struct TProblemData
{
    int n;                                      // size of the RKO vector (number of customers)

    // other variables of the problem at hand
    int numNodes;                               // total number of nodes (depot + customers)
    int numIntervals;                           // number of time intervals |H|
    int intervalLength;                         // length of each time interval T
    std::vector<int> serviceTime;               // service time at each node
    std::vector<std::vector<std::vector<int>>> travelTime; // travel time matrices [h][i][j]
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
        exit(1);
    }

    // read data according to readme format
    fscanf(arq, "%d", &data.numNodes);
    fscanf(arq, "%d", &data.numIntervals);
    fscanf(arq, "%d", &data.intervalLength);
    
    data.serviceTime.clear();
    data.serviceTime.resize(data.numNodes);
    for (int i = 0; i < data.numNodes; i++)
    {
        fscanf(arq, "%d", &data.serviceTime[i]);
    }

    data.travelTime.clear();
    data.travelTime.resize(data.numIntervals);
    for (int h = 0; h < data.numIntervals; h++)
    {
        data.travelTime[h].resize(data.numNodes);
        for (int i = 0; i < data.numNodes; i++)
        {
            data.travelTime[h][i].resize(data.numNodes);
            for (int j = 0; j < data.numNodes; j++)
            {
                fscanf(arq, "%d", &data.travelTime[h][i][j]);
            }
        }
    }
    
    // define the random-key vector size (number of customers to permute)
    data.n = data.numNodes - 1;
    
    fclose(arq);
}

/************************************************************************************
 Method: Decoder 
 Description: mapping the random-key solution into a problem solution
*************************************************************************************/
double Decoder(TSol &s, const TProblemData &data)
{   
    // decode random keys into a permutation of customers
    std::vector<std::pair<double, int>> rk_sorted(data.n);
    for (int i = 0; i < data.n; i++)
    {
        rk_sorted[i] = std::make_pair(s.rk[i], i + 1); // Customers are indexed 1 to n
    }
    std::sort(rk_sorted.begin(), rk_sorted.end());

    double totalTravelTime = 0.0;
    int currentTime = 0;
    int currentNode = 0; // start at the depot

    // traverse the permutation
    for (int i = 0; i < data.n; i++)
    {
        int nextNode = rk_sorted[i].second;
        
        // determine current time interval h
        int h = currentTime / data.intervalLength;
        if (h >= data.numIntervals) 
        {
            h = data.numIntervals - 1; // cap at last interval
        }
        
        int tt = data.travelTime[h][currentNode][nextNode];
        totalTravelTime += tt;
        currentTime += tt + data.serviceTime[nextNode];
        currentNode = nextNode;
    }

    // return to depot
    int h = currentTime / data.intervalLength;
    if (h >= data.numIntervals) 
    {
        h = data.numIntervals - 1;
    }
    
    totalTravelTime += data.travelTime[h][currentNode][0];
    
    // return objective function (minimize travel times)
    return totalTravelTime;
}

/************************************************************************************
 Method: FreeMemoryProblem
 Description: Free local memory allocate by Problem
*************************************************************************************/
void FreeMemoryProblem(TProblemData &data)
{
    data.serviceTime.clear();
    data.travelTime.clear();
}

#endif
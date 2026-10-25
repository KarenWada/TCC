// *******************************************************************
//      file with specific functions to solve Time-Dependent TSP
// *******************************************************************
#ifndef _PROBLEM_H
#define _PROBLEM_H

#include <vector>
#include <algorithm>
#include <cstdio>
#include <cstdlib>

//----------------- DEFINITION OF PROBLEM SPECIFIC TYPES -----------------------

struct TProblemData
{
    int n;                                      // size of the RKO vector (number of clients)

    // other variables of the problem at hand
    int nNodes;                                 // number of nodes (clients + 1 depot)
    int maxTime;                                // maximum time horizon for the time-dependent matrix
    
    // travelTime[i][j][t] -> travel time from node i to node j departing at time t
    std::vector<std::vector<std::vector<double>>> travelTime; 
};


//-------------------------- FUNCTIONS OF SPECIFIC PROBLEM --------------------------

/************************************************************************************
 Method: ReadData
 Description: read the input data
*************************************************************************************/
void ReadData(char name[], TProblemData &data)
{ 
    FILE *arq = fopen(name,"r");

    if (arq == NULL)
    {
        printf("\nERROR: File (%s) not found!\n",name);
        getchar();
        exit(1);
    }

    // => read data
    // Assuming first line: nNodes maxTime
    fscanf(arq, "%d %d", &data.nNodes, &data.maxTime);
    
    data.n = data.nNodes - 1; // RKO vector size represents the clients (excluding depot)

    // Resize travel time 3D matrix
    data.travelTime.clear();
    data.travelTime.resize(data.nNodes, std::vector<std::vector<double>>(data.nNodes, std::vector<double>(data.maxTime, 0.0)));

    // Assuming file format lists travel times: origin destination time_bin cost
    // Alternatively, reading a full 3D matrix
    for (int i = 0; i < data.nNodes; i++)
    {
        for (int j = 0; j < data.nNodes; j++)
        {
            for (int t = 0; t < data.maxTime; t++)
            {
                fscanf(arq, "%lf", &data.travelTime[i][j][t]);
            }
        }
    }
    
    fclose(arq);
}

/************************************************************************************
 Method: Decoder 
 Description: mapping the random-key solution into a problem solution
*************************************************************************************/
double Decoder(TSol &s, const TProblemData &data)
{   
    // decode random-keys into a sequence of clients
    std::vector<std::pair<double, int>> sequence(data.n);
    for (int i = 0; i < data.n; i++)
    {
        // clients are indexed from 1 to n (depot is 0)
        sequence[i] = std::make_pair(s.rk[i], i + 1); 
    }

    // sort to find the visit order based on random keys
    std::sort(sequence.begin(), sequence.end());

    // calculate the objective function value (total time)
    double currentTime = 0.0;
    int currentNode = 0; // start at depot (node 0)

    for (int i = 0; i < data.n; i++)
    {
        int nextNode = sequence[i].second;
        
        // determine the time bin, capping at maxTime - 1 if time exceeds horizon
        int t = (int)currentTime;
        if (t >= data.maxTime) t = data.maxTime - 1;

        // add travel time
        currentTime += data.travelTime[currentNode][nextNode][t];
        
        // move to next node
        currentNode = nextNode;
    }

    // Return to depot (node 0)
    int t = (int)currentTime;
    if (t >= data.maxTime) t = data.maxTime - 1;
    currentTime += data.travelTime[currentNode][0][t];
    
    // return cost (minimization problem)
    return currentTime;
}

/************************************************************************************
 Method: FreeMemoryProblem
 Description: Free local memory allocate by Problem
*************************************************************************************/
void FreeMemoryProblem(TProblemData &data){
    data.travelTime.clear();
}

#endif
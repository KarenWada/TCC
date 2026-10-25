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
    int n;                                      // size of the RKO vector (number of clients)[cite: 1]

    // other variables of the problem at hand[cite: 1]
    int num_nodes;                              // total nodes (depot_start + clients + depot_end)
    int num_intervals;                          // number of time intervals
    double interval_size;                       // duration of each time interval
    std::vector<double> service_time;           // service time for each node
    std::vector<std::vector<std::vector<double>>> travel_time; // travel_time[u][v][interval]
};

//-------------------------- FUNCTIONS OF SPECIFIC PROBLEM --------------------------

/************************************************************************************
 Method: ReadData
 Description: read the input data[cite: 1]
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

    // => read data[cite: 1]
    int num_clients;
    fscanf(arq, "%d", &num_clients);
    fscanf(arq, "%d", &data.num_intervals);
    fscanf(arq, "%lf", &data.interval_size);
    
    data.num_nodes = num_clients + 2; // 0 is start depot, num_clients+1 is end depot
    
    data.service_time.assign(data.num_nodes, 0.0);
    for (int i = 0; i < data.num_nodes; i++)
    {
        fscanf(arq, "%lf", &data.service_time[i]);
    }
    
    data.travel_time.assign(data.num_nodes, std::vector<std::vector<double>>(
        data.num_nodes, std::vector<double>(data.num_intervals, 0.0)));

    for (int i = 0; i < data.num_nodes; i++)
    {
        for (int j = 0; j < data.num_nodes; j++)
        {
            for (int k = 0; k < data.num_intervals; k++)
            {
                fscanf(arq, "%lf", &data.travel_time[i][j][k]);
            }
        }
    }
    
    // define the random-key vector size[cite: 1]
    data.n = num_clients;
    
    fclose(arq);
}

/************************************************************************************
 Method: Decoder 
 Description: mapping the random-key solution into a problem solution[cite: 1]
*************************************************************************************/
double Decoder(TSol &s, const TProblemData &data)
{   
    // Pair of <random_key, client_id> to decode the permutation
    std::vector<std::pair<double, int>> rk_sorted(data.n);                        
    for (int i = 0; i < data.n; i++)
    {
        rk_sorted[i] = std::make_pair(s.rk[i], i + 1); // clients are 1 to n
    }

    // Sort to obtain the sequence of visits based on random keys
    std::sort(rk_sorted.begin(), rk_sorted.end());

    // calculate the objective function value[cite: 1]
    double total_travel_time = 0.0;
    double current_time = 0.0;
    int current_node = 0; // starts at the initial depot (node 0)

    for (int i = 0; i < data.n; i++)
    {
        int next_node = rk_sorted[i].second;

        // Determine current time interval
        int interval = static_cast<int>(current_time / data.interval_size);
        if (interval >= data.num_intervals) {
            interval = data.num_intervals - 1; // Cap at the last available interval
        }

        // Add travel time to objective and update current time
        double t_ij = data.travel_time[current_node][next_node][interval];
        total_travel_time += t_ij;
        
        // Update current time (travel time + service time at the visited client)
        current_time += t_ij + data.service_time[next_node];

        current_node = next_node;
    }

    // Return to the final depot (node num_clients + 1)
    int end_depot = data.num_nodes - 1;
    int interval = static_cast<int>(current_time / data.interval_size);
    if (interval >= data.num_intervals) {
        interval = data.num_intervals - 1;
    }
    
    double t_return = data.travel_time[current_node][end_depot][interval];
    total_travel_time += t_return;

    return total_travel_time;
}

/************************************************************************************
 Method: FreeMemoryProblem
 Description: Free local memory allocate by Problem[cite: 1]
*************************************************************************************/
void FreeMemoryProblem(TProblemData &data){
    data.service_time.clear();
    data.travel_time.clear();
}

#endif
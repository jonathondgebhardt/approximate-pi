#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <mpi.h>

/*
* Approximates the value of pi. 
*
* Author: Jonathon Gebhardt
* Class: CS4900-B90
* Instructor: Dr. John Nehrbass
* Assignment: Homework 2 
* GitHub: https://github.com/jonathondgebhardt/approximate-pi 
*
* Requirements:
*   - Number of cpus used
*   - Total run time
*   - Smallest value of epsilon
*   - Best estimate of pi
*   - Number of iterations before converging
*   - Values of M (numbers inside the circle) and N (number of points 
*     generated)
*
* Challenge:
*   - The optimal value for P, where P is the number of points generated 
*     by each node
*/

void showUsage();
void initializeRNG();
int getInsidePoints(int);

int main(int argc, char *argv[])
{
    
    int rank, ncpu;

    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &ncpu);

    int opt, numPoints;
    double epsilon;
    numPoints = -1, epsilon = -1;

    // TODO: Should also check that numPoints and epsilon don't equal -1 
    // after this loop.
    while((opt = getopt(argc, argv, "n:e:")) != -1)
    {
        switch(opt)
        {
            case 'n':
                numPoints = atoi(optarg);
                break;
            case 'e':
                epsilon = atof(optarg);
                break;
            case '?':
                showUsage();
                break;
        }
    }

    if(numPoints == 0)
    {
        fprintf(stderr, "Error: invalid parameter '-n'\n");
        showUsage();
        return 1;
    }

    if(epsilon == 0)
    {
        fprintf(stderr, "Error: invalid parameter '-e'\n");
        showUsage();
        return 1;
    }

    int startTask = 1, stopTask = 0;
    if(rank == 0)
    {
        int i, totalInsidePoints = 0, totalPoints = 0, iterations = 0;
        double delta = 999, newApproximation, approximation = 0;
        
        // Dispatch tasks while delta is greater than epsilon.        
        printf("[%d] Starting work with %d nodes\n", rank, ncpu);
        while(delta > epsilon)
        {
            int msg;

            approximation = newApproximation;
            
            // Signal to workers to start task.
            for(i = 1; i < ncpu; ++i)
            {
                MPI_Send(&startTask, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
            }
           
            // Get response from workers.
            for(i = 1; i < ncpu; ++i)
            {
                MPI_Recv(&msg, 1, MPI_INT, i, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                totalInsidePoints += msg;
            }
            
            totalPoints += numPoints * (ncpu-1);

            newApproximation = 4 * (totalInsidePoints / (double)totalPoints);
//            if(approximation == 0)
//            {
//                printf("[%d] Starting with %f\n", rank, newApproximation);
//            }
//            else
//            {
//                approximation = newApproximation;
//            }
            
            printf("[%d] Comparing new %f with previous %f using epsilon %f\n", rank, newApproximation, approximation, epsilon);
            delta = abs(newApproximation - approximation);
            iterations++;
        }

        // Signal to workers that task is finished.
        printf("[%d] Stopping work\n", rank);
        for(i = 1; i < ncpu; ++i)
        {
            MPI_Send(&stopTask, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
        }
    
        printf("[%d] Approximation is %f after %d tries using %d workers\n", rank, approximation, iterations, (ncpu-1));
        printf("[%d] %d total inside points out of %d total points generated\n", rank, totalInsidePoints, totalPoints);
    }
    else
    {
        printf("[%d] Beginning task\n", rank);

        // Otherwise, generate points.
        initializeRNG();

        int i;
        while(1) 
        {
            int msg;
            // Wait for signal from taskmaster.
            MPI_Recv(&msg, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            if(msg == stopTask)
            {
                printf("[%d] Finished\n", rank);
                break;
            }

            int insidePoints;
            insidePoints = getInsidePoints(numPoints);

            MPI_Send(&insidePoints, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
        }
    } 
    
    MPI_Finalize();

    return 0;
}

void showUsage()
{
    printf("showing usage...\n");
}

/*
 * Using a constant as a seed, or even system time, can generate an 
 * identical series of numbers. Use /dev/random instead to seed to
 * more diversity in the seed.
 *
 * forums.justlinux.com/showthread.php?48570-srand-and-dev-urandom
 */
void initializeRNG()
{
    FILE *random_file = fopen("/dev/random","r");
    char random_seed = getc(random_file);
    srand(random_seed);
}

int getInsidePoints(int iterations)
{
    int i, insidePoints;
    double x, y;

    insidePoints = 0;
    for(i = 0; i < iterations; ++i)
    {
        x = (double)rand() / (double)RAND_MAX; 
        y = (double)rand() / (double)RAND_MAX;

        // We're using the unit circle so the radius is 1. Only consider 
        // points strictly within the radius as inside the circle.
        if((x * x + y * y) < 1)
        {
            insidePoints++;
        }
    }

    return insidePoints;
}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mpi.h>

/*
* Leverage Slurm and MPI to approximate pi.
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

void showUsage(char*);
void initializeRNG();
int getInsidePoints(int);

int main(int argc, char* argv[])
{
    
    int rank, ncpu;

    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &ncpu);

    int opt, numPoints;
    double epsilon;
    numPoints = -1, epsilon = -1;

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
                if(rank == 0)
                {
                    showUsage(argv[0]);
                }
                break;
        }
    }

    if(numPoints == 0 || numPoints == -1)
    {
        if(rank == 0)
        {
            fprintf(stderr, "Error: invalid parameter '-n'\n");
            showUsage(argv[0]);
        }
        
        return 1;
    }

    if(epsilon == 0 || epsilon == -1)
    {
        if(rank == 0)
        {
            fprintf(stderr, "Error: invalid parameter '-e'\n");
            showUsage(argv[0]);
        }
        
        return 1;
    }

    int startTask = 1, stopTask = 0;
    if(rank == 0)
    {
        double startTime = MPI_Wtime();
       
        int i, totalInsidePoints = 0, totalPoints = 0, iterations = 0, 
            converge = 0;
        double newApproximation, approximation = 0, delta = 999;
        
        // Dispatch tasks while delta is greater than epsilon.        
        printf("[%d] Starting work. n=%d, e=%f.\n", rank, numPoints, 
               epsilon);
        while(converge < 4)
        {
            // Signal to workers to start task.
            for(i = 1; i < ncpu; ++i)
            {
                MPI_Send(&startTask, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
            }
           
            // Get response from workers.
            int msg;
            for(i = 1; i < ncpu; ++i)
            {
                MPI_Recv(&msg, 1, MPI_INT, i, 1, MPI_COMM_WORLD, 
                         MPI_STATUS_IGNORE);
                totalInsidePoints += msg;
            }
            
            totalPoints += numPoints * (ncpu-1);

            newApproximation = 4 * 
                               (totalInsidePoints / (double)totalPoints);
            delta = fabs(newApproximation - approximation);
            approximation = newApproximation;
            
            iterations++;

            if(delta <= epsilon)
            {
                converge++;
            }
            else 
            {
                converge = 0;
            }
        }

        // Signal to workers that task is finished.
        printf("[%d] Stopping work\n", rank);
        for(i = 1; i < ncpu; ++i)
        {
            MPI_Send(&stopTask, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
        }
    
        double endTime = MPI_Wtime();

        // Show results.
        printf("[%d] Approximation is %f after %d tries using %d workers\n", 
               rank, approximation, iterations, (ncpu-1));
        printf("[%d] %d total inside points out of %d total points generated\n", 
               rank, totalInsidePoints, totalPoints);
        printf("[%d] Task took %f seconds\n", rank, 
               (endTime - startTime));
    }
    else
    {
        printf("[%d] Beginning task\n", rank);

        // Seed the worker nodes before generating random numbers.
        srand(rank);

        while(1) 
        {
            // Wait for signal from task master.
            int msg;
            MPI_Recv(&msg, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, 
                     MPI_STATUS_IGNORE);
            if(msg == stopTask)
            {
                printf("[%d] Finished\n", rank);
                break;
            }

            // Report amount of inside points generated to task master.
            int insidePoints = getInsidePoints(numPoints);
            MPI_Send(&insidePoints, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
        }
    } 
    
    MPI_Finalize();

    return 0;
}

void showUsage(char* applicationName)
{
    printf("Usage: %s [-n ] [-e]\n", applicationName);
    printf("\t-n: Number of points for each node to generate\n");
    printf("\t-e: Value of epsilon\n");
}

/*
 * Using a constant as a seed, or even system time, can generate an 
 * identical series of numbers. Use /dev/random instead to provide 
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

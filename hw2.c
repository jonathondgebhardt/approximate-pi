#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

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

int main(int argc, char *argv[])
{
    int opt, numPoints, epsilon;
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
                epsilon = atoi(optarg);
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

    initializeRNG();

    int i, insidePoints;
    double x, y;

    insidePoints = 0;
    for(i = 0; i < numPoints; ++i)
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

    printf("insidePoints is %d after %d runs\n", insidePoints, numPoints);

    double approximation; 
    approximation = 4 * (insidePoints / (double)numPoints);
    printf("approximation of pi is %f\n", approximation);

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

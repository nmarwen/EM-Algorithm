// Assignment 2(EM Algorithm) CompBio.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#define _USE_MATH_DEFINES

#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cmath>
#include <ctime>
#include <algorithm>
using namespace std;


// Todo: Take filename from command line and as parameter here.
vector<double> ReadFileToVector()
{
    ifstream inFile;
    inFile.open("dataFile.txt");

    vector<double> ret;
    std::string   line;
    // Read one line at a time into the variable line:
    while (getline(inFile, line))
    {
        std::stringstream  lineStream(line);

        double value;
        // Read an integer at a time from the line
        while (lineStream >> value)
        {
            ret.push_back(value);
        }
    }

    return ret;
}

class EMClustering
{
public:
    EMClustering(vector<double> trainingSet):
        m_trainingSet(trainingSet)
        {
        }

    void Train();

private:
    vector<double> InitializeMeans(double clusterCount);
    vector<double> InitializeVariance(double clusterCount, bool fUseVariance);

private:
    vector<double> m_trainingSet;
};

vector<double> EMClustering::InitializeMeans(double clusterCount)
{
    vector<double> retMeans = vector<double>(clusterCount, 0);
    for (int k = 0; k < clusterCount; k++)
    {
        int trainingCount = m_trainingSet.size();
        int usableRandomMax = ((RAND_MAX) / (trainingCount));
        usableRandomMax = usableRandomMax * (trainingCount);
        int randomIndex = RAND_MAX;
        while (randomIndex > usableRandomMax) { randomIndex = rand(); }
        randomIndex = randomIndex % (trainingCount);
        retMeans[k] = m_trainingSet[randomIndex];
    }

    // Initialize with k min.
    //for (int k = 0; k < clusterCount; k++)
    //{
    //    means[k] = m_trainingSet[k];
    //}

    return retMeans;
}

vector<double> EMClustering::InitializeVariance(double clusterCount, bool fUseVariance)
{
    vector<double> retMeans = vector<double>(clusterCount, 1);

    if (fUseVariance)
    {
        for (int k = 0; k < clusterCount; k++)
        {
            int trainingCount = m_trainingSet.size();
            int usableRandomMax = ((RAND_MAX) / (trainingCount));
            usableRandomMax = usableRandomMax * (trainingCount);
            int randomIndex = RAND_MAX;
            while (randomIndex > usableRandomMax) { randomIndex = rand(); }
            randomIndex = randomIndex % (trainingCount);
            retMeans[k] = m_trainingSet[randomIndex];
        }
    }
    // Initialize with k min.
    //for (int k = 0; k < clusterCount; k++)
    //{
    //    means[k] = m_trainingSet[k];
    //}

    return retMeans;
}
void EMClustering::Train()
{
    // Define the number of clusters.
    int numOfClusters = 1;
    int countRestart = 10;

    // set this flag for varying variance other than 1.
    bool fUseVariance = true;

    sort(m_trainingSet.begin(), m_trainingSet.end());
    double bicScore = INT_MIN;
    int maxCluster = -1;
    for (double clusterCount = 1; clusterCount <= 5; clusterCount++)
    {

        // Declare cluster specific variables.
        // We need to find the max among all the iterations.
        vector<vector<vector<double>>> outputMeans(countRestart);
        vector<vector<vector<double>>> outputVariance(countRestart);
        vector<vector<double>> outputProbabilities;
        double maxLogLikelihood = INT_MIN;
        double maxLogLikelihoodRestart = -1;

        for (int restartTimes = 0; restartTimes < countRestart; restartTimes++)
        {
            // Initialize means with random inputs
            vector<double> means = InitializeMeans(clusterCount);
            vector<double> variance = InitializeVariance(clusterCount, fUseVariance);
            vector<vector<double>> z = vector<vector<double>>(m_trainingSet.size(), vector<double>(clusterCount, 0));

            {
                // Populate the first row with initialized means.
                vector<double> tempOutputMeans;
                vector<double> tempOutputVariance;
                for(int k = 0; k < clusterCount; k++) tempOutputMeans.emplace_back(means[k]);
                for(int k = 0; k < clusterCount; k++) tempOutputVariance.emplace_back(variance[k]);
                tempOutputMeans.emplace_back(INT_MIN);
                tempOutputMeans.emplace_back(INT_MIN);
                outputMeans[restartTimes].emplace_back(tempOutputMeans);
                outputVariance[restartTimes].emplace_back(tempOutputVariance);
            }

            bool shouldContinue = true;
            int iteration = 0;
            double logLikelihood = 0;
            while (shouldContinue)
            {
                // E step
                for (int i = 0; i < m_trainingSet.size(); i++)
                {
                    double denominator = 0;
                    double numerator = 0;
                    for (int j = 0; j < clusterCount; j++)
                    {
                        double x_minus_u = m_trainingSet[i] - means[j];
                        denominator += exp( (-0.5 * x_minus_u * x_minus_u) / (variance[j]));
                    }

                    for (int j = 0; j < clusterCount; j++)
                    {
                        double x_minus_u = m_trainingSet[i] - means[j];
                        numerator = exp((-0.5 * x_minus_u * x_minus_u) / (variance[j]));
                        if (numerator == 0 || denominator == 0) 
                        {
                            z[i][j] = 0;
                        }
                        else
                        {
                            z[i][j] = numerator / denominator;
                        }
                    }
                }

                // M step
                // We need to calculate mean of every cluster.
                // By multiplying zij with xi
                for (int i = 0; i < clusterCount; i++)
                {
                    double numerator = 0;
                    double denominator = 0;
                    for (int j = 0; j < m_trainingSet.size(); j++)
                    {
                        double tempNum = z[j][i] * m_trainingSet[j];
                        numerator += tempNum;
                        double tempDeno = z[j][i];
                        denominator += tempDeno;
                    }
                    means[i] = numerator / denominator;
                }
                if (fUseVariance)
                {
                    // Now lets calculate variance for each cluster
                    for (int i = 0; i < clusterCount; i++)
                    {
                        double numerator = 0;
                        double denominator = 0;
                        for (int j = 0; j < m_trainingSet.size(); j++)
                        {
                            double tempNum = z[j][i] * (m_trainingSet[j] - means[i]) * (m_trainingSet[j] - means[i]);
                            numerator += tempNum;
                            double tempDeno = z[j][i];
                            denominator += tempDeno;
                        }
                        variance[i] = numerator / denominator;
                    }
                }
                // Calculate BIC score and logLikelihood
                double newLogLikelihood = 0;
                for (int i = 0; i < m_trainingSet.size(); i++)
                {
                    double prob_i = 0;
                    for (int j = 0; j < clusterCount; j++)
                    {
                        double x_minus_u = m_trainingSet[i] - means[j];
                        prob_i += ((1.0/sqrt(2.0 * M_PI)) * exp(( -0.5 * x_minus_u * x_minus_u )/(variance[j])));
                    }

                    // Divide by clusterCount as tau is 1/k
                    prob_i = prob_i/clusterCount;
                    newLogLikelihood += log(prob_i);
                }

                double curBicScore = 2 * newLogLikelihood - (clusterCount * log(m_trainingSet.size()));

                {
                    // Populate the first row with initialized means.
                    vector<double> tempOutputMeans;
                    vector<double> tempOutputVariance;
                    for (int k = 0; k < clusterCount; k++) tempOutputMeans.emplace_back(means[k]);
                    for (int k = 0; k < clusterCount; k++) tempOutputVariance.emplace_back(variance[k]);
                    tempOutputMeans.emplace_back(newLogLikelihood);
                    tempOutputMeans.emplace_back(curBicScore);
                    outputMeans[restartTimes].emplace_back(tempOutputMeans);
                    outputVariance[restartTimes].emplace_back(tempOutputVariance);
                }

                // check if we should continue or not
                if (abs(newLogLikelihood - logLikelihood) < 0.0001)
                {
                    if (bicScore < curBicScore )
                    {
                        bicScore = curBicScore;
                        maxCluster = clusterCount;
                    }
                    if (maxLogLikelihood < newLogLikelihood)
                    {
                        maxLogLikelihood = newLogLikelihood;
                        maxLogLikelihoodRestart = restartTimes;
                        outputProbabilities = z;
                    }
                    shouldContinue = false;
                }
                else
                {
                    logLikelihood = newLogLikelihood;
                }

                iteration++;
            }
        }

        // We have completed the algorithm for one cluster.
        // Lets print the output.
        cout << "CLUSTER: " << clusterCount << endl;
        string del = "";
        for (int ii = 0; ii < 10; ii++)del += " ";
        for (int ii = 0; ii < clusterCount; ii++) cout << "mu" << ii + 1 << del;
        cout << del << "LogLik" << del << "BIC" << endl;

        for (int ii = 0; ii < outputMeans[maxLogLikelihoodRestart].size(); ii++)
        {
            for (int jj = 0; jj < outputMeans[maxLogLikelihoodRestart][ii].size(); jj++)
                cout << outputMeans[maxLogLikelihoodRestart][ii][jj] << del;
            cout << endl;
        }
        cout<<endl<<"Variance"<<endl;
        for (int ii = 0; ii < outputVariance[maxLogLikelihoodRestart].size(); ii++)
        {
            for (int jj = 0; jj < outputVariance[maxLogLikelihoodRestart][ii].size(); jj++)
                cout << outputVariance[maxLogLikelihoodRestart][ii][jj] << del;
            cout << endl;
        }

        cout << endl;
        cout << del << "xi";
        for (int ii = 0; ii < clusterCount; ii++) cout << del << del<<"P(cls " << ii + 1 << " | xi)";
        cout << endl;
        for (int ii = 0; ii < min((int)outputProbabilities.size(), 25); ii++)
        {
            cout << "[" << ii + 1 << ",]" << del << m_trainingSet[ii];
            for (int jj = 0; jj < outputProbabilities[ii].size(); jj++)
            {
                cout << del << del<< outputProbabilities[ii][jj];
            }
            cout << endl;
        }

        cout << endl << endl;

    }

    cout<<"Max BIC Score: "<<bicScore<<endl<<"Cluster achieving max score:"<<maxCluster<<endl;
    getchar();
    getchar();
}

int main()
{
    // This is to set system clock and needs to be done once per program execution
    std::srand(std::time(0));

    EMClustering emClustering(ReadFileToVector());

    emClustering.Train();
    return 0;
}


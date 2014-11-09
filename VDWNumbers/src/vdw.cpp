// Find van der Waerden lower bounds by constructing certificates using 
// the power residue method.

//#include "stdafx.h"
#include <iostream>
#include <math.h>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
using namespace std;


void writeBestPrimes(long long candidateprime, long long bestprimes[100][100]) {
	cout << "Saving:" << candidateprime << endl;
	ofstream outputfile;
	outputfile.open("vdwtable.csv");
	if (outputfile.is_open())
		outputfile << candidateprime << endl;

	for (int lengths = 3; lengths < 20; lengths++) {
		for (int color = 2; color < 10; color++) {
			if (outputfile.is_open()) {
				outputfile << bestprimes[color][lengths] << ",";
//				cout << bestprimes[color][lengths] << ",";
			}
		}
		if (outputfile.is_open()) {
			outputfile << endl;
//			cout << endl;
		}
	}
	outputfile.close();
}

long long readBestPrimes(long long bestprimes[100][100]) {
	ifstream inputfile;
	inputfile.open("vdwtable.csv");
	long long startrange = 2;
	if (inputfile.is_open()) {
		if (inputfile) {
			string s;
			if (getline(inputfile, s)) {
				istringstream ss(s);
				startrange = atoi(s.c_str());
				cout << "Read start range: " << startrange << endl;
			}
		}
		int lengths = 3;
		int color = 2;
		while (inputfile) {
			string s;
			if (!getline(inputfile, s)) {
				break;
			}
			istringstream ss(s);
			while (ss) {
				if (!getline(ss, s, ',')) {
					color = 2;
					break;
				}
				bestprimes[color][lengths] = atoi(s.c_str());
				color++;
			}
			lengths++;
		}
		inputfile.close();
	} else {
		cout << "Error opening file" << endl;
	}
//	cout << "Finished reading input " << endl;
	return startrange;
}

bool isPrime(long long candidateprime) {
	// verify that candidateprime has no divisors>1
	for (long long divisor = 2;
			divisor <= sqrt((long double) (candidateprime)) + 1; divisor++) {
		if (candidateprime % divisor == 0) {
			return false;
			break;
		}
	}
	return true;
}

long long writeEveryThousand(long long candidateprime, long long threshold,
		long long bestprimes[100][100]) {
	//	  cout<<"Prime "<<prime<<endl;
	if (candidateprime > threshold) {
		writeBestPrimes(candidateprime, bestprimes);
		threshold = threshold + 1000;
	}
	return threshold;
}

int main() {
	long long maxnumcolors = 6;
	long long bestprimes[100][100];
	for (int i = 0; i < 100; i++)
		for (int j = 0; j < 100; j++)
			bestprimes[i][j] = 0;

	long long startrange = readBestPrimes(bestprimes);
	// look for primes and use them to color a list
	long long threshold = startrange + 1000;
	for (long long candidateprime = startrange;; candidateprime++) {
		if (isPrime(candidateprime)) {
//	  cout<<"Prime "<<prime<<endl;
			threshold = writeEveryThousand(candidateprime, threshold,
					bestprimes);

			//use prime to color the list
			long long* colors = NULL;
			colors = new long long[(unsigned int) candidateprime - 1 + 20];
//	  long long* zipcolors = NULL;
//	  zipcolors=new long long[(unsigned int)prime-1+20];
			for (long long i = 1; i < candidateprime; i++)
				colors[i] = 0;
			//find a primitive root of the prime
			for (long long candidateroot = 2;
					candidateroot < candidateprime - 1; candidateroot++) {
				colors[candidateroot] = 1;
				bool isroot = true;
				long long power = candidateroot;
				// Compute color series and check primitive root simultaneously
				for (long long exponent = 2; exponent <= candidateprime - 1;
						exponent++) {
					power = (power * candidateroot) % candidateprime;
					if (power == 1 && (exponent < candidateprime - 1))
						isroot = false;
					colors[power] = exponent;
				}
//		zipcolors[0]=0;
//		for(long long position=1; position<=prime-1; position++) {
//		  if ((colors[position] % 4)==0) zipcolors[position]=0;
//		  if ((colors[position] % 4)==1) zipcolors[position]=1;
//		  if ((colors[position] % 4)==2) zipcolors[position]=2;
//		  if ((colors[position] % 4)==3) zipcolors[position]=3;
//		  if ((colors[position] % 4)==4) zipcolors[position]=4;
//		  if ((colors[position] % 4)==5) zipcolors[position]=5;
//		}
//

//		for(long long position=0; position<=prime-1; position++) {
//		  cout<<zipcolors[position]; //<<",";
//		}
				//with 2 colors there is no need to find a primitive root
				//for(long long exponent=2; exponent<=prime/2; exponent++) {
				//power=exponent * exponent % prime;
				//if (power==1 ) colors[power] = 1;
				//}

				if (isroot) {
//		  cout<<"root: "<<candidateroot<<endl;
					// check conditions
					long long* lastentry = NULL;
					lastentry = new long long[(unsigned int) maxnumcolors + 1];
					long long* longestsequence = NULL;
					longestsequence = new long long[(unsigned int) maxnumcolors
							+ 1];
					long long* thissequence = NULL;
					thissequence =
							new long long[(unsigned int) maxnumcolors + 1];
					long long* sequencefromone = NULL;
					sequencefromone = new long long[(unsigned int) maxnumcolors
							+ 1];
					long long numcolors;
					for (numcolors = 2; numcolors <= maxnumcolors;
							numcolors++) {
						lastentry[numcolors] = colors[1] % numcolors;
						longestsequence[numcolors] = 0;
						thissequence[numcolors] = 1;
						sequencefromone[numcolors] = 1;
					}
					for (long long position = 2; position <= candidateprime - 1;
							position++) {
						for (numcolors = 2; numcolors <= maxnumcolors;
								numcolors++) {
							if ((colors[position] % numcolors)
									== lastentry[numcolors]) {
								thissequence[numcolors]++;
							} else {
								if (longestsequence[numcolors] == 0)
									sequencefromone[numcolors] =
											thissequence[numcolors];
								if (thissequence[numcolors]
										> longestsequence[numcolors])
									longestsequence[numcolors] =
											thissequence[numcolors];
								lastentry[numcolors] = colors[position]
										% numcolors;
								thissequence[numcolors] = 1;
							}
						}
					}
					for (numcolors = 2; numcolors <= maxnumcolors;
							numcolors++) {
						if ((candidateprime % numcolors) != 1)
							continue;
						long long length = 0;
						if ((colors[candidateprime - 1] % numcolors) == 0) {
							length = max(longestsequence[numcolors] + 1,
									sequencefromone[numcolors] * 2 + 2);
						} else {
							length = max(longestsequence[numcolors] + 1,
									sequencefromone[numcolors] + 2);
						}
						if (candidateprime > bestprimes[numcolors][length])
							bestprimes[numcolors][length] = candidateprime;
					}
					break;
				}
			}
			delete[] colors;
			colors = NULL;
//	  delete [] zipcolors;
//	  zipcolors=NULL;
		}
	}
	return 0;
}


// Find van der Waerden lower bounds by constructing certificates using 
// the power residue method.

#include <iostream>
#include <math.h>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <ctime>
using namespace std;
bool verbose = false;

void writeBestPrimes(long long candidateprime, long long bestprimes[100][100],
		long long bestbounds[100][100]) {
	cout << "Saving:" << candidateprime << endl;
	ofstream outputfile;
	outputfile.open("vdwtable.csv");
	if (outputfile.is_open())
		outputfile << candidateprime << endl;
	for (int lengths = 3; lengths < 20; lengths++) {
		for (int color = 2; color < 10; color++)
			outputfile << bestprimes[color][lengths] << ",";
		outputfile << endl;
	}
	outputfile << "-1" << endl;
	for (int lengths = 3; lengths < 20; lengths++) {
		for (int color = 2; color < 10; color++)
			outputfile << (bestprimes[color][lengths] * (lengths - 1) + 1)
					<< ",";
		outputfile << endl;
	}
	outputfile.close();
}

long long readBestPrimes(long long bestprimes[100][100],
		long long bestbounds[100][100]) {
	ifstream inputfile;
	inputfile.open("vdwtable.csv");
	long long startrange = 3;
	if (inputfile.is_open()) {
		if (inputfile) {
			string s;
			if (getline(inputfile, s)) {
				istringstream ss(s);
				startrange = atoi(s.c_str());
				if (verbose)
					cout << "Read start range: " << startrange << endl;
			}
		}
		int lengths = 3;
		int color = 2;
		bool foundBestBounds = false;
		while (inputfile) {
			string s;
			if (!getline(inputfile, s))
				break;
			istringstream ss(s);
//			cout << "Line:" << s << endl;
			while (ss) {
				if (!getline(ss, s, ',')) {
					color = 2;
					break;
				}
				if (atoi(s.c_str()) == -1) {
					foundBestBounds = true;
					lengths = 2;
					continue;
				}
				if (!foundBestBounds)
					bestprimes[color][lengths] = atoi(s.c_str());
				else {
					bestbounds[color][lengths] = atoi(s.c_str());
				}
				color++;
			}
			lengths++;
		}
		inputfile.close();
	} else {
		if (verbose)
			cout << "Error opening file" << endl;
	}
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
		long long bestprimes[100][100], long long bestbounds[100][100]) {
	if (candidateprime > threshold) {
		writeBestPrimes(candidateprime, bestprimes, bestbounds);
		threshold = threshold + 1000;
	}
	return threshold;
}

int main() {
	long long maxNumColors = 6;
	long long bestPrimes[100][100];
	long long bestBounds[100][100];
//	for (int i = 0; i < 100; i++)
//		for (int j = 0; j < 100; j++) {
//			bestPrimes[i][j] = 0;
//			bestBounds[i][j] = 0;
//		}
	long long startRange = readBestPrimes(bestPrimes, bestBounds);
	// look for primes and use them to color a list
	long long threshold = startRange + 1000;
	int counter = 0;
	clock_t begin3 = clock();
	for (long long candidatePrime = startRange;;
			candidatePrime = candidatePrime + 2) {
		if (isPrime(candidatePrime)) {
			clock_t end3 = clock();
			cout << "Primes:" << (end3 - begin3) << endl;

			counter++;
			if (counter > 8)
				break;
			threshold = writeEveryThousand(candidatePrime, threshold,
					bestPrimes, bestBounds);
			//use prime to color the list
			long long* powersOfPrimitiveRootModp =
					new long long[(unsigned int) candidatePrime - 1 + 20];
			//find a primitive root of the prime
			for (long long candidateRoot = 2;
					candidateRoot < candidatePrime - 1; candidateRoot++) {
				powersOfPrimitiveRootModp[candidateRoot] = 1;
				bool isRoot = true;
				long long power = candidateRoot;
				// Compute powers[] of the root and check primitive root simultaneously
				clock_t begin = clock();

				for (long long exponent = 2; exponent <= candidatePrime - 1;
						exponent++) {
					power = (power * candidateRoot) % candidatePrime;
					if (power == 1 && (exponent < candidatePrime - 1)) {
						isRoot = false;
						break;
					}
					powersOfPrimitiveRootModp[power] = exponent;
				}
				clock_t end = clock();
				cout << "Powers:" << (end - begin) << endl;
				if (isRoot) {
					// check conditions
					long long* lengthOfLongestSequence =
							new long long[(unsigned int) maxNumColors + 1];
					long long* lengthOfThisSequence =
							new long long[(unsigned int) maxNumColors + 1];
//					long long* sequenceFromOne =
//							new long long[(unsigned int) maxNumColors + 1];
					long long numColors;
					long long difference;
					for (numColors = 2; numColors <= maxNumColors;
							numColors++) {
						lengthOfLongestSequence[numColors] = 0;
						lengthOfThisSequence[numColors] = 1;
//						sequenceFromOne[numColors] = 1;
					}
					clock_t begin2 = clock();
					for (long long position = 2; position <= candidatePrime - 1;
							position++) {
						difference = powersOfPrimitiveRootModp[position]
								- powersOfPrimitiveRootModp[position - 1];
						for (numColors = 2; numColors <= maxNumColors;
								numColors++)
							// If the color at this position is the same as the last position
							if (difference % numColors == 0) {
								lengthOfThisSequence[numColors]++;
							} else {
//								if (lengthOfLongestSequence[numColors] == 0)
//									sequenceFromOne[numColors] =
//											lengthOfThisSequence[numColors];
								lengthOfLongestSequence[numColors] = max(
										lengthOfLongestSequence[numColors],
										lengthOfThisSequence[numColors]);
								lengthOfThisSequence[numColors] = 1;
							}
					}
					clock_t end2 = clock();
					cout << "Sequences:" << (end2 - begin2) << endl;
					for (numColors = 2; numColors <= maxNumColors;
							numColors++) {
						if ((candidatePrime % numColors) != 1)
							continue;
						long long length = 0;
						length = lengthOfLongestSequence[numColors] + 1;
//						if ((powersOfPrimitiveRootModp[candidatePrime - 1]
//								% numColors) == 0) {
//							length = max(lengthOfLongestSequence[numColors] + 1,
//									sequenceFromOne[numColors] * 2 + 2);
//						} else {
//							length = max(lengthOfLongestSequence[numColors] + 1,
//									sequenceFromOne[numColors] + 2);
//						}
						if (candidatePrime > bestPrimes[numColors][length])
							bestPrimes[numColors][length] = candidatePrime;
					}
					// Only try one primitive root per prime
					break;
				}
			}
			delete[] powersOfPrimitiveRootModp;
			powersOfPrimitiveRootModp = NULL;
			begin3 = clock();

		}
	}
	return 0;
}

//	  long long* zipcolors = NULL;
//	  zipcolors=new long long[(unsigned int)prime-1+20];
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
//		  if(verbose) cout<<zipcolors[position]; //<<",";
//		}
//with 2 colors there is no need to find a primitive root
//for(long long exponent=2; exponent<=prime/2; exponent++) {
//power=exponent * exponent % prime;
//if (power==1 ) colors[power] = 1;
//}

//	  delete [] zipcolors;
//	  zipcolors=NULL;

#include <iostream>
#include <math.h>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
using namespace std;

void savebestprimes(long long lastPossiblePrime, int colors,
		long long bestPrimes[100][12]) {
	// Writes the best primes into daniel.csv
	ofstream outputfile;
	outputfile.open("daniel.csv");
	if (outputfile.is_open()) {
		if (outputfile.is_open())
			outputfile << lastPossiblePrime << endl;

		for (int i = 3; i < 24; i++) {
			for (int numberofcolors = 2; numberofcolors < colors;
					numberofcolors++) {
				outputfile << bestPrimes[i][numberofcolors];
				outputfile << ",";
			}
			outputfile << endl;
		}
	}
	outputfile.close();
}

int main() {

	int colors = 11;
	// creates array of best primes of each length and initializes it to zero
	long long bestPrimes[100][12];
	for (int i = 0; i < 100; i++)
		for (int numberofcolors = 2; numberofcolors < colors; numberofcolors++)
			bestPrimes[i][colors] = 0;

	long long startValue = 3;

	//opens vdwtable.csv and reads the file into bestPrimes
	ifstream inputfile;
	inputfile.open("daniel.csv");
	int lengths = 3;
	if (inputfile.is_open()) {
		if (inputfile) {
			string s;
			if (getline(inputfile, s)) {
				istringstream ss(s);
				startValue = atoi(s.c_str());
				cout << "Start value is: " << startValue << endl;
			}
		}
		int numberofcolors = 2;
		while (inputfile) {
			string s;
			if (!getline(inputfile, s)) {
				break;
			}
			istringstream ss(s);
			while (ss) {
				if (!getline(ss, s, ',')) {
					numberofcolors = 2;
					break;
				}
				bestPrimes[lengths][numberofcolors] = atoi(s.c_str());
				numberofcolors++;
			}
			lengths++;
		}
		inputfile.close();
	}
	int counter = 0;
	//startValue = 37;
	for (long long possiblePrime = startValue;; possiblePrime++) {
		if (counter++ >= 499) {
			counter = 0;
			savebestprimes(possiblePrime, colors, bestPrimes);
			cout << possiblePrime << endl;
		}
		bool isPrime = true;
		//checks if possiblePrime is prime
		for (long long possibleFactor = 2;
				possibleFactor < sqrt(possiblePrime) + 1; possibleFactor++)
			if (possiblePrime % possibleFactor == 0) {
				isPrime = false;
				break;
			}
		if (isPrime) {
			//checks if possibleRoot is a primitive root
			long long* powers = new long long[(unsigned int) possiblePrime + 20];
			for (long possibleRoot = 2; possibleRoot < possiblePrime - 1;
					possibleRoot++) {
				powers[possibleRoot] = 1;

				long long accumulatedPower = possibleRoot;
				bool isPrimitive = true;
				for (long long exponent = 2; exponent <= possiblePrime - 1;
						exponent++) {
					accumulatedPower = (possibleRoot * accumulatedPower)
							% possiblePrime;
					if ((accumulatedPower == 1)
							&& (exponent < possiblePrime - 1)) {
						isPrimitive = false;
						break;
					}
					powers[accumulatedPower] = exponent;

				}

				//If possibleRoot is a primitive root, then check
				//check what the longest sequence for this prime is
				if (isPrimitive) {
//					cout << " " << possibleRoot << " is a primitive root of "
//							<< possiblePrime << "." << endl;
					long long* finalLength = new long long[(unsigned int) colors
							+ 1];
					long long* currentLength =
							new long long[(unsigned int) colors + 1];
					long long* sequenceFromOne =
							new long long[(unsigned int) colors + 1];
					for (int numberofcolors = 2; numberofcolors < colors;
							numberofcolors++) {
						currentLength[numberofcolors] = 1;
						finalLength[numberofcolors] = 0;
						sequenceFromOne[numberofcolors] = 1;
					}
					int numberofcolors = 0;
//					cout << powers[1] << ":" << powers[2] << ":" << powers[3]
//							<< ":" << powers[4] << ":" << powers[5] << endl;
					for (long long position = 2; position <= possiblePrime - 1;
							position++) {
						for (numberofcolors = 2; numberofcolors < colors;
								numberofcolors++) {
							if ((powers[position] % numberofcolors)
									== (powers[position - 1] % numberofcolors)) {
								currentLength[numberofcolors]++;
							} else {
								if (finalLength[numberofcolors] == 0)
									sequenceFromOne[numberofcolors] =
											currentLength[numberofcolors];
								finalLength[numberofcolors] = max(
										finalLength[numberofcolors],
										currentLength[numberofcolors]);
								currentLength[numberofcolors] = 1;
							}
						}
//						cout << position << ":" << currentLength[4] << ":"
//								<< finalLength[4] << ":" << sequenceFromOne[4]
//								<< endl;
					}
					// If possiblePrime is greater then the previous bestPrime for that length,
					// set the bestPrimes to the possiblePrime
					for (numberofcolors = 2; numberofcolors < colors;
							numberofcolors++) {
						if ((possiblePrime % numberofcolors) != 1)
							continue;
						long long length = 0;
						length = finalLength[numberofcolors] + 1;
						if ((powers[possiblePrime - 1] % numberofcolors) == 0) {
							length = max(finalLength[numberofcolors] + 1,
									sequenceFromOne[numberofcolors] * 2 + 2);
						} else {
							length = max(finalLength[numberofcolors] + 1,
									sequenceFromOne[numberofcolors] + 2);
						}
//						if (numberofcolors == 4)
//							cout << "results:" << numberofcolors << ":"
//									<< length << ":" << possiblePrime << endl;
						if (possiblePrime
								> bestPrimes[length][numberofcolors]) {
							bestPrimes[length][numberofcolors] = possiblePrime;
						}
					}
					delete[] finalLength;
					finalLength = NULL;
					delete[] currentLength;
					currentLength = NULL;
					break;
				}
			}
			delete[] powers;
			powers = NULL;
			//break;
		}
	}

	// Writes the best primes into daniel.csv
	return 0;
}

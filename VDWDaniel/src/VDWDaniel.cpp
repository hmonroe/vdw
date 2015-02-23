#include <iostream>
#include <math.h>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
using namespace std;

//returns the result of a to be b power using the binary representation of b
long long fastExponent(long long a, long long b, long long prime) {
	long long power, digit;
	power = 1;
	//for very large powers, this can be adjusted accordingly
	digit = ((long long) 1 << 50);

	while ((digit & b) == 0) {
		digit = digit >> 1;
	}
	while (digit != 1) {
		if (digit & b) {
			power = (power * a) % prime;
		}
		power = (power * power) % prime;
		digit = digit >> 1;
	}
	if (digit & b) {
		power = (power * a) % prime;
	}
	return (power % prime);
}

long long slowExponent(long long a, long long b, long long prime) {
	long long power, i;
	power = 1;
	//for very large powers, this can be adjusted accordingly
	for (i = 1; i < b + 1; i++) {
		power = (power * a) % prime;
	}
	return (power % prime);
}

//finds the least primitive root of parameter prime
long long getFactorRoot(long long prime) {
	long long primefactors[256]; // number of prime factors is less than number of bits
	long long guess, curNumber, i, foundone;
	long long factorcount, curPrime, factordown;
	factordown = (prime - 1);
	factorcount = 0;
	for (curPrime = 2;; curPrime++) {
		bool isPrime = true;
		for (long long possibleFactor = 2; possibleFactor < sqrt(curPrime) + 1; possibleFactor++) {
			if (curPrime % possibleFactor == 0 && curPrime != 2) {
				isPrime = false;
				break;
			}
		}
		if (!isPrime)
			continue;
		if ((factordown % curPrime == 0)) {
			//cout << "New factor:" << curPrime << endl;
			primefactors[factorcount] = curPrime;
			factorcount++;
			while (factordown % curPrime == 0) {
				factordown = factordown / curPrime;
				//cout << "Remaining:" << factordown << endl;
			}
		}
		if (factordown > 1 && curPrime > sqrt(factordown) + 1) {
			primefactors[factorcount] = factordown;
			factorcount++;
			factordown = 1;
		}
		if (factordown == 1)
			break;
	}
	guess = 2;
	while (true) {
		foundone = 0;
		for (i = 0; i < factorcount; ++i) {
			curNumber = fastExponent(guess, ((prime - 1) / primefactors[i]), prime);
			if (curNumber == 1) {
				foundone = 1;
				break;
			}
		}
		if (foundone == 0) {
			break;
		}
		guess++;
	}
	return guess;
}

void savebestprimes(long long lastPossiblePrime, int colors, long long bestPrimes[100][13]) {
// Writes the best primes into daniel.csv
	ofstream outputfile;
	outputfile.open("daniel.csv");
	if (outputfile.is_open()) {
		if (outputfile.is_open())
			outputfile << lastPossiblePrime << endl;

		for (int i = 3; i < 24; i++) {
			for (int numberofcolors = 2; numberofcolors < colors; numberofcolors++) {
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
	long long bestPrimes[100][13];
	for (int i = 0; i < 100; i++)
		for (int numberofcolors = 0; numberofcolors < colors; numberofcolors++)
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
	for (long long possiblePrime = startValue;; possiblePrime++) {
		if (counter++ >= 50) {
			counter = 0;
			savebestprimes(possiblePrime, colors, bestPrimes);
			cout << possiblePrime << endl;
		}
		bool isPrime = true;
		for (long long possibleFactor = 2; possibleFactor < sqrt(possiblePrime) + 1; possibleFactor++)
			if (possiblePrime % possibleFactor == 0) {
				isPrime = false;
				break;
			}
		if (isPrime) {
			long long* powers = new long long[(unsigned int) possiblePrime + 20];
			long long possibleRoot = getFactorRoot(possiblePrime);
			//cout << possibleRoot << " is the primitive root of " << possiblePrime << endl;
			{
				powers[possibleRoot] = 1;
				long long accumulatedPower = possibleRoot;
				bool isPrimitive = true;
				for (long long exponent = 2; exponent <= possiblePrime - 1; exponent++) {
					accumulatedPower = (possibleRoot * accumulatedPower) % possiblePrime;
					if ((accumulatedPower == 1) && (exponent < possiblePrime - 1)) {
						isPrimitive = false;
						break;
					}
					powers[accumulatedPower] = exponent;
				}
				if (!isPrimitive) {
					cout << possibleRoot << " is not a primitive root of " << possiblePrime << endl;
					return 0;
				}
				if (isPrimitive) {
					long long* finalLength = new long long[(unsigned int) colors + 1];
					long long* currentLength = new long long[(unsigned int) colors + 1];
					long long* sequenceFromOne = new long long[(unsigned int) colors + 1];
					for (int numberofcolors = 0; numberofcolors < colors; numberofcolors++) {
						currentLength[numberofcolors] = 1;
						finalLength[numberofcolors] = 0;
						sequenceFromOne[numberofcolors] = 1;
					}
					int numberofcolors = 2;
					for (long long position = 2; position <= possiblePrime - 1; position++) {
						for (numberofcolors = 2; numberofcolors < colors; numberofcolors++) {
							if ((powers[position] % numberofcolors) == (powers[position - 1] % numberofcolors)) {
								currentLength[numberofcolors]++;
								//cout << "Current:" << currentLength[numberofcolors] << endl;
							} else {
								if (finalLength[numberofcolors] == 0)
									sequenceFromOne[numberofcolors] = currentLength[numberofcolors];
								finalLength[numberofcolors] = max(finalLength[numberofcolors], currentLength[numberofcolors]);
								//cout << "Final:" << finalLength[numberofcolors] << endl;
								currentLength[numberofcolors] = 1;
							}
						}
					}
					// If possiblePrime is greater then the previous bestPrime for that length,
					// set the bestPrimes to the possiblePrime
					for (numberofcolors = 2; numberofcolors < colors; numberofcolors++) {
						if ((possiblePrime % numberofcolors) != 1)
							continue;
						long long length = 0;
						//cout << "Final Length:" << numberofcolors << ":" << finalLength[numberofcolors] << endl;
						length = finalLength[numberofcolors] + 1;
						if ((powers[possiblePrime - 1] % numberofcolors) == 0) {
							length = max(finalLength[numberofcolors] + 1, sequenceFromOne[numberofcolors] * 2 + 2);
						} else {
							length = max(finalLength[numberofcolors] + 1, sequenceFromOne[numberofcolors] + 2);
						}
						if (length < 24 && possiblePrime > bestPrimes[length][numberofcolors]) {
							//cout << "Found better prime:" << length << ":" << numberofcolors << ":" << possiblePrime << endl;
							if (bestPrimes[length][numberofcolors] < 10000 && length < 5) {
								cout << "Found bug" << endl;
								return 0;
								// 8284951
								// 10717548

							}
							bestPrimes[length][numberofcolors] = possiblePrime;
						}
					}
					delete[] finalLength;
					finalLength = NULL;
					delete[] currentLength;
					currentLength = NULL;
					delete[] sequenceFromOne;
					sequenceFromOne = NULL;
				}
			}
			delete[] powers;
			powers = NULL;
		}
	}
	return 0;
}

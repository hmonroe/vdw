#include <iostream>
#include <math.h>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
using namespace std;

int main() {
	// creates array of best primes of each and initializes it to zero
	int bestPrimes[100];
	for (int i = 0; i < 100; i++)
		bestPrimes[i] = 0;

	long startValue = 3;

	//opens vdwtable.csv and reads the file into bestPrimes
	ifstream inputfile;
	inputfile.open("vdwtable.csv");
	int lengths = 0;
	long long startrange = 3;
	if (inputfile.is_open()) {
		if (inputfile) {
			string s;
			if (getline(inputfile, s)) {
				istringstream ss(s);
				startrange = atoi(s.c_str());
			}
		}
		while (inputfile) {
			string s;
			if (!getline(inputfile, s)) {
				break;
			}
			istringstream ss(s);
			while (ss) {
				bestPrimes[lengths] = atoi(s.c_str());
			}
			lengths++;
		}
		inputfile.close();
	}

	for (long possiblePrime = startValue; possiblePrime < startValue + 99;
			possiblePrime++) {
		bool isPrime = true;
		//checks if possiblePrime is prime
		for (long possibleFactor = 2; possibleFactor < sqrt(possiblePrime) + 1;
				possibleFactor++)
			if (possiblePrime % possibleFactor == 0) {
				isPrime = false;
				break;
			}
		if (isPrime) {
			//checks if possibleRoot is a primitive root
			for (long possibleRoot = 2; possibleRoot < possiblePrime;
					possibleRoot++) {
				int powers[possiblePrime];
				long accumulatedPower = 1;
				bool isPrimitive = true;
				for (long exponent = 1; exponent < possiblePrime - 1;
						exponent++) {
					accumulatedPower = (possibleRoot * accumulatedPower)
							% possiblePrime;
//					cout << accumulatedPower << " ";
					powers[accumulatedPower] = exponent;
					if (accumulatedPower == 1) {
//						cout << " " << possibleRoot
//								<< " is not a primitive root of "
//								<< possiblePrime << "." << endl;
						isPrimitive = false;
						break;
					}

				}
				cout << endl;
				//If possibleRoot is a primitve root, then check
				if (isPrimitive) {
//					cout << possibleRoot << " is a primitive root of "
//							<< possiblePrime << "." << endl;
					//check what the longest sequence for this prime is
					long currentLength = 1;
					long finalLength = 1;
					for (long position = 2; position <= possiblePrime - 1;
							position++) {
						cout << powers[position] % 2;
						if (powers[position] % 2 == powers[position - 1] % 2) {
							currentLength++;
							if (currentLength > finalLength)
								finalLength = currentLength;

						} else
							currentLength = 1;
					}
					cout << endl;


					cout << "Length " << finalLength + 1 << " " << possiblePrime
							<< endl;
					// If possiblePrime is greater then the previous bestPrime for that length,
					// set the bestPrimes to the possiblePrime
					if (possiblePrime > bestPrimes[finalLength]) {
						bestPrimes[finalLength] = possiblePrime;
//						cout <<"better prime"<< bestPrimes[finalLength] << endl;
					}

					break;
				}
			}
		}
	}

	// Writes the best primes into daniel.csv
	ofstream outputfile;
	outputfile.open("daniel.csv");
	if (outputfile.is_open()) {
		for (int i = 1; i < 24; i++)
			outputfile << bestPrimes[i] << endl;
		outputfile << endl;
		outputfile << ",";
	}
	outputfile.close();
	return 0;
}

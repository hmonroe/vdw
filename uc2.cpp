#ifdef _WIN32
#include "boinc_win.h"
#else
#include "config.h"
#include <cstdio>
#include <cctype> 
#include <ctime>
#include <cstring>
#include <cstdlib>
#include <csignal>
#include <unistd.h>
#endif


#include "str_util.h"
#include "util.h"
#include "filesys.h"
#include "boinc_api.h"
#include "mfile.h"
#include "graphics2.h"
#include "uc2.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cmath>

using namespace std;

using std::string;

#define CHECKPOINT_FILE "upper_case_state"
#define INPUT_FILENAME "in"
#define OUTPUT_FILENAME "out"

bool run_slow = false;
bool early_exit = false;
bool early_crash = false;
bool early_sleep = false;
bool trickle_up = false;
bool trickle_down = false;
bool critical_section = false;    // run most of the time in a critical section
bool report_fraction_done = true;
bool network_usage = false;
double cpu_time = 0, comp_result;

//returns the result of a to be b power using the binary representation of b
// Based orginally on code by Mark Lotts
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
	for (i = 1; i < b + 1; i++) {
		power = (power * a) % prime;
	}
	return (power % prime);
}

//finds the least primitive root of parameter prime
// Based orginally on code by Mark Lotts
long long getFactorRoot(long long prime) {
	long long primefactors[256]; // number of prime factors is less than number of bits
	long long guess, curNumber, i, foundone;
	long long factorcount, curPrime, factordown;
	factordown = (prime - 1);
	factorcount = 0;
	for (curPrime = 2;; curPrime++) {
		bool isPrime = true;
		for (long long possibleFactor = 2; possibleFactor < sqrt((long double)curPrime) + 1; possibleFactor++) {
			if (curPrime % possibleFactor == 0 && curPrime != 2) {
				isPrime = false;
				break;
			}
		}
		if (!isPrime)
			continue;
		if ((factordown % curPrime == 0)) {
			primefactors[factorcount] = curPrime;
			factorcount++;
			while (factordown % curPrime == 0) {
				factordown = factordown / curPrime;
			}
		}

		if (factordown > 1 && curPrime > sqrt((long double)factordown)  + 1) {
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

// Code written by Daniel Monroe
int main(int argc, char **argv) {
	int i;
	int retval;
	double fsize;
	char input_path[512], output_path[512], buf[256];
	MFILE out;
	FILE* infile;

	for (i=0; i<argc; i++) {
		if (strstr(argv[i], "early_exit")) early_exit = true;
		if (strstr(argv[i], "early_crash")) early_crash = true;
		if (strstr(argv[i], "early_sleep")) early_sleep = true;
		if (strstr(argv[i], "run_slow")) run_slow = true;
		if (strstr(argv[i], "critical_section")) critical_section = true;
		if (strstr(argv[i], "network_usage")) network_usage = true;
		if (strstr(argv[i], "cpu_time")) {
			cpu_time = atof(argv[++i]);
		}
		if (strstr(argv[i], "trickle_up")) trickle_up = true;
		if (strstr(argv[i], "trickle_down")) trickle_down = true;
	}
	retval = boinc_init();
	if (retval) {
		fprintf(stderr, "%s boinc_init returned %d\n",
			boinc_msg_prefix(buf, sizeof(buf)), retval
			);
		exit(retval);
	}
	boinc_resolve_filename(INPUT_FILENAME, input_path, sizeof(input_path));
	long long thisPrime=0;
	long long interval =1;
	ifstream inputfile(input_path);
	if (inputfile.is_open()) {
		if (inputfile) {
			string s;
			if (getline(inputfile, s)) {
				istringstream ss(s);
				if(getline(ss, s, ',')) {
					thisPrime = atoi(s.c_str());
					if(getline(ss, s, ',')) {
						interval = atoi(s.c_str());
					}
				}
			}
		}
		inputfile.close();
	}
	if(interval==0) interval=1;
	infile = boinc_fopen(input_path, "r");
	if (!infile) {
		fprintf(stderr,
			"%s Couldn't find input file, resolved name %s.\n",
			boinc_msg_prefix(buf, sizeof(buf)), input_path
			);
		exit(-1);
	}
	file_size(input_path, fsize);
	int colors = 11;
	long long bestPrimes[100][13];
	for (i = 0; i < 100; i++)
		for (int numberofcolors = 0; numberofcolors < colors; numberofcolors++)
			bestPrimes[i][colors] = 0;
	for (long long possiblePrime = thisPrime; possiblePrime <thisPrime+interval; possiblePrime++) {
		bool isPrime = true;
		for (long long possibleFactor = 2; possibleFactor*possibleFactor < possiblePrime+2; possibleFactor++) {
			if (possiblePrime % possibleFactor == 0) {
				isPrime = false;
				break;
			}
		}
		if (isPrime) {
			long long* powers = new long long[(unsigned int) possiblePrime + 20];
			long long possibleRoot = getFactorRoot(possiblePrime);
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
							} else {
								if (finalLength[numberofcolors] == 0)
									sequenceFromOne[numberofcolors] = currentLength[numberofcolors];
								finalLength[numberofcolors] = max(finalLength[numberofcolors], currentLength[numberofcolors]);
								currentLength[numberofcolors] = 1;
							}
						}
					}
					for (numberofcolors = 2; numberofcolors < colors; numberofcolors++) {
						if ((possiblePrime % numberofcolors) != 1)
							continue;
						long long length = 0;
						length = finalLength[numberofcolors] + 1;
						if ((powers[possiblePrime - 1] % numberofcolors) == 0) {
							length = max(finalLength[numberofcolors] + 1, sequenceFromOne[numberofcolors] * 2 + 2);
						} else {
							length = max(finalLength[numberofcolors] + 1, sequenceFromOne[numberofcolors] + 2);
						}
						if (length < 24 && possiblePrime > bestPrimes[length][numberofcolors]) {
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

	boinc_resolve_filename(OUTPUT_FILENAME, output_path, sizeof(output_path));
	ofstream outputfile(output_path);
	if (outputfile.is_open()) {
		if (outputfile.is_open()) {
			outputfile << thisPrime<<","<<interval << endl;
			//outputfile << "test" << endl;
			fprintf(stderr,"%llu,%llu\n", thisPrime,interval);
		}
		for ( i = 3; i < 24; i++) {
			for (int numberofcolors = 2; numberofcolors < colors; numberofcolors++) {
				outputfile << bestPrimes[i][numberofcolors];
				outputfile << ",";
				fprintf(stderr,"%llu,",bestPrimes[i][numberofcolors]);

			}
			outputfile << endl;
			fprintf(stderr, "\n");
		}
	}
	outputfile.close();
	boinc_fraction_done(1);
	boinc_finish(0);
}

#ifdef _WIN32
int WINAPI WinMain(
	HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR Args, int WinMode
	) {
		LPSTR command_line;
		char* argv[100];
		int argc;

		command_line = GetCommandLine();
		argc = parse_command_line(command_line, argv);
		return main(argc, argv);
}
#endif

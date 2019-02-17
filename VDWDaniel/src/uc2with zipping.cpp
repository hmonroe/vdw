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

//structure that is returned by progression checking methods
struct progCheckReturn {
	//whether or not a progression is found
	int foundProgression;
	//the index where the progression starts
	int initIndex;
	//the difference between progression elements
	int difference;
};

//so the progCheckReturn structures can be called CHECKRETURN
typedef struct progCheckReturn CHECKRETURN;

/*
 * This function takes a power-residue coloring for a given prime
 * and power and zips it to an extended coloring.
 * Parameters:
 *    prcolors - array containing the power-residue coloring
 *    target - array to be filled with the extended coloring
 *    prime - underlying prime used in creating initial coloring
 *    power - power used in deterimining the initial coloring
 */

void rezipSequence(char prcolors[], char target[], long prime, //long length,
		int power) {
	int i, j;

	//this code just filled the zipped array with 9's in order to see
	//how the other parts of the code were working

//	for (i = 0; i < (2 * length); ++i) {
//		target[i] = 9;
//	}

	j = 0;

	for (i = 0; i < (2 * prime); ++i) {
		target[j] = prcolors[i];
		j += 2;
	}

	//odd indices from 1 to the midpoint of the zipped array are the
	//first half of the original coloring in reverse order
	j = 1;

	//printf("\n\nOdd Numbered Entries:\n");
	for (i = prime - (prime / 2); i < (2 * prime); ++i) {
		target[j] = (prcolors[i] + (power / 2)) % power;
		j += 2;
		//	printf("%d",((prcolors[i] + (power/2)) % power));
	}

	//odd indices from the midpoint of the zipped array to the end are
	//the second half of the original coloring in reverse order

	//printf("\n");
	for (i = 0; i < prime - (prime / 2); ++i) {
		target[j] = (prcolors[i] + (power / 2)) % power;
		j += 2;
		//	printf("%d",((prcolors[i] + (power/2)) % power));
	}

	/*printf("\n\nZipped Sequence: ");

	 for(i = 0; i < (2*prime); ++i)
	 {
	 printf("%d",target[i]);
	 }
	 */
}

/* the destination is where the progression is passed in to the
 *
 */
CHECKRETURN progressionChecker(char destination[], long prime,
		int progression) {
	CHECKRETURN returnstruct;

	/* valid is 1 if the progression is not found, 0 if otherwise
	 * startscan is the index of the array where the program starts to
	 *   look for a progression for a given element in the array
	 * curdiff is the distance between elements in the array for the
	 *   current progression that is being tested
	 */
	int valid, startscan, curdiff, nextstep, i, j;

	valid = 1;

	/* if for some reason the user wants to look for a two or one
	 * progression, it will always exist for sequence made from a power
	 * residue coloring
	 */
	if (progression <= 2) {
		//printf("\nThere is a %d progression\n\n",progression);
		returnstruct.foundProgression = 1;
		return returnstruct;
		//return 1;
	} else {
		/* check each element in the array to see if it is in an invalid
		 * progression
		 */
		for (i = 0; i < (prime - 1 - (progression - 1)); ++i) {
			//printf("\nLooking at index number %d, which is %d...\n",i,destination[i]);
			/* always start looking for a progression at the position
			 * directly after the current element being examined
			 */
			startscan = i + 1;

			//make sure startscan stays in the bounds of the array
			while (startscan <= (prime - 1)) {
				//if the elements are equal, we have the beginning of a
				//progression
				if (destination[i] == destination[startscan]) {
					//printf("Next possibility at %d, which is %d\n",startscan,destination[startscan]);

					//calculate the difference between the two elements
					//found so far
					curdiff = startscan - i;

					//nextstep is how far away from the intial progression
					//element the next element to check will be
					nextstep = curdiff;

					//printf("Looking at difference of %d\n", curdiff);
					//curdiff = startscan - i;

					//this loop will count if the progression is long
					//enough to be invalid
					for (j = 2; j < progression; ++j) {
						//if ((startscan + curdiff) < (prime-1))

						//just code here for the checking statement
						//if ((startscan + nextstep) <= (prime-1))
						//{
						//printf("Next number in sequence is at index %d and is %d\n", (startscan+nextstep/*curdiff*/), destination[startscan+nextstep/*curdiff*/]);
						//}

						//if the next element in the progression is a valid
						//progression element
						if (((startscan + nextstep) <= (prime - 1))
								&& (destination[startscan]
										== destination[startscan + nextstep])) {
							//calculate where the next element would be
							nextstep += curdiff;

							/* valid is now 0 since we are in what might
							 * become an invalid progression (it will change
							 * back to 1 if the progression is short enough to
							 * remain within the given length
							 */
							valid = 0;
						}

						/* if the next element in the progression is not a
						 * valid element, then we are done checking this
						 * progression and we can break out of the for loop
						 * and make valid 1 again
						 */
						else {
							valid = 1;
							break;
						}
					}

					//if the for loop ended and valid is still 0, then an
					//invalid progression was found
					if (valid == 0) {
						//break out of the while loop since we know that
						//this sequence has an invalid progression
						break;
					}
					//if the progression ended and was short enough
					else {
						//start looking for a progression and the next array
						//index
						startscan++;
					}
				} else {
					startscan++;
				}
			}
			//break out of the larger for loop when an invalid
			//progression was found
			if (valid == 0) {
				break;
			}
		}

		//could just write return ! valid but I like the readability of
		//this code better
		if (valid == 1) {
			//printf("\nNo %d-progression found\n",progression);
			returnstruct.foundProgression = 0;
			returnstruct.difference = curdiff;
			returnstruct.initIndex = i;
			return returnstruct;
		} else {
			//printf("\n%d-progression found, starts at position %d with a difference of %d\n\n",progression,i,(curdiff/*/(progression-1)*/));
			returnstruct.foundProgression = 1;
			returnstruct.difference = curdiff;
			returnstruct.initIndex = i;
			return returnstruct;
		}
	}
}

//ASSUMES THAT ONLY SINGLE ZIPPING IS DONE (i.e. THAT ZIPPED SEQUENCE IS 2p!!!
//check for progressions with successive elements
CHECKRETURN inARow(char destination[], long length, int progression,
		long prime) {
	//is eventually returned with results of what is found
	CHECKRETURN returnstruct;
	//congruent is 1 if the prime is congruent to 1 mod 4
	int i, cur, congruent, success;

	cur = 1;
	congruent = 0;
	success = 0;

	//if prime is congruent to 1 mod 4
	if (prime % 4 == 1) {
		congruent = 1;
	}

	//if prime is congruent to 1 mod 4
	if (congruent == 1) {
		//are 1,2,...(l-1)/2 in the same class?
		for (i = 1; i < ((progression - 1) / 2); ++i) {
			//if the beginning of the progression is not invalid
			if (!(destination[i] == destination[i + 1])) {
				success = 1;
				break;
			}
		}
	}
	//if prime is not congruent to 1 mod 4
	else {
		//are 1,2,...(l-1) in the same class?
		for (i = 1; i < (progression - 1); ++i) {
			if (!(destination[i] == destination[i + 1])) {
				success = 1;
				break;
			}
		}
	}

	//THERE WAS A PROBLEM WITH THE BEGINNING OF THE ARRAY
	if (success != 1) {
		//set up diagnostic info and return the structure
		returnstruct.foundProgression = 1;
		returnstruct.difference = 1;
		//-1 for initIndex means there was a problem at the beginning
		returnstruct.initIndex = -1;
		return returnstruct;
	}

	//printf("\nCommencing string checking\n");
	//printf("Sequence Length: %d\nProgression:	%d\n",length,progression);
	//printf("%d %d\n",destination[1],destination[618]);

	//check for strings
	for (i = 0; i < (length - 1); ++i) {
		//IS THERE SYMMETRY IN THE ZIPPED SEQUENCE???
		if (i < ((2 * prime) - 1)) {
			//printf("i = %d\ni+prime = %d\n",i,(i+prime)%(2*prime));

			//IF DOUBLE ZIPPING, USE THIS IF STATEMENT instead of the below
			//if (destination[i] == destination[(i+(2*prime))%(4*prime)])

			//IF SINGLE ZIPPING, USE THIS IF STATEMENT
			if (destination[i] == destination[(i + prime) % (2 * prime)]) {
				//partition is invalid since the symmetry will create
				//progressions
				//printf("Invalid Symmetry\n");
				success = 0;
				break;
			}
		}

		//if there is are back-to-back numbers in the same kth power
		//class then a progression string is present
		if (destination[i] == destination[i + 1]) {
			//add 1 to cur, which represents the length of the
			//progression
			cur++;
			//if there is a progression that is equal to the progression
			//length that needs to be avoided, then the partition is
			//invalid
			if (cur == progression) {
				break;
			}
		}
		//otherwise, the progression length counter is reset to 1
		else {
			cur = 1;
		}
	}
	//printf("i = %d\n",i);

	//if success is 0 then the partition has invalid symmetry
	if (success == 0) {
		returnstruct.foundProgression = 1;
		returnstruct.difference = prime;
		returnstruct.initIndex = i;

		return returnstruct;
	}

	//if cur is equal to progression then there was an invalid
	//progression string found
	if (cur == progression) {
		//printf("String Found\n");
		returnstruct.foundProgression = 1;
		returnstruct.difference = 1;
		returnstruct.initIndex = i - (progression - 1);
	}
	//otherwise the partition was valid
	else {
		//printf("String not found\n");
		//printf("cur = %d\nprogression = %d\n",cur,progression);
		returnstruct.foundProgression = 0;
	}

	//return the structure
	return returnstruct;
}

//take the zipped sequence, the length of the zipped sequence, and the
//progression length, and then build the entire partition and store it
//in the partition[] array
void buildPartition(char zipped[], char partition[], long length, int prog) {
	//loop counter
	int i;

	//build the partition composed of (prog-1) multiples of the zipped
	//progression (that has length length)
	for (i = 0; i < (length * (prog - 1)); ++i) {
		partition[i] = zipped[(i % length)];
	}

	//put the last number in the partition
	partition[i] = 0;

	printf("\nPartition of Length %ld\n", (length * (prog - 1)) + 1);
}
void zipSequence(char prcolors[], char target[], long prime, int power) {
	int i, j, offset;
//	long m;

	//SETTING THE GLUE POINTS

	//the color of the first member of the two parts that compose the zipped
	//coloring can be arbitrary, but these should be chosen so as not to form a
	//monochromatic progression amongst themselves
	target[0] = 1;
	target[prime] = (1 + (power / 2)) % power;

	//printf("\nOriginal Coloring:\n");
	//for(i = 0; i < prime-1; i++)
	//	printf("%d", prcolors[i]);

	//SPREADING

	//even indices of the zipped coloring are simply the numbers of
	//the original coloring
	j = 2;

	for (i = 0; i < (prime - 1); ++i) {
		target[j] = prcolors[i];
		j += 2;
	}

	offset = power / 2;

	//TURNING, SHIFTING, AND MERGING

	//odd indices from 1 to the midpoint of the zipped array are the
	//first half of the original coloring in reverse order
	j = 1;
	//printf("\n\nOdd Numbered Entries:\n");
	for (i = (prime - 1) / 2; i < (prime - 1); ++i) {
		target[j] = (prcolors[i] + offset) % power;
		j += 2;
		//printf("%d",((prcolors[i] + offset) % power));
	}

	//odd indices from the midpoint of the zipped array to the end are
	//the second half of the original coloring in reverse order
	j += 2;
	//printf("\n");
	for (i = 0; i < (prime - 1) / 2; ++i) {
		target[j] = (prcolors[i] + offset) % power;
		j += 2;
		//printf("%d",((prcolors[i] + offset) % power));
	}

}

void getPowerProgression(int power, int root, long prime, char dest[]) {
	int i;
	long long target;

	target = 1;

	for (i = 1; i < prime; ++i) {
		//get power of root
		target = ((target % prime) * root) % prime;
		dest[target - 1] = i % power;
	}
}

/* this method examines a particular power and progression and tries
 * to find a prime that can be zipped in order to create a better
 * progression-free partition
 */
int zipit(int power, int prog, long prime, int root) {
//	fprintf(stderr,"%d,%d,%d,%lu\n", power, prog,root,prime);

	int zip2, shortcut;
	if( prime> 40000000 || prog>18) {
		return 0;
	}
	//this is 0 if only one zip is to be done, 1 if two zips
	zip2 = 0;
	

	char* zipped = new char[2 * prime + 1];
	char* zipped2;
	if(zipped2!=0)
		zipped2= new char[4 * prime + 1];
	char* partition = new char[((zip2+1) * prime) * (prog - 1) + 1];
	char* destination = new char[prime];

	CHECKRETURN progret, inarowret;

	//this is 0 if a full partition check is to be done, 1 if only
	//string checking is going to be done on the zipped sequence
	shortcut = 1;

	//get the power progression and store it in destination
	getPowerProgression(power, root, prime, destination);

	//arithmetically zip the sequence and store the result in the
	//zipped array
	zipSequence(destination, zipped, prime, power);

	if (zip2 != 0) {
			rezipSequence(zipped,zipped2,prime,power);
	}
	//if the full partition is going to be checked, it has to be
	//built
	if (shortcut == 0) {
		//build partition twice as long as p when single zipping
		if (zip2 == 0) {
			buildPartition(zipped, partition, (prime * 2), prog);
		} else {
				buildPartition(zipped2,partition,(prime*4),prog);
		}
	}

	/* check to see if the zipped progression has an arithmetic
	 * progression of prog length, result is a 1 if there is a
	 * progression, 0 if there is not a progression
	 */

	//if a full partition check is going to be done
	if (shortcut == 0) {
		//single zipping
		if (zip2 == 0) {
			//check partition for progression strings
			inarowret = inARow(partition, (prime * 2) * (prog - 1) + 1, prog,
					prime);
			//print out result
			if (inarowret.foundProgression == 1) {
//				printf("Found string\n");
			} else {
//				printf("No string\n");
			}
			//check partition for any progressions
			progret = progressionChecker(partition,
					(prime * 2) * (prog - 1) + 1, prog);
		}
		//double zipping
		else {
			inarowret = inARow(partition, (prime * 4) * (prog - 1) + 1, prog,
					prime);
			if (inarowret.foundProgression == 1) {
//				printf("Found string\n");
//				printf("\nFailure: %d-string found in zipped sequence\n", prog);
//				printf("Initial Index - %d\n", inarowret.initIndex);
//				printf("Common Difference - %d\n", inarowret.difference);
			} else {
//				printf("No string\n");
			}
			progret = progressionChecker(partition,
					(prime * 4) * (prog - 1) + 1, prog);
		}

		//if string checking says a partition is valid but the full
		//check finds a progression that is not a string, string
		//checking is obviously insufficient
		if ((inarowret.foundProgression == 0)
				&& (progret.foundProgression == 1)) {
//			printf("ERROR: Checking strings was not sufficient\n");
		}
		//if no progression is found
		if (progret.foundProgression == 0) {
			//if a string progression is found but the full checker
			//does not find a progression, then something is wrong
			if (inarowret.foundProgression == 1) {
//				printf("ERROR: Consecutive progression found incorrectly\n");
			}
			//break out of the loop since a progression has been found
			//break;
		}
		//if a progression is found
		if (progret.foundProgression == 1) {
			//print out all of the diagnostic info for the progression
//			printf("\nFailure: %d-progression found in zipped sequence\n",
//					prog);
//			printf("Initial Index - %d\n", progret.initIndex);
//			printf("Common Difference - %d\n", progret.difference);
			//print out diagnostic info for a string progression if
			//one is found
			if (inarowret.foundProgression == 1) {
//				printf("\nFailure: %d-string found in zipped sequence\n", prog);
//				printf("Initial Index - %d\n", inarowret.initIndex);
//				printf("Common Difference - %d\n", inarowret.difference);

			}
		}
	}
	//if only the zipped sequence is going to be checked for strings
	else {
		//single-zipping
		if (zip2 == 0) {
			inarowret = inARow(zipped, prime * 2, prog, prime);
		}
		//string-checking not sufficient for double-zipping
		else {
			inarowret = inARow(zipped2,prime*4,prog,prime);
		}

		//if no progression was found, exit the program
		if (inarowret.foundProgression == 0) {
			//break;
		}
		//print out diagnostic info for a progression if one is found
		else {
//			printf("\nFailure: %d-string found in zipped sequence\n", prog);
//			printf("Initial Index - %d\n", inarowret.initIndex);
//			printf("Common Difference - %d\n", inarowret.difference);
		}
	}


//	printf("\n");
	delete[] destination;
	destination = NULL;
	delete[] partition;
	partition = NULL;
	delete[] zipped;
	zipped = NULL;
	if(zip2!=0) {
	delete[] zipped2;
	zipped2 = NULL;
	}
	//print out the results of the checker
	if ((progret.foundProgression == 1) || (inarowret.foundProgression == 1)) {
//		printf("\nFailure: The prime was not able to be zipped\n");
		return 0;
	} else {
//		printf("\nSuccess: No %d-progression found in zipped sequence\n", prog);
//		printf("\nThe prime used was %ld\n", prime);
		return 1;
	}

}


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
	int rows = 26;
	char input_path[512], output_path[512], buf[256];
	MFILE out;
	FILE* infile;
	int zip=1;

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
			bestPrimes[i][numberofcolors] = 0;
	for (long long possiblePrime = thisPrime; possiblePrime <thisPrime+interval; possiblePrime++) {
		bool isPrime = true;
		for (long long possibleFactor = 2; possibleFactor*possibleFactor < possiblePrime+2; possibleFactor++) {
			if (possiblePrime % possibleFactor == 0) {
				isPrime = false;
				break;
			}
		}
		if (isPrime) {
//			__int16* powers = new __int16[(unsigned int) possiblePrime + 20];
//			int* powers = new int[(unsigned int) possiblePrime + 20];
			short* powers = new short[(unsigned int) possiblePrime + 20];
			long long possibleRoot = getFactorRoot(possiblePrime);
			{
				powers[possibleRoot] = 1;
				long long accumulatedPower = possibleRoot;
				bool isPrimitive = true;
				for (long exponent = 2; exponent <= possiblePrime - 1; exponent++) {
					accumulatedPower = (possibleRoot * accumulatedPower) % possiblePrime;
					if ((accumulatedPower == 1) && (exponent < possiblePrime - 1)) {
						isPrimitive = false;
						break;
					}
					powers[accumulatedPower] = exponent % 2520;
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
						if (length < rows && possiblePrime > bestPrimes[length][numberofcolors]) {
							// found best prime
							bestPrimes[length][numberofcolors] = possiblePrime;
						}
						if (numberofcolors % 2 == 0 && zip == 1 && length < rows && possiblePrime * 2 > bestPrimes[length][numberofcolors]) {
//							fprintf(stderr,"%llu,%llu,%d,%d\n",  possiblePrime, possibleRoot,numberofcolors,length);
							if(	zipit((int)numberofcolors, (int)length, (long)possiblePrime, (int)possibleRoot)) 
								bestPrimes[length][numberofcolors] = possiblePrime * 2;
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
			fprintf(stderr,"%llu,%llu\n", thisPrime,interval);
		}
		for ( i = 3; i < rows; i++) {
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

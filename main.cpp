#include <iostream>
#include <math.h>

using namespace std;

int main() {
	long startValue = 3;
	for (long possiblePrime = startValue; possiblePrime < startValue + 999;
			possiblePrime++) {
		bool isPrime = true;
		for (long possibleFactor = 2; possibleFactor < sqrt(possiblePrime) + 1;
				possibleFactor++)
			if (possiblePrime % possibleFactor == 0)
				isPrime = false;
		if (isPrime) {
			for (long possibleRoot = 2; possibleRoot < possiblePrime;
					possibleRoot++) {
				int powers[possiblePrime];
				long accumulatedPower = 1;
				bool isPrimitive = true;
				for (long exponent = 1; exponent < possiblePrime - 1;
						exponent++) {
					accumulatedPower = (possibleRoot * accumulatedPower)
							% possiblePrime;
					//cout << accumulatedPower << " ";
					powers[exponent] = accumulatedPower % 2;
					if (accumulatedPower == 1) {
						cout << " " << possibleRoot
								<< " is not a primitive root of "
								<< possiblePrime << "." << endl;
						isPrimitive = false;
						break;
					}

				}
				if (isPrimitive) {
					cout << possibleRoot << " is a primitive root of "
							<< possiblePrime << "." << endl;
					for (long position = 1; position < possiblePrime - 1;
							position++) {
						cout << powers[position];
					}
					cout<<endl;

					break;
				}
			}
		}
	}
	return 0;
}

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>

using namespace std;

int mainx() {
	ifstream inputfile;
	ofstream outputfile;
	long long bestprimes[100][100];
	inputfile.open("vdwtable.txt", ifstream::in);
	long long startrange = 2;
	if (inputfile.is_open()) {
		if (inputfile) {
			string s;
			if (getline(inputfile, s)) {
				istringstream ss(s);
				startrange = atoi(s.c_str());
				//		cout << "Val:" << startrange << endl;
				cout << "Startrange: " << startrange << endl;
			}
		}
		int lengths = 3;
		int color = 2;
		while (inputfile) {
			string s;
			if (!getline(inputfile, s)) {
				cout << "Here ";
				break;
			}
			cout << "Length " << lengths << ":";
			istringstream ss(s);
			while (ss) {
				if (!getline(ss, s, ',')) {
					color = 2;
					cout << endl;
					break;
				}
				cout << " " << atoi(s.c_str()) << "(" << color << ")";
				bestprimes[color][lengths] = atoi(s.c_str());
				color++;
			}
			lengths++;
		}
		inputfile.close();
	} else {
		cout << "Error opening file";
	}
	return 0;
}

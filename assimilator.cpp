// This file is a part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

// A sample assimilator that:
// 1) if success, copy the output file(s) to a directory
// 2) if failure, append a message to an error log

#include <vector>
#include <string>
#include <cstdlib>

#include "boinc_db.h"
#include "error_numbers.h"
#include "filesys.h"
#include "sched_msgs.h"
#include "validate_util.h"
#include "sched_config.h"
#include <iostream>
#include <math.h>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
using namespace std;

using std::vector;
using std::string;

int write_error(char* p) {
    static FILE* f = 0;
    if (!f) {
        f = fopen(config.project_path("sample_results/errors"), "a");
        if (!f) return ERR_FOPEN;
    }
    fprintf(f, "%s", p);
    fflush(f);
    return 0;
}

int assimilate_handler(
    WORKUNIT& wu, vector<RESULT>& results, RESULT& canonical_result
) {
    int retval;
    unsigned int i;

    retval = boinc_mkdir(config.project_path("sample_results"));
    int colors = 11;
    long bestPrimes[27][13]={{0}};
    int j;
    int numberofcolors;
    int thisPrime=0;
    if (retval) return retval;

    if (wu.canonical_resultid) {
        vector<OUTPUT_FILE_INFO> output_files;
        get_output_file_infos(canonical_result, output_files);
        unsigned int n = output_files.size();
        for (i=0; i<n; i++) {
            OUTPUT_FILE_INFO& fi = output_files[i];
    ifstream inputfile;
    inputfile.open("/home/boincadm/projects/vdwnumbers/html/user/output.txt");
    int lengths = 3;
    if (inputfile.is_open()) {
      if (inputfile) {
	string s;
	if (getline(inputfile, s)) {
	  istringstream ss(s);
	  thisPrime = atoi(s.c_str());
	}
      }
       numberofcolors = 2;
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
    ifstream inputfilethis;
    inputfilethis.open(fi.path.c_str());
    int lengthsthis = 3;
    if (inputfilethis.is_open()) {
      if (inputfilethis) {
	string s;
	if (getline(inputfilethis, s)) {
	  istringstream ss(s);
	  thisPrime = atoi(s.c_str());
	}
      }
       numberofcolors = 2;
      while (inputfilethis) {
	string s;
	if (!getline(inputfilethis, s)) {
	  break;
	}
	istringstream ss(s);
	while (ss) {
	  if (!getline(ss, s, ',')) {
	    numberofcolors = 2;
	    break;
	  }
	    long thisPrimeResult= atoi(s.c_str());
	    if(thisPrimeResult>bestPrimes[lengthsthis][numberofcolors])// &&thisPrimeResult<thisPrime*5)
	      bestPrimes[lengthsthis][numberofcolors]=thisPrimeResult; 
	  numberofcolors++;
	}
	lengthsthis++;
      }
      inputfilethis.close();
    }

    ofstream outputfile;
    outputfile.open("/home/boincadm/projects/vdwnumbers/html/user/output.txt");
    if (outputfile.is_open()) {
      if (outputfile.is_open())
	outputfile << thisPrime << endl;

      for (j = 3; j < 26; j++) {
	for (numberofcolors = 2; numberofcolors < colors; numberofcolors++) {
	  outputfile << bestPrimes[j][numberofcolors];
	  outputfile << ",";
	}
	outputfile << endl;
      }
    }
    outputfile.close();

            }
        }
    return 0;
}

#include <iostream>
#include <fstream>
#include <sstream>

#include <sys/param.h>
#include <unistd.h>
#include <cstdlib>
#include <string>
#include <cstring>

#include "backend_lib.h"
#include "boinc_db.h"
#include "error_numbers.h"
#include "filesys.h"
#include "parse.h"
#include "str_replace.h"
#include "str_util.h"
#include "svn_version.h"
#include "util.h"
#include <math.h>

#include "sched_config.h"
#include "sched_util.h"
#include "sched_msgs.h"
using namespace std;

#define CUSHION 1000
    // maintain at least this many unsent results
#define REPLICATION_FACTOR  2
    // number of instances of each job

const char* app_name = "example_app";
const char* in_template_file = "example_app_in";
const char* out_template_file = "example_app_out";

char* in_template;
DB_APP app;
int start_time;
int seqno;

// create one new job
//
int make_job() {
    DB_WORKUNIT wu;
    char name[256], path[MAXPATHLEN];
    const char* infiles[1];
    int retval;

    // VDW
    long long lastPrime=3;
    ifstream inputfile;
    inputfile.open("/home/boincadm/projects/vdwnumbers/html/user/lastprime.txt");
    if (inputfile.is_open()) {
      if (inputfile) {
	string s;
	if (getline(inputfile, s)) {
	  istringstream ss(s);
	  lastPrime = atoi(s.c_str());
	}
      }
      inputfile.close();
    }
    long long possiblePrime;
    for (possiblePrime = lastPrime+1;; possiblePrime++) {
      bool isPrime = true;
      for (long long possibleFactor = 2; possibleFactor < sqrt(possiblePrime) + 1; possibleFactor++)
	if (possiblePrime % possibleFactor == 0) {
	  isPrime = false;
	  break;
	}
      if(isPrime)
	break;
    }
    int interval = 250;


    // make a unique name (for the job and its input file)
    //
    sprintf(name, "ZipPrimes_%llu_plus_%d_%d_%d", possiblePrime,interval, start_time, seqno++);

    // Create the input file.
    // Put it at the right place in the download dir hierarchy
    //
    retval = config.download_path(name, path);
    if (retval) return retval;
    FILE* f = fopen(path, "w");
    if (!f) return ERR_FOPEN;
    fprintf(f, "%llu,%d", possiblePrime,interval);
    fclose(f);

    lastPrime=possiblePrime+interval;

    FILE* ff = fopen("/home/boincadm/projects/vdwnumbers/html/user/lastprime.txt", "w");
    if (!ff) return ERR_FOPEN;
    fprintf(ff, "%llu", lastPrime);
    fclose(ff);
    // END VDW


    // Fill in the job parameters
    //
    wu.clear();
    wu.appid = app.id;
    safe_strcpy(wu.name, name);
    wu.rsc_fpops_est = 1e12;
    wu.rsc_fpops_bound = 1e16;
    wu.rsc_memory_bound = 1e8;
    wu.rsc_disk_bound = 1e8;
    wu.delay_bound = 500000;
    wu.min_quorum = REPLICATION_FACTOR;
    wu.target_nresults = REPLICATION_FACTOR;
    wu.max_error_results = REPLICATION_FACTOR*4;
    wu.max_total_results = REPLICATION_FACTOR*8;
    wu.max_success_results = REPLICATION_FACTOR*4;
    infiles[0] = name;

    // Register the job with BOINC
    //
    sprintf(path, "templates/%s", out_template_file);
    return create_work(
        wu,
        in_template,
        path,
        config.project_path(path),
        infiles,
        1,
        config
    );
}

void main_loop() {
    int retval;

    while (1) {
        check_stop_daemons();
        int n;
        retval = count_unsent_results(n, app.id);
        if (retval) {
            log_messages.printf(MSG_CRITICAL,
                "count_unsent_jobs() failed: %s\n", boincerror(retval)
            );
            exit(retval);
        }
        if (n > CUSHION) {
            daemon_sleep(10);
        } else {
            int njobs = (CUSHION-n)/REPLICATION_FACTOR;
            log_messages.printf(MSG_DEBUG,
                "Making %d jobs\n", njobs
            );
            for (int i=0; i<njobs; i++) {
                retval = make_job();
                if (retval) {
                    log_messages.printf(MSG_CRITICAL,
                        "can't make job: %s\n", boincerror(retval)
                    );
                    exit(retval);
                }
            }
            // Wait for the transitioner to create instances
            // of the jobs we just created.
            // Otherwise we'll create too many jobs.
            //
            double now = dtime();
            while (1) {
                daemon_sleep(5);
                double x;
                retval = min_transition_time(x);
                if (retval) {
                    log_messages.printf(MSG_CRITICAL,
                        "min_transition_time failed: %s\n", boincerror(retval)
                    );
                    exit(retval);
                }
                if (x > now) break;
            }
        }
    }
}

void usage(char *name) {
    fprintf(stderr, "This is an example BOINC work generator.\n"
        "This work generator has the following properties\n"
        "(you may need to change some or all of these):\n"
        "  It attempts to maintain a \"cushion\" of 100 unsent job instances.\n"
        "  (your app may not work this way; e.g. you might create work in batches)\n"
        "- Creates work for the application \"example_app\".\n"
        "- Creates a new input file for each job;\n"
        "  the file (and the workunit names) contain a timestamp\n"
        "  and sequence number, so that they're unique.\n\n"
        "Usage: %s [OPTION]...\n\n"
        "Options:\n"
        "  [ --app X                Application name (default: example_app)\n"
        "  [ --in_template_file     Input template (default: example_app_in)\n"
        "  [ --out_template_file    Output template (default: example_app_out)\n"
        "  [ -d X ]                 Sets debug level to X.\n"
        "  [ -h | --help ]          Shows this help text.\n"
        "  [ -v | --version ]       Shows version information.\n",
        name
    );
}

int main(int argc, char** argv) {
    int i, retval;
    char buf[256];

    for (i=1; i<argc; i++) {
        if (is_arg(argv[i], "d")) {
            if (!argv[++i]) {
                log_messages.printf(MSG_CRITICAL, "%s requires an argument\n\n", argv[--i]);
                usage(argv[0]);
                exit(1);
            }
            int dl = atoi(argv[i]);
            log_messages.set_debug_level(dl);
            if (dl == 4) g_print_queries = true;
        } else if (!strcmp(argv[i], "--app")) {
            app_name = argv[++i];
        } else if (!strcmp(argv[i], "--in_template_file")) {
            in_template_file = argv[++i];
        } else if (!strcmp(argv[i], "--out_template_file")) {
            out_template_file = argv[++i];
        } else if (is_arg(argv[i], "h") || is_arg(argv[i], "help")) {
            usage(argv[0]);
            exit(0);
        } else if (is_arg(argv[i], "v") || is_arg(argv[i], "version")) {
            printf("%s\n", SVN_VERSION);
            exit(0);
        } else {
            log_messages.printf(MSG_CRITICAL, "unknown command line argument: %s\n\n", argv[i]);
            usage(argv[0]);
            exit(1);
        }
    }

    retval = config.parse_file();
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "Can't parse config.xml: %s\n", boincerror(retval)
        );
        exit(1);
    }

    retval = boinc_db.open(
        config.db_name, config.db_host, config.db_user, config.db_passwd
    );
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "can't open db\n");
        exit(1);
    }

    sprintf(buf, "where name='%s'", app_name);
    if (app.lookup(buf)) {
        log_messages.printf(MSG_CRITICAL, "can't find app %s\n", app_name);
        exit(1);
    }

    sprintf(buf, "templates/%s", in_template_file);
    if (read_file_malloc(config.project_path(buf), in_template)) {
        log_messages.printf(MSG_CRITICAL, "can't read input template %s\n", buf);
        exit(1);
    }

    start_time = time(0);
    seqno = 0;

    log_messages.printf(MSG_NORMAL, "Starting\n");

    main_loop();
}
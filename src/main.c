//
//  main.c
//  WDSat
//
//  Created by Gilles Dequen and Monika Trimoska on 15/12/2018.
//  Copyright © 2018 Gilles Dequen. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#ifdef _OPENMP
#include <omp.h>
#endif

#include "wdsat_utils.h"
#include "dimacs.h"
#include "cnf.h"
#include "xorset.h"
#include "xorgauss.h"
#include "wdsat.h"

extern byte_t thread_digits;

/// ompWTC is OpenMP Wall Clock Time
#ifdef _OPENMP

double wct_start;
double w_ct_end;

/**
 * @var int_t SATThreads
 * @brief This defines the number of threads that should partipate to the computation
 * within an OpenMP Context.
 */
int_t threads = 0;

#else

clock_t wct_start;
clock_t wct_end;

#endif

char input_filename[__STATIC_STRING_SIZE__] = "s_n4l2.dimacs";
char output_filename[__STATIC_STRING_SIZE__] = "out.cid";
char irr[160] = "11001";
char X3[160] = "0001";
int action = 2;
int n = 4;
int _l = 0;
int _m = 3;
int occ = 0;
int s = 0;
int xg = 0;
int br_sym = 0;
char mvc_graph[1000] = "";
char thread[1000] = "";
/**
 * @fn int scan_opt(int argc, char **argv, const char *opt)
 * @brief It scans and checks command line options
 */
int scan_opt(int argc, char **argv, const char *opt) {
	char c;
	while ((c = getopt (argc, argv, opt)) != -1)
		switch (c) {
			case 'a': if(!strncmp(optarg, "compile", 7)) action = 0; if(!strncmp(optarg, "time", 4)) action = 2; if(!strncmp(optarg, "delete", 6)) action = 3; if(!strncmp(optarg, "color", 5)) action = 4; break;
			case 'i': strcpy(input_filename, optarg); break;
			case 'o': strcpy(output_filename, optarg); break;
			case 'n': n = atoi(optarg); break;
			case 'l': _l = atoi(optarg); break;
			case 'm': _m = atoi(optarg); break;
			case 'x': xg = 1; break;
			case 'b': br_sym = 1; break;
			case 'c': occ = 1; break;
			case 's': s = 1; break;
			case 'g': strcpy(mvc_graph, optarg); break;
			case 't': strcpy(thread, optarg); break;
			default: return(-1);
		}
	return(0);
}

boolean_t generate(char *ifn, char *ofn) {
	FILE *d;
	
	if((d = fopen(ifn, "r")) == NULL) {
		_cid_cout("%s :: unkown filename", ifn);
		return(__OFF__);
	}
	/// read the header if exists
	dimacs_read_header(d);
	/// read the formula
	dimacs_read_formula(d);
	fclose(d);
	/// if there is no heading
	/// then create one
	//if(!dimacs_is_header_read()) dimacs_generate_meaning();
	//_cid_cout("#atoms [boolean equations] : %lld\n", dimacs_nb_atoms());
	//_cid_cout("#atoms [xor equations]     : %lld\n", dimacs_nb_xor_atoms());
	//_cid_cout("size of xor gauss          : %lld\n", __SZ_GAUSS__);
	
	if(_l == 0)
		_l = n;
	wdsat_solve(n ,_l, _m, irr, X3, xg, mvc_graph, thread);
	return(__ON__);
}

int main(int argc, char * argv[]) {
	byte_t exit_value = (byte_t) EXIT_SUCCESS;
	
	char *syntax =
	"c          -x : to enable Gaussian Elimination\n"
	"c          -i file    : where file is the input file\n"
	"c          -g mvc    : where mvc is a string of comma-separated variables that defines statically the branching order\n"
	"c          -h : help (shows the argument list)\n"
	;
	
	goto on_continue;
on_break:
#ifdef _OPENMP
	thread_digits = fast_int_log10(omp_get_max_threads());
	printf("c Available Cores: %*d (%*d)\n", thread_digits, omp_get_num_procs(), thread_digits, omp_get_max_threads());
#endif
	printf("c Syntax: %s <... Args ...>\n", argv[0]);
	printf("c Args:\n");
	printf("%s", syntax);
	printf("\n");
	exit_value = 1;
	goto end;
	
on_continue:
	if(scan_opt(argc, argv, "i:o:a:n:l:xbcshm:g:t:")) goto on_break;
	
	
	if(!generate(input_filename, output_filename)) {
		exit_value = (byte_t) EXIT_FAILURE;
	}
	
end:
	//printf("\n");
	return((int) exit_value);
}

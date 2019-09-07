#include <kernel.h>
#include <proc.h>
#include <q.h>
#include "math.h"

double pow(double x, int y) {
	double result = 1;
	while(y) {
		if(y & 1)
			result = result*x;
		y = y >> 1;
		x = x*x;
	}	
	return result;
}

double log(double x) {
	double l = 1 - x;
        double result = 0;
        int i = 1;
        for(i = 1; i <= 21; i++) {
                result = result + pow(l, i)/i;
        }
        return (0 - result);
}

double expdev(double lambda) {
	double dummy;
	do {
		dummy = (double) rand() / RAND_MAX;
	} while(dummy == 0.0);
	int ret =  (int)(-log(dummy)/lambda);
	return ret;
}



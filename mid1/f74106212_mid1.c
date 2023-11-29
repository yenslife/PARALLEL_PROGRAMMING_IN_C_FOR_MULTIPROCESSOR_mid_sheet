#include<stdio.h>

int main () {
	double Tp = 0.87;
	double Ts = 0.13;

	double answer;

	// 阿母答爾speedup = 1/(Ts + Tp/p)
	// maximun means p = inf

	answer = 1/Ts;

	printf("%.2f", answer);

	return 0;

}

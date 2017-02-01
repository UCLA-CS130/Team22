#include <stdio.h>

int doNothing(){
	if (false){
		printf("wow\n");
	}
	return 7;
	printf("wow2\n");
}
bool doNothing2(){
	return false;
}

int main(){
	
	for (int i=0; i<100; i++){
		doNothing2();
	}
	printf("hello world\n");
	return 0;
}
#include <stdlib.h>
#include <stdio.h>

struct vector{

	size_t size;
	size_t len;
	int* num;

	vector() {
		
		size = 10;
		len = 0;
		num = (int*)malloc(size * sizeof(int));
	}

	int operator[](int i){
		return num[i];
	}

	~vector() {
		free(num);
	}

	//push back



};
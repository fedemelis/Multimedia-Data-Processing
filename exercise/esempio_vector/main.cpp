/*#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include <stdio.h>


int comp(const void* a, const void* b) {

	int ia = *(int*)a;
	int ib = *(int*)b;

	if (ia < ib)
	{
		return -1;
	}
	else if (ia > ib)
	{
		return 1;
	}
	else
	{
		return 0;
	}

}

struct vector {

	size_t size_;
	size_t capacity_;
	int* data_;

	vector() {
		size_ = 0;
		capacity_ = 10;
		data_ = (int*)malloc(capacity_ * sizeof(int));
	}


	~vector() {
		free(data_);
	}


	int operator[](size_t index) const {
		return data_[index];
	}

	size_t size() const{
		return size_;
	}
	


	void push_back(int n) {
	
		if (size_ == capacity_) {
			capacity_ *= 2;
			data_ = (int*)realloc(data_, capacity_ * sizeof(int));
		}
		data_[size_] = n;
		size_++;

	}

	void quicksort() const {
		qsort(data_, size_, sizeof(int), comp);
	}


};




int main(int argc, char*argv[]) {

	if (argc != 3){
		return -1;
	}

	FILE* input = fopen(argv[1], "r");
	
	if (input == NULL)
	{
		return -1;
	}

	FILE* output = fopen(argv[2], "w");

	if (output == NULL)
	{
		fclose(input);
		return -1;
	}

	vector v;
	int n;

	while ((fscanf(input, "%d", &n)) == 1){
		v.push_back(n);
	}

	v.quicksort();

	for (size_t i = 0; i < v.size(); i++)
	{
		fprintf(output, "%d,", v[i]);
	}

	fclose(input);
	fclose(output);



	return 0;

}
*/
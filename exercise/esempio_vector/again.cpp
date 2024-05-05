#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <random>
#include <iostream>
#include <string>

int sorting(const void* a, const void* b) {
	
	int* ai = (int*)a;
	int *bi = (int*)b;

	if (*ai > *bi)
	{
		return 1;
	}
	if (*ai < *bi)
	{
		return -1;
	}
	return 0;

}

struct vector {

	size_t capacity_;
	size_t size_;
	int* data;

	vector() {
	
		capacity_ = 10;
		size_ = 0;
		/*
		malloc ritorna un puntatore a void, che in cpp non va bene.
		Dobbiamo castare il risultato al tipo che ci interessa, ovvero puntatore a int
		*/
		data = (int*)malloc(capacity_ * sizeof(int));

	}

	/*
	il distruttore rilascia le risorse
	*/
	~vector() {
		
		free(data);
	
	}

	void push_back(int val) {
	
		if (size_ == capacity_)	{
			capacity_ *= 2;
			data = (int*)realloc(data, capacity_ * sizeof(int));
		}
		data[size_] = val;
		size_ += 1;
		
	}


	//in questo modo posso usare v.data[index]
	int operator[](int i) const{
	
		return data[i];
	
	}

	void quicksort() {
		qsort(data, size_, sizeof(int), sorting);
	}

	size_t get_size() {
		return size_;
	}

	int set_capacity(int c) {
		capacity_ = c;
	}




};


int main(int argc, char argv[]) {


	//così viene chiamato automaticamente il costruttore
	vector v;
	//printf("%zu", v.capacity_);


	FILE* input = fopen("in.txt", "r");
	int num;

	//fopen ritorna un puntatore a FILE
	FILE* output = fopen("out.txt", "w");

	while (fscanf(input, "%d", &num) == 1)
	{
		v.push_back(num);
	}

	v.quicksort();	

	
	for (size_t i = 0; i < v.get_size(); i++)
	{
		fprintf(output, "%d\n", v.data[i]);
	}

	std::vector<std::vector<int>> matrix;

	

	for (int i = 0; i < v.get_size(); i+=3)
	{
		std::vector<int> tmpvec = { v.data[i], v.data[i + 1], v.data[i + 2] };
		matrix.push_back(tmpvec);
	}

	
	/*
	for (size_t i = 0; i < matrix.size(); i++)
	{
		//così sto aggiungendo una colonna
		matrix[i].push_back(4);
	}
	*/

	//vecchio modo di fare 
	for (size_t i = 0; i < matrix.size(); i++)
	{
		for (size_t ii = 0; ii < matrix[i].size(); ii++)
		{
			std::cout << matrix[i][ii] << " ";
		}
		std::cout << std::endl;
	}


	std::cout << std::endl;

	for (const auto& row : matrix) {
		for (int value : row) {
			std::cout << value << " ";
		}
		std::cout << std::endl;
	}



	fclose(output);


}
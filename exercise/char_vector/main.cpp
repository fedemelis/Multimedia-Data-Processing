#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int compare(const void* a, const void* b) {

	char aa = *(char*)a;
	char bb = *(char*)b;

	if (aa > bb)
	{
		return 1;
	}
	if (aa < bb)
	{
		return -1;
	}
	return 0;

}


struct vector_char {

	size_t dim_;
	size_t capacity_;
	char* data_;

	vector_char() {
		dim_ = 0;
		capacity_ = 10;
		data_ = (char*)malloc(capacity_ * sizeof(char));
	}

	~vector_char() {
		free(data_);
	}

	void push_back(const char c) {
		if (dim_ == capacity_)
		{
			capacity_ *= 2;
			data_ = (char *)realloc(data_, capacity_ * sizeof(char));
		}
		data_[dim_] = c;
		dim_++;
	}

	void push_back(const char *s) {
		for (size_t i = 0; i < strlen(s); i++)
		{
			if (dim_ == capacity_)
			{
				capacity_ *= 2;
				data_ = (char*)realloc(data_, capacity_ * sizeof(char));
			}
			data_[dim_] = s[i];
			dim_++;
		}
	}

	const size_t get_dim() const{
		return dim_;
	}

	const char& operator[](const size_t index) const{
		return data_[index];
	}

	char& operator[](const size_t index) {
		return data_[index];
	}

	void sort() {
		qsort(data_, dim_, sizeof(char), compare);
	}
		
};


int main(void) {

	FILE* input = fopen("input.txt", "rb");
	
	vector_char v;
	char c;
	char *str = (char *)malloc(100 * sizeof(char));


	while ((c = fgetc(input)) != EOF)
	{
		v.push_back(c);
	}

	fclose(input);

	//v.sort();

	for (size_t i = 0; i < v.get_dim(); i++)
	{
		if ((v[i] != ' ') && (v[i] != '\n'))
		{
			//v[i] -= 32;
		}
	}

	for (size_t i = 0; i < v.get_dim(); i++)
	{
		printf("%c", v[i]);
	}

	free(str);

	/*
	int n = 0;

	char** stringhe = (char**)malloc(100 * sizeof(char*));


	while (fscanf(input, "%s", str) == 1) {
		v.push_back(str);
		stringhe[n] = (char*)malloc((strlen(str)+1) * sizeof(char));
		for (size_t i = 0; i < strlen(str)+1; i++)
		{
			stringhe[n][i] = str[i];
		}
		n++;
	}

	for (size_t i = 0; i < n; i++)
	{
		printf("%s\n", stringhe[i]);
		free(stringhe[i]);
	}

	fclose(input);

	free(str);
	free(stringhe);
	*/
	
	return 0;
}
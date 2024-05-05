#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "compare.h"
#include <utility>

int compare(const void* a, const void* b) {

	int aa = *(int*)a;
	int bb = *(int*)b;

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




struct vector{
private:
	size_t capacity_;
	size_t size_;
	int* data;
public:

	/*
	move in pratica casta a rvalue reference
	auto ti dice di desumere il tipo della lhs var dalla rhs var
	*/
	friend void swap(vector& x, vector& y) {
		using std::swap; //abilita ADL argument dependant lookup
		swap(x.capacity_, y.capacity_);
		swap(x.size_, y.size_);
		swap(x.data, y.data);
	}
	//default constructor
	vector() {
	
		capacity_ = 10; //initial capacity
		size_ = 0;
		data = (int*)malloc(capacity_ * sizeof(int));
	
	}

	
	/*
	stiamo facendo overload del default copy constructor
	questo lo chiamiamo come: 
	vector v(old_vec);
	vector v = oldvec;

	NB: fa una deep copy
	*/
	/*
	* 
	* && rvalue refernece
	* & lvalue reference
	* 
	*/
	//copy constructor lvalue
	vector(const vector& other) {
		capacity_ = other.capacity_;
		size_ = other.size_;
		data = (int*)malloc(capacity_ * sizeof(int));
		for (size_t i = 0; i < size_; i++)
		{
			data[i] = other.data[i];
		}
	}
	//move constructor rvalue
	/*
	chiamato quando inizializzo un oggetto restituendo un oggetto dello stesso tipo da una funzione
	*/
	vector(vector&& other) noexcept { //move constructor
		capacity_ = other.capacity_;
		size_ = other.size_;
		data = other.data;
		other.capacity_ = 0;
		other.size_ = 0;
		other.data = nullptr;
		//elimino il riferimento alla memoria nel vecchio oggetto
	}

	/*
	sto sovrascrivendo il comportamento dell'operatore "="
	Il metodo restituisce un riferimento a vector
	*/
	//copy assignment operator
	//copy and swap Idiom
	vector& operator=(vector rhs) noexcept {
		/*
		se il lhs e il rhs PUNTANO alla stessa risorsa in memoria
		non devo liberare la memoria occupata dal lhs. Se lo faccio
		perdo tutto
		*/
		swap(*this, rhs);
		/*
		restituisco il valore puntato da this perche avrò qualcosa come v1 = v2
		l'operatore "=" resituisce il riferimento al lhs
		*/
		return *this;
	}

	//il distruttore libera rilascia le risorse che un oggetto alloca
	~vector() {
		free(data);
	}

	vector read_vector(const char* filename)
	{
		vector q;
		FILE* f = fopen(filename, "r");
		if (f == NULL) {
			return q;
		}
		int num;
		while (fscanf(f, "%d", &num) == 1) {
			q.push_back(num);
		}
		fclose(f);
		return q;
	}

	void push_back(int n) {
	
		if (capacity_ <= size_){
			capacity_ *= 2;
			data = (int*)realloc(data, capacity_ * sizeof(int));
		}
		data[size_] = n;
		size_++;

	}

	void sort() {
		qsort(data, size_, sizeof(int), compare);
	}

	size_t size() const {
		return size_;
	}

	/*
	la differenza fra questi due metodi è const.
	Quello con const viene chiamato quando lavoriamo con un const vector&
	*/
	const int& operator[](size_t index) const{
		return data[index];
	}
	//setter
	int& operator[](size_t index) {
		return data[index];	
	}

};

vector read_vector(const char* filename) {
	//named return value optimization
	vector v;

	FILE* input_file = fopen(filename, "r");
	if (input_file != NULL) {
		int num;
		while (fscanf(input_file, "%d", &num) == 1) {
			v.push_back(num);
		}
		fclose(input_file);
	}
	return v;
}

/*
prendo un riferimento a const vector perché lavoro sui rifeirmenti
*/
void scrivi_vettore(FILE* f, const vector& v) {
	for (int i = 0; i < v.size(); i++) {
		fprintf(f, "%d\n", v[i]);
	}
}





int main(int argc, char* argv[]) { 

	/*
	if (argc != 3)
	{
		printf("I parametri devono essere 2");
		return -1;
	}

	FILE* input = fopen(argv[1], "r");
	FILE* output = fopen(argv[2], "w");

	int num;
	vector v;
	while (fscanf(input, "%d", &num) == 1){
		v.push_back(num);
	}

	v.sort();

	for (size_t i = 0; i < v.get_size(); i++)
	{
		fprintf(output, "%d\n", v[i]);
	}

	fclose(input);
	fclose(output);
	*/

	int x = 5;


	if (argc != 3) {
		return -1;
	}

	FILE* input_file = fopen(argv[1], "r");
	if (input_file == NULL) {
		return 1;
	}

	FILE* output_file = fopen(argv[2], "w");
	if (output_file == NULL) {
		fclose(input_file);
		return 1;
	}

	vector v;

	int num;
	while (fscanf(input_file, "%d", &num) == 1) {
		v.push_back(num);
	}
	fclose(input_file);

	// Sort the numbers
	v.sort();

	v[0] = 0;

	// Write sorted numbers to the output file
	for (int i = 0; i < v.size(); i++) {
		fprintf(output_file, "%d\n", v[i]);
	}
	fclose(output_file);

	return 0;
}

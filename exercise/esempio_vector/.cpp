#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

int compare_integers(const void* a, const void* b)
{
    int ia = *(int*)a;
    int ib = *(int*)b;
    if (ia < ib) {
        return -1;
    }
    else if (ia > ib) {
        return 1;
    }
    else {
        return 0;
    }
}

struct vector {
    size_t capacity_;
    size_t size_;
    int* data_;

    vector() {
        capacity_ = 10; // Initial capacity
        size_ = 0;
        data_ = (int*)malloc(capacity_ * sizeof(int));
    }
    ~vector() {
        free(data_);
    }
    void push_back(int num) {
        // Resize the array if necessary
        if (size_ == capacity_) {
            capacity_ *= 2;
            data_ = (int*)realloc(data_, capacity_ * sizeof(int));
        }
        data_[size_] = num;
        size_++;
    }
    void sort() {
        qsort(data_, size_, sizeof(int), compare_integers);
    }
    int at(int i) const {
        assert(i >= 0 && i < size_);
        return data_[i];
    }

    int operator[](int i) const {
        return data_[i];
    }

    size_t size() const {
        return size_;
    }
};


int main(int argc, char* argv[])
{
    if (argc != 3) {
        return 1;
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
    // Write sorted numbers to the output file
    for (int i = 0; i < v.size(); i++) {
        fprintf(output_file, "%d\n", v[i]);
    }
    fclose(output_file);

    return 0;
}
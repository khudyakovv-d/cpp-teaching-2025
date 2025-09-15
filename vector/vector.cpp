#include "vector.h"
#include <iostream>

Vector::Vector() {
    this->arr_ = nullptr;
    this->size_ = 0;
    std::cout << "Create vector " << this << "\n";
}

Vector::Vector(int size) {
    this->arr_ = new int[size]{};
    this->size_ = size;
    std::cout << "Create vector " << this << " with size " << size << "\n";
}

Vector::~Vector() {
    delete[] this->arr_;
    std::cout << "Delete vector " << this << "\n";
}

int Vector::size() const {
    return this->size_;
}

Vector::Vector(const Vector &vec) : arr_(new int[vec.size()]), size_(vec.size()) {
    for (int i = 0; i < vec.size(); i++) {
        this->arr_[i] = vec.arr_[i];
    }
    std::cout << "Create " << this << " from " << &vec << "\n";
}

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

Vector &Vector::operator=(const Vector &vec) {
    if (this == &vec) {
        return *this; // защита от самоприсваивания
    }

    delete[] this->arr_;

    this->size_ = vec.size();
    this->arr_ = new int[vec.size()];

    for (int i = 0; i < vec.size(); i++) {
        this->arr_[i] = vec.arr_[i];
    }

    std::cout << "Copy " << &vec << " to " << this << "\n";
    return *this;
}

int &Vector::operator[](int i) {
    return this->arr_[i];
}

int const &Vector::operator[](int i) const {
    return this->arr_[i];
}

Vector const Vector::foo() {
    return Vector(10);
}

// Vector operator+(const Vector &vec1, const Vector &vec2) {
//     if (vec1.size() != vec2.size()) {
//         throw std::length_error("Vectors must be same size");
//     }

//     Vector v = Vector(vec1.size());
//     for (int i = 0; i < vec1.size(); i++) {
//         v[i] = vec1[i] + vec2[i];
//     }
//     std::cout << "Free operator" << "\n";
//     return v;
// }

Vector Vector::operator+(const Vector &other) const {
    if (this->size_ != other.size_) {
        throw std::length_error("Vectors must be same size");
    }
    Vector v(this->size_);
    for (int i = 0; i < this->size_; i++) {
        v[i] = this->arr_[i] + other[i];
    }
    std::cout << "Class method" << this << "\n";
    return v;
}

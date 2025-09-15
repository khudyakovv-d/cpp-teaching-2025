#include <iostream>
#include "vector.h"

int foo1(Vector v) {
    return v.size();
}

int foo2(Vector &v) {
    return v.size();
}

int main(int, char**){
    std::cout << "Hello, from vector!\n";
    
    // {
    //     Vector * v1 = new Vector(10);
    //     {
    //         Vector v2 = Vector(10);
    //     }
    //     delete v1;
    // }

    // std::cout << "\n\n\n\n";

    // {
    //     Vector v1 = Vector(20);
    //     Vector v2 = v1;
    //     bool equals = v2.size() == v1.size();
    //     std::cout << equals << "\n";
    // }

    {
        Vector v1 = Vector(20);
        int size = foo2(v1);
        std::cout << size << "\n";
    }
}

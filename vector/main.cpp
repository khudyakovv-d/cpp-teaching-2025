#include <iostream>
#include "vector.h"

int foo1(Vector v) {
    return v.size();
}

int foo2(Vector &v) {
    return v.size();
}

Vector foo3(Vector v) {
    return v;
}

int main(int, char**){
    std::cout << "Hello, from vector!\n";
    
    {
        std::cout << "Example1\n";
        Vector * v1 = new Vector(10);
        {
            Vector v2 = Vector(10);
        }
        delete v1;
    }

    std::cout << "\n\n\n\n";

    {
        std::cout << "Example2\n";
        Vector v1 = Vector(20);
        Vector v2 = v1;
        bool equals = v2.size() == v1.size();
        std::cout << equals << "\n";
    }

    {
        std::cout << "Example3\n";
        Vector v1 = Vector(20);
        int size = foo1(v1);
        std::cout << size << "\n";
    }

    {
        std::cout << "Example4\n";
        Vector v1 = Vector(20);
        auto v = foo3(v1);
        std::cout << "foo3" << "\n";
    }

    {
        std::cout << "Example5\n";
        Vector v1 = Vector(10);
        Vector v2 = Vector(20);
        v2 = v1;
        std::cout << v2.size() << "\n";
    }

    {
        std::cout << "Example6\n";
        Vector * v1 = new Vector(10);
        Vector * v2 = new Vector(20);
        v2 = v1;
        std::cout << v2->size() << "\n";
    }

    {
        std::cout << "Example7\n";
        Vector v1 = Vector(10);
        v1[5] = 10;
        for (size_t i = 0; i < v1.size(); i++) {
            std::cout << v1[i] << " ";
        }    
        std::cout << "\n";
    }

    {
        std::cout << "Example8\n";
        const Vector v1 = Vector(10);
        // v1[5] = 10;
    }

    {
        std::cout << "Example8\n";
        const Vector v1 = Vector(10);
        for (size_t i = 0; i < v1.size(); i++) {
            std::cout << v1[i] << " ";
        }    
        std::cout << "\n";
    }

}

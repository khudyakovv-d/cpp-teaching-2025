class Vector {
private:
    int * arr_;
    int size_;

public:
    Vector();
    explicit Vector(int size);
    ~Vector();
    int size() const;
    Vector(const Vector &vec);
    Vector &operator=(const Vector &vec);
    int &operator[](int i);
    int const &operator[](int i) const;
    Vector const foo();
    Vector operator+(const Vector &other) const;
};

//Vector operator+(const Vector &vec1, const Vector &vec2);
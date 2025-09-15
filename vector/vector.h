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
};
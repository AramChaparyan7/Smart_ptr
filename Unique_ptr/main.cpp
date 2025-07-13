#include <iostream>
#include <utility>
#include <gtest/gtest.h>

template<typename T>
class Unique_ptr{
private:
    T* ptr;
public:
    explicit Unique_ptr(T* p = nullptr) : ptr{p} {}

    Unique_ptr(const Unique_ptr&) = delete;

    Unique_ptr& operator=(const Unique_ptr&) = delete; 

    Unique_ptr(Unique_ptr&& other) noexcept{
        ptr = other.ptr;
        other.ptr = nullptr;
    }

    Unique_ptr& operator=(Unique_ptr&& other) noexcept{
        if(this == &other){
            return *this;
        }
        delete ptr;
        ptr = other.ptr;
        other.ptr = nullptr;
        return *this;
    }  

    ~Unique_ptr() {
        delete ptr;
    }
    
    T& operator*(){
        return *ptr;
    }

    T* operator->(){
        return ptr;
    }

    T* get() const{
        return ptr;
    }

    T* release(){
        T* tmp = ptr;
        ptr = nullptr;
        return tmp;
    }

    void reset(T* p = nullptr){
        delete ptr;
        ptr = p;
    }

    operator bool() noexcept{
        return ptr?true:false;
    }

    void swap(Unique_ptr& other) noexcept{
        std::swap(ptr, other.ptr);
    }
};

int main(int argc, char** argv){
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

TEST(UniquePtrTest, Dereference) {
    Unique_ptr<int> ptr(new int(10));
    EXPECT_EQ(*ptr, 10);
    EXPECT_EQ(*ptr, 10);
}

TEST(UniquePtrTest, MoveConstructor) {
    Unique_ptr<int> ptr1(new int(20));
    Unique_ptr<int> ptr2(std::move(ptr1));
    EXPECT_EQ(*ptr2, 20);
    EXPECT_EQ(ptr1.get(), nullptr); 
}

TEST(UniquePtrTest, MoveAssignment) {
    Unique_ptr<char> ptr1(new char('a'));
    Unique_ptr<char> ptr2;
    ptr2 = std::move(ptr1);
    EXPECT_EQ(*ptr2, 'a');
    EXPECT_EQ(ptr1.get(), nullptr);
}

TEST(UniquePtrTest, ResetReleasesOldObject) {
    Unique_ptr<double> ptr(new double(40.5));
    ptr.reset(new double(50.9));
    EXPECT_EQ(*ptr, 50.9);
}
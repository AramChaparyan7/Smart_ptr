#include <utility>
#include <gtest/gtest.h>
#include <cstddef>
#include <memory>

template<typename T>
class Control_block{
private:
    size_t count;
    T* ptr;
public:
    Control_block(T* p) : count{1}, ptr{p} {}
    void incr() { ++count; }
    void decr() { --count; }
    size_t getcount() {
        return count;
    }
};

template<typename T>
class Shared_ptr{
private:
    T* ptr;
    Control_block<T> *controlptr;
public:
    explicit Shared_ptr(T* p = nullptr) : ptr{p} {
        if(p != nullptr) {
            controlptr = new Control_block<T>(p);
        } else{
            controlptr = nullptr;
        }
        
    }

    void Deleter() {
        if (!controlptr) {
            return;
        }
        controlptr->decr();
        if(this->getcount() == 0) {
            std::cout << "delete" << std::endl;
            delete ptr;
            delete controlptr;
        }
        ptr = nullptr;
        controlptr = nullptr;
    }

    ~Shared_ptr() {
        this->Deleter();
    }

    Shared_ptr(const Shared_ptr& other) {
        ptr = other.ptr;
        controlptr = other.controlptr;
        controlptr->incr();
    } 

    template<typename U>
    Shared_ptr(std::unique_ptr<U> && other) {
        ptr = other.release();
        controlptr = new Control_block<T>(ptr);
    }

    Shared_ptr& operator=(const Shared_ptr& other) {
        if(this == &other) {
            return *this;
        }
        this->Deleter();
        ptr = other.ptr;
        controlptr =  other.controlptr;
        controlptr->incr();
        return *this;
    } 

    Shared_ptr(Shared_ptr&& other) noexcept{
        ptr = other.ptr;
        controlptr = other.controlptr;
        controlptr->incr();
        other.Deleter();
    }

    Shared_ptr& operator=(Shared_ptr&& other) noexcept{
        if(this == &other){
            return *this;
        }
        this->Deleter();
        ptr = other.ptr;
        controlptr =  other.controlptr;
        other.controlptr->incr();
        other.Deleter();
        return *this;
    }  
    
    T& operator*(){
        return *ptr;
    }

    T* operator->(){
        return ptr;
    }

    T* get(){
        return ptr;
    }

    size_t getcount() const {
        if(controlptr) {
            return controlptr->getcount();
        }
        return 0;
    }

    void reset(T* p = nullptr){
        this->Deleter();
        if(p) {
            ptr = p;
            controlptr = new Control_block<T>(p);
        }
    }

    operator bool() const {
        return ptr?true:false;
    }

    void swap(Shared_ptr& other) noexcept {
        std::swap(ptr, other->ptr);
        std::swap(controlptr, other->controlptr);
    }

    bool operator==(const Shared_ptr& other) const {
        return ptr == other.ptr;
    }
};

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

TEST(SharedPtrTest, Dereference) {
    Shared_ptr<int> ptr(new int(10));
    EXPECT_EQ(*ptr, 10);
    EXPECT_EQ(ptr.getcount(), 1);
}

TEST(SharedPtrTest, MoveConstructor) {
    Shared_ptr<int> ptr1(new int(20));
    Shared_ptr<int> ptr2(std::move(ptr1));
    EXPECT_EQ(*ptr2, 20);
    EXPECT_EQ(ptr2.getcount(), 1); 
}

TEST(SharedPtrTest, MoveAssignment) {
    Shared_ptr<char> ptr1(new char('a'));
    Shared_ptr<char> ptr2;
    ptr2 = std::move(ptr1);
    EXPECT_EQ(*ptr2, 'a');
    EXPECT_EQ(ptr1, false);
}

TEST(SharedPtrTest, CopyConstructor) {
    Shared_ptr<int> ptr1(new int(20));
    Shared_ptr<int> ptr2(ptr1);
    EXPECT_EQ(*ptr2, 20);
    EXPECT_EQ(ptr2.getcount(), 2); 
}

TEST(SharedPtrTest, CopyAssignment) {
    Shared_ptr<char> ptr1(new char('a'));
    Shared_ptr<char> ptr2;
    ptr2 = (ptr1);
    EXPECT_EQ(*ptr2, 'a');
    EXPECT_EQ(ptr1.getcount(), 2);
}

TEST(SharedPtrTest, ResetReleaseObject) {
    Shared_ptr<double> ptr(new double(40.5));
    ptr.reset(new double(50.9));
    EXPECT_EQ(*ptr, 50.9);
}

#include <utility>
#include <gtest/gtest.h>
#include <cstddef>
#include <memory>

template<typename T>
class Control_block{
private:
    std::atomic<size_t> ref_count;
    std::atomic<size_t> weak_count;
    T* ptr;
public:
    Control_block(T* p) : ref_count{1}, ptr{p} {}
    void incr_ref_count() { ++ref_count; }
    void decr_ref_count() { --ref_count; }
    void incr_weak_count() { ++weak_count; }
    void decr_weak_count() { --weak_count; }

    size_t get_ref_count() const {
        return ref_count;
    }

    size_t get_weak_count() const {
        return weak_count;
    }

    T* get() const {
        return ptr;
    }
};

template<typename T>
class Weak_ptr;

template<typename T>
class Shared_ptr{
private:
    T* ptr;
    Control_block<T> *controlptr;

    void Deleter() {
        if (!controlptr) {
            return;
        }
        controlptr->decr_ref_count();
        if(this->get_ref_count() == 0) {
            delete ptr;
            if(this->get_weak_count() == 0) {
                delete controlptr;
            }
        }
        ptr = nullptr;
        controlptr = nullptr;
    }

public:
    explicit Shared_ptr(T* p = nullptr) : ptr{p} {
        if(p != nullptr) {
            controlptr = new Control_block<T>(p);
        } else{
            controlptr = nullptr;
        }       
    }

    ~Shared_ptr() {
        this->Deleter();
    }

    Shared_ptr(const Shared_ptr& other) {
        ptr = other.ptr;
        controlptr = other.controlptr;
        if(controlptr != nullptr){
            controlptr->incr_ref_count();
        }
    } 

    template<typename U>
    Shared_ptr(std::unique_ptr<U> && other) {
        ptr = other.release();
        controlptr = new Control_block<T>(ptr);
    }

    explicit Shared_ptr(const Weak_ptr<T>& weak) {
        if(weak.expired()) {
            ptr = nullptr;
            controlptr = nullptr;
        } else{
            ptr = weak.ptr;
            controlptr = weak.controlptr;
            controlptr->incr_ref_count();
        }
    }

    Shared_ptr& operator=(const Shared_ptr& other) {
        if(this == &other) {
            return *this;
        }
        this->Deleter();
        ptr = other.ptr;
        controlptr =  other.controlptr;
        if(controlptr != nullptr){
            controlptr->incr_ref_count();
        }
        return *this;
    } 

    Shared_ptr(Shared_ptr&& other) noexcept{
        ptr = other.ptr;
        controlptr = other.controlptr;
        other.ptr = nullptr;
        other.controlptr = nullptr;
    }

    Shared_ptr& operator=(Shared_ptr&& other) noexcept{
        if(this == &other){
            return *this;
        }
        this->Deleter();
        ptr = other.ptr;
        controlptr =  other.controlptr;
        other.ptr = nullptr;
        other.controlptr = nullptr;
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

    size_t get_ref_count() const {
        if(controlptr) {
            return controlptr->get_ref_count();
        }
        return 0;
    }

    size_t get_weak_count() const {
        if(controlptr) {
            return controlptr->get_weak_count();
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
        std::swap(ptr, other.ptr);
        std::swap(controlptr, other.controlptr);
    }

    bool operator==(const Shared_ptr& other) const {
        return ptr == other.ptr;
    }
};

template <typename T>
class Weak_ptr{
private:
    Control_block<T>* controlptr;   
    void Deleter() {
        if (!controlptr) {
            return;
        }
        controlptr->decr_weak_count();
        if(this->use_count == 0 && this->controlptr->get_weak_count() == 0) {
            delete controlptr;
        }
        controlptr = nullptr;
    }
public:
    Weak_ptr(const Shared_ptr<T>& other) {
        controlptr = other.controlptr;
        controlptr->incr_weak_count();
    }

    ~Weak_ptr() {
        this->Deleter();
    }

    Weak_ptr(const Weak_ptr& other) {
        controlptr = other.controlptr;
        if(controlptr != nullptr){
            controlptr->incr_weak_count();
        }
    } 

    Weak_ptr& operator=(const Weak_ptr& other) {
        if(this == &other) {
            return *this;
        }
        this->Deleter();
        controlptr = other.controlptr;
        if(controlptr != nullptr){
            controlptr->incr_weak_count();
        }
        return *this;
    } 

    Weak_ptr(Weak_ptr&& other) noexcept{
        controlptr = other.controlptr;
        if(controlptr != nullptr){
            controlptr->incr_weak_count();
        }
        other.Deleter();
    }

    Weak_ptr& operator=(Weak_ptr&& other) noexcept{
        if(this == &other){
            return *this;
        }
        this->Deleter();
        controlptr =  other.controlptr;
        if(controlptr != nullptr){
            controlptr->incr_weak_count();
        }
        other.Deleter();
        return *this;
    }  

    void use_count() const{
        return controlptr->get_ref_count();
    }

    void swap(Weak_ptr& other) noexcept {
        std::swap(controlptr, other.controlptr);
    }

    void reset() {
        this->Deleter();
        controlptr = nullptr;
    }

    bool expired() const {
        return !controlptr || use_count();
    }

    Shared_ptr<T> lock() const noexcept {
        if(this->expired()) {
            return Shared_ptr<T>();
        }
        return Shared_ptr<T>(*this);
    }
};

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

TEST(SharedPtrTest, Dereference) {
    Shared_ptr<int> ptr(new int(10));
    EXPECT_EQ(*ptr, 10);
    EXPECT_EQ(ptr.get_ref_count(), 1);
}

TEST(SharedPtrTest, MoveConstructor) {
    Shared_ptr<int> ptr1(new int(20));
    Shared_ptr<int> ptr2(std::move(ptr1));
    EXPECT_EQ(*ptr2, 20);
    EXPECT_EQ(ptr2.get_ref_count(), 1); 
}

TEST(SharedPtrTest, MoveAssignment) {
    Shared_ptr<char> ptr1(new char('a'));
    Shared_ptr<char> ptr2;
    ptr2 = std::move(ptr1);
    EXPECT_EQ(*ptr2, 'a');
    EXPECT_EQ(ptr1, false);
}

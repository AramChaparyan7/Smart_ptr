#include <utility>

template<typename T>
class Control_block{
private:
    T* ptr;
    size_t count;
public:
    Control_block(T* p) : count{0}, ptr{p} {}
    incr() { ++count; }
};

template<typename T>
class Shared_ptr{
private:
    T* ptr;
    Control_block<T> *controlptr;
public:
    explicit Shared_ptr(T* p = nullptr) : ptr{p} {
        controlptr = new Control_block<T>(p);
    }

    void Deleter() {
        --controlptr.count;
        if(controlptr.count == 0) {
            delete ptr;
            delete controlptr;
        }
    }

    ~Shared_ptr() {

    }

    Shared_ptr(const Shared_ptr& other) {
        ptr = other.ptr;
        controlptr = other.controlptr;
        controlptr->incr();
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
        other.ptr = nullptr;
        controlptr = other.controlptr;
    }

    Shared_ptr& operator=(Shared_ptr&& other) noexcept{
        if(this == &other){
            return *this;
        }
        this->Deleter();
        ptr = other.ptr;
        controlptr =  other.controlptr;
        other.ptr = nullptr;
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

    // void reset(T* p = nullptr){
    //     delete ptr;
    //     ptr = p;
    // }

    explicit operator bool(){
        return ptr?true:false;
    }
};


#include <cstddef>
#include <memory>
#include <new>
#include <sstream>
#include <stdexcept>
#include <type_traits>
namespace algorithm {

template <typename T>
class vector {
  public:
    vector();
    vector(const vector<T>& other);
    vector(vector<T>&& other);
    vector& operator=(const vector& other);
    vector& operator=(vector&& other);
    T& operator[](std::size_t index);
    const T& operator[](std::size_t index) const;
    T& at(std::size_t index);
    const T& at(std::size_t index) const;
    void push_back(const T& value);
    std::size_t size();
    std::size_t size() const;
    std::size_t capacity();
    std::size_t capacity() const;

    ~vector();
    
  private:
    T* data_;
    std::size_t size_;
    std::size_t capacity_;

    void destruct_elems();
};

template <typename T>
vector<T>::vector():data_(nullptr), size_(0), capacity_(0) {
}

template <typename T>
vector<T>::vector(const vector& other) {
  size_ = other.size_;
  capacity_ = other.capacity_;
  data_ = ::operator new(sizeof(T)*other.capacity_);
  for (std::size_t i = 0; i < size_; i++) {
    new (data_ + i) T(other[i]);
  }
}

template <typename T>
vector<T>::vector(vector&& other) {
  data_ = other.data_;
  size_ = other.size_;
  capacity_ = other.capacity_;
  other.data_ = nullptr;
  other.size_ = 0;
  other.capacity_ = 0;
}

template <typename T>
vector<T>& vector<T>::operator=(const vector& other) {
  if (this != std::addressof(other)) {
    destruct_elems();
    size_ = other.size_;
    if (capacity_ < other.capacity_) {
      ::operator delete(data_);
      capacity_ = other.capacity_;
      data_ = ::operator new(sizeof(T)*capacity_);
    }
    
    for(size_t i = 0; i < size_; i++) {
      new (data_ + i) T(other[i]);
    }
  }
  return *this;
}

template <typename T>
vector<T>& vector<T>::operator=(vector&& other) {
  if (this != std::addressof(other)) {
    this->~vector();
    data_ = other.data_;
    size_ = other.size_;
    capacity_ = other.capacity_;
    
    other.data_ = nullptr;
    other.size_ = 0;
    other.capacity_ = 0;
  }
  return *this;
}

template <typename T>
T& vector<T>::operator[](std::size_t index) {
  if (index >= size_) {
    std::ostringstream oss;
    oss << "curr size is " << size_
        << " but got index is " << index
        << "\n";
    throw std::out_of_range(oss.str());
  }
  return data_[index];
}

template <typename T>
const T& vector<T>::operator[](std::size_t index) const {
  if (index >= size_) {
    std::ostringstream oss;
    oss << "curr size is " << size_
        << " but got index is " << index
        << "\n";
    throw std::out_of_range(oss.str());
  }
  return data_[index];
}

template <typename T>
T& vector<T>::at(std::size_t index) {
  if (index >= size_) {
    std::ostringstream oss;
    oss << "curr size is " << size_
        << " but got index is " << index
        << "\n";
    throw std::out_of_range(oss.str());
  }
  return data_[index];
}

template <typename T>
const T& vector<T>::at(std::size_t index) const {
  if (index >= size_) {
    std::ostringstream oss;
    oss << "curr size is " << size_
        << " but got index is " << index
        << "\n";
    throw std::out_of_range(oss.str());
  }
  return data_[index];
}

// 不能将将原有的元素移动到新开辟的内存，如果移动的过程失败，者破坏原有的状态
// 需要区分T类型 不可移动和不可复制的
template <typename T>
void vector<T>::push_back(const T& value) {
  if (capacity_ < size_ + 1) {
    capacity_ = capacity_ ==0 ? 2 : capacity_ * 2;
    T* new_data = ::operator new(sizeof(T)*capacity_);
    for (std::size_t i = 0; i < size_; i++) {
      if constexpr(std::is_nothrow_move_constructible_v<T>) {
        new (new_data + i) T(std::move(data_[i]));
      } else {
        new (new_data + i) T(data_[i]);
      }
    }
    if constexpr (!std::is_nothrow_move_constructible_v<T>) {
      destruct_elems();
      ::operator delete(data_);
      data_ = new_data;
    }
  }
  new (data_ + size_) T(value);
  size_ += 1;
}

template <typename T>
void vector<T>::destruct_elems() {
  for (size_t i = 0; i < size_; i++) {
    data_[i].~T();
  }
}

template<typename T>
vector<T>::~vector() {
  destruct_elems();
  if (data_) {
    ::operator delete(data_);
    data_ = nullptr;
  }
}

template<typename T>
std::size_t vector<T>::size() {
  return size_;
}

template<typename T>
std::size_t vector<T>::size() const {
  return size_;
}

template<typename T>
std::size_t vector<T>::capacity() {
  return capacity;
}

template<typename T>
std::size_t vector<T>::capacity() const {
  return capacity;
}


} // end namespace algorithm
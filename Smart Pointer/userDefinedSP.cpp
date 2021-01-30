// 

#include <iostream>

template <class T>
class smart_ptr {
private:
 T *ptr;
public:
	smart_ptr (T *p = nullptr) {
		ptr = p;
	}

	T * get() {
		return ptr;
	}

	T& operator *() {
		return *ptr;
	}

	~smart_ptr() {
		std::cout<<"~smart_ptr"<<std::endl;
		delete ptr;
	}

};

int main () {
	std::cout<<"Smart Pointer program started"<<std::endl;
	smart_ptr<int> obj(new int());
	std::cout<<obj.get()<<std::endl;
	std::cout<<*obj<<std::endl;
	*obj = 20;
	std::cout<<*obj<<std::endl;
	return 0;
}
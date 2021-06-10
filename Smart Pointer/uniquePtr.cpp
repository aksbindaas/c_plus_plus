#include <iostream>
#include <memory>
#include <cassert>
class BaseClass {
public:
	virtual void Publish() {
		std::cout<<"BaseClass::Publish\n";
	}
	BaseClass() { std::cout<<"BaseClass\n"; }
	virtual ~BaseClass()  { std::cout<<"~BaseClass\n"; }
};

int main() {
	std::cout<<"============== Classical pointer Demo ============="<<std::endl; {
		BaseClass *obj = new BaseClass;
		delete obj;
	}

	std::cout<<"============== Unique Ptr creation demo ==============\n"; {
		std::unique_ptr<BaseClass> obj = std::make_unique<BaseClass>();
		std::unique_ptr<BaseClass> obj1 (new BaseClass);
	}

	std::cout<<"============== Unique Ptr ownership demo ==============\n"; {
		std::unique_ptr<BaseClass> obj = std::make_unique<BaseClass>();
		std::unique_ptr<BaseClass> obj2 = std::move(obj);
		assert(obj == nullptr);
		obj2->Publish();
	}
	return 0;
}
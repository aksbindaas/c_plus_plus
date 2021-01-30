#include <iostream>
#include <string>
#include <memory>
class BigObj {
private:
	std::string name;
public:
	BigObj(std::string name) {
		this->name = name;
	}

	void PrintName() {
		std::cout<<name<<std::endl;
	}

	~BigObj() {
		std::cout<<"~BigObj"<<std::endl;
	}
};

int main() {
	std::cout<<"Unique Ptr program started"<<std::endl;
	{
		BigObj *obj = new BigObj("Big Object Normal pointer");
		obj->PrintName();
		delete obj;
	}
	{
		// creating object using make_unique
		std::unique_ptr<BigObj> obj = std::make_unique<BigObj>("Big Object Unique ptr");
		obj->PrintName();
		obj.get()->PrintName();
	}
	{
		// creating object using object creation
		std::unique_ptr<BigObj> obj(new BigObj("Big Object Unique ptr"));
		obj->PrintName();
		obj.get()->PrintName();
	}
	return 0;
}
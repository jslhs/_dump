#include <iostream>
#include <algorithm>
#include <vector>
#include <future>
#include <functional>
#include <thread>
#include "IRelay.h"
//#include <initializer_list>

class A
{
public:
	A(): _d(new int [32]), _c(32) {}
//	A(std::initializer_list<int>){}
	~A(){};

	void func()
	{ 
		std::cout << "hello, world!" << std::endl;
		int ch;
		std::cout << "press enter to continue!" << std::endl;
		std::cin >> ch;
	}

private:
	int *_d;
	int _c;
};

enum class Color {red, blue, green};

int main(int argc, char *argv[])
{
	std::cout 
		<< "hardware threads: "
		<< std::thread::hardware_concurrency() 
		<< std::endl;
	A a;
	auto f = std::bind(&A::func, &a);
	std::thread t(f);
	
	int b = {323};
	int c[] = {1,2,4,5,32};
	for(int x:c)
	{
		std::cout << x;
	}

	std::cout << "sizeof IEndpoint: " << sizeof(IEndpoint) << std::endl;

	t.join();
	return 0;
}
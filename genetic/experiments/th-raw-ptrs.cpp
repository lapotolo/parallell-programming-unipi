#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>
#include <utility>
#include <random>
#include <chrono> 
#include <thread>

class Holder
{
public:

  Holder(int size)         // Constructor
  {
    m_data = new int[size];
    m_size = size;
  }

  ~Holder()                // Destructor
  {
    delete[] m_data;
  }

  Holder(const Holder& other) // copy constructor
  {
    m_data = new int[other.m_size];  // (1)
    std::copy(other.m_data, other.m_data + other.m_size, m_data);  // (2)
    m_size = other.m_size;
  }

private:
  int*   m_data;
  size_t m_size;

};

int global = 100;
int& setGlobal() { return global; }


int main()
{

  std::cout<< std::thread::hardware_concurrency() <<"\n";

  char a;
  uint16_t i;
  std::thread th;
  std::thread* th_ptr;
  std::unique_ptr<std::thread> th_un_prt;


  std::cout<< "sizeof char = "<< sizeof(a)<<"\n";
  std::cout<< "sizeof uint16_t = "<< sizeof(i)<<"\n";

  std::cout<< "sizeof std::thread = "<< sizeof(th)<<"\n";
  std::cout<< "sizeof std::thread* = "<< sizeof(th_ptr)<<"\n";
  std::cout<< "sizeof std::unique_ptr<std::thread> = "<< sizeof(th_un_prt)<<"\n";


  std::cout<< "global pre = "<< global <<"\n";

  // ... somewhere in main() ...

  setGlobal() = 400; // OK
  std::cout<< "global pre = "<< global <<"\n";

  int x = 5;
  int* ptr = &x; 
  int& ref = x; // OK
std::cout<<"ptr"<<ptr<<"\n";
std::cout<<"ref"<<ref<<"\n";

  return 0;

}

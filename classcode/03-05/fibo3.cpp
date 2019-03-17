#include <iostream>
#include <chrono>
#include <future>

using namespace std;


// sequential fibonacci
int fib_seq(int n)
{
    if (n < 2)
        return n;
    int x = fib_seq(n-1);
    int y = fib_seq(n-2);
    return x + y;
}

// number of recursive calls in fibonacci
int calls = 0;

int fib_call(int n)
{
  calls++;
  if (n < 2)
    return n;
  int x = fib_call(n-1);
  int y = fib_call(n-2);
  return x + y;
}

// async fibonacci
int fib_async(int n)
{
    if (n < 2)
        return n;
    auto x =  async(launch::async,
		    fib_async,
		    n-1);
    int y = fib_async(n-2);
    return x.get() + y;
}


int main(int argc, char * argv[]) {

  int n = atoi(argv[1]);
  int fibn = 0;

  calls = 0;
  fibn = fib_call(n);
  
  cout << "Fib(" << n << ")=" << fibn << " (" << calls << " rec calls)" << endl;

  auto start = chrono::high_resolution_clock::now();
  fibn = fib_seq(n);
  auto elapsed = chrono::high_resolution_clock::now() - start;
  cout << "Seq time for fib(" << n << ") = " << fibn << " is " <<
    chrono::duration_cast<chrono::microseconds>(elapsed).count() <<
    " usecs" <<endl;

  start = chrono::high_resolution_clock::now();
  fibn = fib_async(n);
  elapsed = chrono::high_resolution_clock::now() - start;
  cout << "Async time for fib(" << n << ") = " << fibn << " is " <<
    chrono::duration_cast<chrono::microseconds>(elapsed).count() <<
    " usecs" <<endl;
  
  
  return(0);
  
}

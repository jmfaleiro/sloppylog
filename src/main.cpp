// A basic sample application using Junction.

#include <iostream>
#include <string>
#include <tbb/concurrent_hash_map.h>

using namespace std;

struct my_hash_compare {

  static size_t hash(const string& x) {
    size_t h = 0;
    for (const char *s = x.c_str(); *s; ++s) 
      h = (h*17)^(*s);
    return h;
  }

  static bool equal(const string& s1, const string& s2) {
    return s1 == s2;
  }
};

int main() {
  tbb::concurrent_hash_map<string, int> temp;  

  tbb::concurrent_hash_map<string, int>::accessor ac;
  temp.insert(ac, "jose");
  ac->second = 1;
  ac.release();

  temp.find(ac, "jose");
  std::cout << "Key: jose. Value: " << ac->second << "\n"; 
  ac.release();

  return 0;
}

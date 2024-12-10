#include <iostream>
#include <cassert>

#include "../include/async/async.h"

using namespace std;
using namespace async;


int main(int argc, const char* argv[])
{
  const auto handle1 = async::connect(3);
  std::cout<<"handle " << handle1 << std::endl;
  async::receive(handle1, "cmd3", 4);
  async::receive(handle1, "cmd2", 4);
  async::receive(handle1, "cmd1", 4);

  async::receive(handle1, "{", 1);
  async::receive(handle1, "cmd22", 5);
  async::receive(handle1, "cmd23", 5);
  async::receive(handle1, "}", 1);
  
  const auto handle2 = async::connect(3);
  async::receive(handle1, "cmd4", 4);

  async::receive(handle2, "cmd13", 5);
  async::receive(handle2, "cmd12", 5);

  async::receive(handle1, "cmd5", 4);

  async::receive(handle2, "cmd11", 5);

  async::receive(handle1, "cmd6", 4);

  async::disconnect(handle1);

  async::disconnect(handle2);
  
  return 0;
}

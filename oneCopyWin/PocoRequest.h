#ifndef CPNET_POCO_REQUEST
#define CPNET_POCO_REQUEST

#include <string>

namespace CPNet {
  class PocoRequest
  {
  public:
    PocoRequest(std::string serverAddr,
      bool secure = true,
      bool noverify = false);
    ~PocoRequest();



    std::string post(std::string request, std::string user = "", std::string passwd = "");

  private:

    std::string serverAddr_;
    bool noverify_;
  };
}


#endif
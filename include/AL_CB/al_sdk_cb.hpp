#ifndef AL_SDK_CB_H
#define AL_SDK_CB_H

#include <string>
#include <vector>

class AlSDKCb {
public:
  virtual ~AlSDKCb() {}
  virtual void sendMessageToPeer(std::vector<char> peerId,
                                 std::vector<char> msg) = 0;
  virtual void initConnection(std::vector<char> peerId,
                              std::vector<char> mode) = 0;
};
//]

#endif // AL_SDK_CB_H

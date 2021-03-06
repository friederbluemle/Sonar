#ifdef FB_SONARKIT_ENABLED

#include <Flipper/SonarStateUpdateListener.h>
#import "FlipperStateUpdateListener.h"

/*
 * This class exists to bridge the gap between Objective C and C++.
 * A SKStateUpdateCPPWrapper instance allows for wrapping an Objective-C object
 * and passing it to the pure C++ SonarClient, so it can be triggered when updates occur.
 */
class SKStateUpdateCPPWrapper : public SonarStateUpdateListener {
public:
  SKStateUpdateCPPWrapper(id<FlipperStateUpdateListener> delegate_);
  void onUpdate();
private:
  __weak id<FlipperStateUpdateListener> delegate_;
};

#endif

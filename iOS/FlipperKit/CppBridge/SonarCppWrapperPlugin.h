/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *
 *  This source code is licensed under the MIT license found in the LICENSE
 *  file in the root directory of this source tree.
 *
 */
#ifndef __OBJC__
#error This header can only be included in .mm (ObjC++) files
#endif

#import <Flipper/SonarPlugin.h>
#import <FlipperKit/CppBridge/FlipperCppBridgingConnection.h>
#import <FlipperKit/FlipperPlugin.h>

namespace facebook {
namespace flipper {

using ObjCPlugin = NSObject<FlipperPlugin> *;

/**
SonarCppWrapperPlugin is a simple C++ wrapper around Objective-C Sonar plugins
that can be passed to SonarClient. This class allows developers to write pure
Objective-C plugins if they want.
*/
class SonarCppWrapperPlugin final : public facebook::flipper::SonarPlugin {
public:
  // Under ARC copying objCPlugin *does* increment its retain count
  SonarCppWrapperPlugin(ObjCPlugin objCPlugin) : _objCPlugin(objCPlugin) {}

  std::string identifier() const override { return [[_objCPlugin identifier] UTF8String]; }

  void didConnect(std::shared_ptr<facebook::flipper::FlipperConnection> conn) override
  {
    FlipperCppBridgingConnection *const bridgingConn = [[FlipperCppBridgingConnection alloc] initWithCppConnection:conn];
    [_objCPlugin didConnect:bridgingConn];
  }

  void didDisconnect() override { [_objCPlugin didDisconnect]; }

  ObjCPlugin getObjCPlugin() { return _objCPlugin; }

private:
  ObjCPlugin _objCPlugin;
};

} // namespace flipper
} // namespace facebook

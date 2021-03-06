/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *
 *  This source code is licensed under the MIT license found in the LICENSE
 *  file in the root directory of this source tree.
 *
 */

#pragma once

#include "FlipperResponder.h"
#include "SonarWebSocket.h"
#include <folly/json.h>

namespace facebook {
namespace flipper {

class SonarResponderImpl : public FlipperResponder {
 public:
  SonarResponderImpl(SonarWebSocket* socket, int64_t responseID)
      : socket_(socket), responseID_(responseID) {}

  void success(const folly::dynamic& response) const override {
    const folly::dynamic message =
        folly::dynamic::object("id", responseID_)("success", response);
    socket_->sendMessage(message);
  }

  void error(const folly::dynamic& response) const override {
    const folly::dynamic message =
        folly::dynamic::object("id", responseID_)("error", response);
    socket_->sendMessage(message);
  }

 private:
  SonarWebSocket* socket_;
  int64_t responseID_;
};

} // namespace flipper
} // namespace facebook

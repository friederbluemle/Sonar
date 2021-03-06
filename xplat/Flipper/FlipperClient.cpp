/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *
 *  This source code is licensed under the MIT license found in the LICENSE
 *  file in the root directory of this source tree.
 *
 */

#include "FlipperClient.h"
#include "FlipperConnectionImpl.h"
#include "SonarResponderImpl.h"
#include "SonarState.h"
#include "SonarStep.h"
#include "SonarWebSocketImpl.h"
#include "ConnectionContextStore.h"
#include "Log.h"
#include <vector>
#include <stdexcept>
#include <iostream>
#include <fstream>

#if FB_SONARKIT_ENABLED

namespace facebook {
namespace flipper {

static FlipperClient* kInstance;

using folly::dynamic;

void FlipperClient::init(FlipperInitConfig config) {
  auto state = std::make_shared<SonarState>();
  auto context = std::make_shared<ConnectionContextStore>(config.deviceData);
  kInstance =
      new FlipperClient(std::make_unique<SonarWebSocketImpl>(std::move(config), state, context), state);
}

FlipperClient* FlipperClient::instance() {
  return kInstance;
}

void FlipperClient::setStateListener(
    std::shared_ptr<SonarStateUpdateListener> stateListener) {
  log("Setting state listener");
  sonarState_->setUpdateListener(stateListener);
}

void FlipperClient::addPlugin(std::shared_ptr<SonarPlugin> plugin) {
  log("FlipperClient::addPlugin " + plugin->identifier());
  auto step = sonarState_->start("Add plugin " + plugin->identifier());

  std::lock_guard<std::mutex> lock(mutex_);
  performAndReportError([this, plugin, step]() {
    if (plugins_.find(plugin->identifier()) != plugins_.end()) {
      throw std::out_of_range(
          "plugin " + plugin->identifier() + " already added.");
    }
    plugins_[plugin->identifier()] = plugin;
    step->complete();
    if (connected_) {
      refreshPlugins();
    }
  });
}

void FlipperClient::removePlugin(std::shared_ptr<SonarPlugin> plugin) {
  log("FlipperClient::removePlugin " + plugin->identifier());

  std::lock_guard<std::mutex> lock(mutex_);
  performAndReportError([this, plugin]() {
    if (plugins_.find(plugin->identifier()) == plugins_.end()) {
      throw std::out_of_range("plugin " + plugin->identifier() + " not added.");
    }
    disconnect(plugin);
    plugins_.erase(plugin->identifier());
    if (connected_) {
      refreshPlugins();
    }
  });
}

std::shared_ptr<SonarPlugin> FlipperClient::getPlugin(
    const std::string& identifier) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (plugins_.find(identifier) == plugins_.end()) {
    return nullptr;
  }
  return plugins_.at(identifier);
}

bool FlipperClient::hasPlugin(const std::string& identifier) {
  std::lock_guard<std::mutex> lock(mutex_);
  return plugins_.find(identifier) != plugins_.end();
}

void FlipperClient::disconnect(std::shared_ptr<SonarPlugin> plugin) {
  const auto& conn = connections_.find(plugin->identifier());
  if (conn != connections_.end()) {
    connections_.erase(plugin->identifier());
    plugin->didDisconnect();
  }
}

void FlipperClient::refreshPlugins() {
    performAndReportError([this]() {
        dynamic message = dynamic::object("method", "refreshPlugins");
        socket_->sendMessage(message);
    });
}

void FlipperClient::onConnected() {
  log("FlipperClient::onConnected");

  std::lock_guard<std::mutex> lock(mutex_);
  connected_ = true;
}

void FlipperClient::onDisconnected() {
  log("FlipperClient::onDisconnected");
  auto step = sonarState_->start("Trigger onDisconnected callbacks");
  std::lock_guard<std::mutex> lock(mutex_);
  connected_ = false;
  performAndReportError([this, step]() {
    for (const auto& iter : plugins_) {
      disconnect(iter.second);
    }
    step->complete();
  });
}

void FlipperClient::onMessageReceived(const dynamic& message) {
  std::lock_guard<std::mutex> lock(mutex_);
  performAndReportError([this, &message]() {
    const auto& method = message["method"];
    const auto& params = message.getDefault("params");

    std::unique_ptr<SonarResponderImpl> responder;
    if (message.find("id") != message.items().end()) {
      responder.reset(
          new SonarResponderImpl(socket_.get(), message["id"].getInt()));
    }

    if (method == "getPlugins") {
      dynamic identifiers = dynamic::array();
      for (const auto& elem : plugins_) {
        identifiers.push_back(elem.first);
      }
      dynamic response = dynamic::object("plugins", identifiers);
      responder->success(response);
      return;
    }

    if (method == "init") {
      const auto identifier = params["plugin"].getString();
      if (plugins_.find(identifier) == plugins_.end()) {
        throw std::out_of_range(
            "plugin " + identifier + " not found for method " +
            method.getString());
      }
      const auto plugin = plugins_.at(identifier);
      auto& conn = connections_[plugin->identifier()];
      conn = std::make_shared<FlipperConnectionImpl>(
          socket_.get(), plugin->identifier());
      plugin->didConnect(conn);
      return;
    }

    if (method == "deinit") {
      const auto identifier = params["plugin"].getString();
      if (plugins_.find(identifier) == plugins_.end()) {
        throw std::out_of_range(
            "plugin " + identifier + " not found for method " +
            method.getString());
      }
      const auto plugin = plugins_.at(identifier);
      disconnect(plugin);
      return;
    }

    if (method == "execute") {
      const auto identifier = params["api"].getString();
      if (connections_.find(identifier) == connections_.end()) {
        throw std::out_of_range(
            "connection " + identifier + " not found for method " +
            method.getString());
      }
      const auto& conn = connections_.at(params["api"].getString());
      conn->call(
          params["method"].getString(),
          params.getDefault("params"),
          std::move(responder));
      return;
    }

    dynamic response =
        dynamic::object("message", "Received unknown method: " + method);
    responder->error(response);
  });
}

void FlipperClient::performAndReportError(const std::function<void()>& func) {
  try {
    func();
  } catch (std::exception& e) {
      dynamic message = dynamic::object(
                                        "error",
                                        dynamic::object("message", e.what())("stacktrace", "<none>"));
    if (connected_) {
      socket_->sendMessage(message);
    } else {
        log("SonarError : " + std::string(e.what()));
    }
  }
}

std::string FlipperClient::getState() {
  return sonarState_->getState();
}

std::vector<StateElement> FlipperClient::getStateElements() {
  return sonarState_->getStateElements();
}

} // namespace flipper
} // namespace facebook

#endif

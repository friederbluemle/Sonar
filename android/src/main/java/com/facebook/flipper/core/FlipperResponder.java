/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *
 *  This source code is licensed under the MIT license found in the LICENSE
 *  file in the root directory of this source tree.
 *
 */
package com.facebook.flipper.core;

/**
 * FlipperResponder is used to asyncronously response to a messaged recieved from the Sonar desktop
 * app. The Sonar Responder will automatically wrap the response in an approriate object depending
 * on if it is an error or not.
 */
public interface FlipperResponder {

  /** Deliver a successful response to the Sonar desktop app. */
  void success(FlipperObject response);

  /** Deliver a successful response to the Sonar desktop app. */
  void success(FlipperArray response);

  /** Deliver a successful response to the Sonar desktop app. */
  void success();

  /** Inform the Sonar desktop app of an error in handling the request. */
  void error(FlipperObject response);
}

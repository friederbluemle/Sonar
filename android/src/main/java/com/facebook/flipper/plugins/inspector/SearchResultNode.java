/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *
 *  This source code is licensed under the MIT license found in the LICENSE
 *  file in the root directory of this source tree.
 *
 */

package com.facebook.flipper.plugins.inspector;

import android.support.annotation.Nullable;
import com.facebook.flipper.core.FlipperArray;
import com.facebook.flipper.core.FlipperObject;
import java.util.List;

public class SearchResultNode {

  private final String id;
  private final boolean isMatch;
  private final FlipperObject element;
  private final FlipperObject axElement;
  @Nullable
  private final List<SearchResultNode> children;

  SearchResultNode(
      String id, boolean isMatch, FlipperObject element, List<SearchResultNode> children, FlipperObject axElement) {
    this.id = id;
    this.isMatch = isMatch;
    this.element = element;
    this.children = children;
    this.axElement = axElement;
  }

  FlipperObject toSonarObject() {
    final FlipperArray childArray;
    if (children != null) {
      final FlipperArray.Builder builder = new FlipperArray.Builder();
      for (SearchResultNode child : children) {
        builder.put(child.toSonarObject());
      }
      childArray = builder.build();
    } else {
      childArray = null;
    }

    return new FlipperObject.Builder()
        .put("id", this.id)
        .put("isMatch", this.isMatch)
        .put("axElement", this.axElement)
        .put("element", this.element)
        .put("children", childArray)
        .build();
  }
}

// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_UTILITY_SAFE_BROWSING_MAC_TEST_UTILS_H_
#define CHROME_UTILITY_SAFE_BROWSING_MAC_TEST_UTILS_H_

#include <vector>

#include "chrome/utility/safe_browsing/mac/read_stream.h"

namespace safe_browsing {
namespace dmg {
namespace test {

// Reads the given |stream| until end-of-stream is reached, storying the read
// bytes into |data|. Returns true on success and false on error.
bool ReadEntireStream(ReadStream* stream, std::vector<uint8_t>* data);

}  // namespace test
}  // namespace dmg
}  // namespace safe_browsing

#endif  // CHROME_UTILITY_SAFE_BROWSING_MAC_TEST_UTILS_H_

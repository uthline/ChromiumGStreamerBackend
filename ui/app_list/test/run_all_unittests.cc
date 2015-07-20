// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/basictypes.h"
#include "base/bind.h"
#include "base/compiler_specific.h"
#include "base/path_service.h"
#include "base/test/launcher/unit_test_launcher.h"
#include "base/test/test_discardable_memory_allocator.h"
#include "base/test/test_suite.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/base/ui_base_paths.h"

#if defined(OS_MACOSX)
#include "base/test/mock_chrome_application_mac.h"
#endif

#if defined(TOOLKIT_VIEWS)
#include "ui/gl/test/gl_surface_test_support.h"
#endif

namespace {

class AppListTestSuite : public base::TestSuite {
 public:
  AppListTestSuite(int argc, char** argv) : base::TestSuite(argc, argv) {}

 protected:
  void Initialize() override {
#if defined(OS_MACOSX)
    mock_cr_app::RegisterMockCrApp();
#endif
#if defined(TOOLKIT_VIEWS)
    gfx::GLSurfaceTestSupport::InitializeOneOff();
#endif
    base::TestSuite::Initialize();
    ui::RegisterPathProvider();

    base::FilePath ui_test_pak_path;
    ASSERT_TRUE(PathService::Get(ui::UI_TEST_PAK, &ui_test_pak_path));
    ui::ResourceBundle::InitSharedInstanceWithPakPath(ui_test_pak_path);

    base::DiscardableMemoryAllocator::SetInstance(
        &discardable_memory_allocator_);
  }

  void Shutdown() override {
    ui::ResourceBundle::CleanupSharedInstance();
    base::TestSuite::Shutdown();
  }

 private:
  base::TestDiscardableMemoryAllocator discardable_memory_allocator_;

  DISALLOW_COPY_AND_ASSIGN(AppListTestSuite);
};

}  // namespace

int main(int argc, char** argv) {
  AppListTestSuite test_suite(argc, argv);

  return base::LaunchUnitTests(
      argc,
      argv,
      base::Bind(&AppListTestSuite::Run, base::Unretained(&test_suite)));
}

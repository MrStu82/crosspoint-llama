#pragma once

#include <functional>

#include "activities/Activity.h"

class StatsActivity final : public Activity {
  const std::function<void()> onGoBack;
  int totalBooksOnDevice = 0;

  int countEpubsRecursively(const char* path);

 public:
  explicit StatsActivity(GfxRenderer& renderer, MappedInputManager& mappedInput, const std::function<void()>& onGoBack)
      : Activity("Stats", renderer, mappedInput), onGoBack(onGoBack) {}

  void onEnter() override;
  void onExit() override;
  void loop() override;
  void render(Activity::RenderLock&&) override;
};

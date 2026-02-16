#pragma once

#include <GfxRenderer.h>
#include <I18n.h>

#include <ctime>
#include <functional>

#include "../Activity.h"
#include "components/UITheme.h"


class MappedInputManager;

class TimeSettingsActivity final : public Activity {
 public:
  explicit TimeSettingsActivity(GfxRenderer& renderer, MappedInputManager& mappedInput,
                                const std::function<void()>& onBack)
      : Activity("TimeSettings", renderer, mappedInput), onBack(onBack) {}

  void onEnter() override;
  void onExit() override;
  void loop() override;
  void render(Activity::RenderLock&&) override;

 private:
  void saveTime();

  std::function<void()> onBack;

  enum Field { YEAR = 0, MONTH, DAY, HOUR, MINUTE, FIELD_COUNT };
  int selectedField = YEAR;

  int year;
  int month;
  int day;
  int hour;
  int minute;
};

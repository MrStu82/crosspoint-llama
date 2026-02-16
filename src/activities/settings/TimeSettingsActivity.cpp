#include "TimeSettingsActivity.h"

#include <GfxRenderer.h>
#include <I18n.h>
#include <sys/time.h>

#include "MappedInputManager.h"
#include "fontIds.h"

void TimeSettingsActivity::onEnter() {
  Activity::onEnter();

  time_t now;
  time(&now);
  struct tm timeinfo;
  localtime_r(&now, &timeinfo);

  if (timeinfo.tm_year < 100) {
    // Default to Jan 1, 2026 12:00 if unset
    year = 2026;
    month = 1;
    day = 1;
    hour = 12;
    minute = 0;
  } else {
    year = timeinfo.tm_year + 1900;
    month = timeinfo.tm_mon + 1;
    day = timeinfo.tm_mday;
    hour = timeinfo.tm_hour;
    minute = timeinfo.tm_min;
  }
  selectedField = YEAR;

  requestUpdate();
}

void TimeSettingsActivity::onExit() { Activity::onExit(); }

int getDaysInMonth(int m, int y) {
  if (m == 2) {
    return ((y % 4 == 0 && y % 100 != 0) || (y % 400 == 0)) ? 29 : 28;
  }
  if (m == 4 || m == 6 || m == 9 || m == 11) return 30;
  return 31;
}

void TimeSettingsActivity::loop() {
  if (mappedInput.wasPressed(MappedInputManager::Button::Back) ||
      mappedInput.wasPressed(MappedInputManager::Button::Confirm)) {
    saveTime();
    return;
  }

  bool changed = false;
  if (mappedInput.wasPressed(MappedInputManager::Button::Left)) {
    selectedField = (selectedField + FIELD_COUNT - 1) % FIELD_COUNT;
    changed = true;
  } else if (mappedInput.wasPressed(MappedInputManager::Button::Right)) {
    selectedField = (selectedField + 1) % FIELD_COUNT;
    changed = true;
  } else if (mappedInput.wasPressed(MappedInputManager::Button::Up)) {
    changed = true;
    switch (selectedField) {
      case YEAR:
        year++;
        break;
      case MONTH:
        month = (month % 12) + 1;
        break;
      case DAY:
        day = (day % getDaysInMonth(month, year)) + 1;
        break;
      case HOUR:
        hour = (hour + 1) % 24;
        break;
      case MINUTE:
        minute = (minute + 1) % 60;
        break;
    }
  } else if (mappedInput.wasPressed(MappedInputManager::Button::Down)) {
    changed = true;
    switch (selectedField) {
      case YEAR:
        year--;
        if (year < 2020) year = 2020;
        break;
      case MONTH:
        month = (month == 1) ? 12 : month - 1;
        break;
      case DAY:
        day = (day == 1) ? getDaysInMonth(month, year) : day - 1;
        break;
      case HOUR:
        hour = (hour == 0) ? 23 : hour - 1;
        break;
      case MINUTE:
        minute = (minute == 0) ? 59 : minute - 1;
        break;
    }
  }

  if (changed) {
    // Re-clamp day if month changed
    int maxDays = getDaysInMonth(month, year);
    if (day > maxDays) day = maxDays;
    requestUpdate();
  }
}

void TimeSettingsActivity::saveTime() {
  struct tm t = {0};
  t.tm_year = year - 1900;
  t.tm_mon = month - 1;
  t.tm_mday = day;
  t.tm_hour = hour;
  t.tm_min = minute;
  t.tm_sec = 0;
  t.tm_isdst = -1;

  time_t epoch = mktime(&t);

  if (epoch != -1) {
    struct timeval tv;
    tv.tv_sec = epoch;
    tv.tv_usec = 0;
    settimeofday(&tv, NULL);
  }

  onBack();
}

void TimeSettingsActivity::render(Activity::RenderLock&&) {
  renderer.clearScreen();

  const auto pageWidth = renderer.getScreenWidth();

  renderer.drawCenteredText(UI_12_FONT_ID, 15, tr(STR_SET_DATE_TIME), true, EpdFontFamily::BOLD);

  int y = 50 + 20;

  char buf[8];
  int blockW = 85;
  int startX = (pageWidth - (blockW * 3)) / 2;

  // Date Row
  renderer.drawText(UI_10_FONT_ID, startX, y - 20, "Year");
  snprintf(buf, sizeof(buf), "%04d", year);
  if (selectedField == YEAR) renderer.fillRect(startX - 4, y - 2, blockW - 6, 24);
  renderer.drawText(UI_12_FONT_ID, startX, y, buf, selectedField != YEAR);

  renderer.drawText(UI_10_FONT_ID, startX + blockW, y - 20, "Month");
  snprintf(buf, sizeof(buf), "%02d", month);
  if (selectedField == MONTH) renderer.fillRect(startX + blockW - 4, y - 2, blockW - 6, 24);
  renderer.drawText(UI_12_FONT_ID, startX + blockW, y, buf, selectedField != MONTH);

  renderer.drawText(UI_10_FONT_ID, startX + blockW * 2, y - 20, "Day");
  snprintf(buf, sizeof(buf), "%02d", day);
  if (selectedField == DAY) renderer.fillRect(startX + blockW * 2 - 4, y - 2, blockW - 6, 24);
  renderer.drawText(UI_12_FONT_ID, startX + blockW * 2, y, buf, selectedField != DAY);

  // Time Row
  y += 70;
  startX = (pageWidth - (blockW * 2)) / 2;

  renderer.drawText(UI_10_FONT_ID, startX, y - 20, "Hour");
  snprintf(buf, sizeof(buf), "%02d", hour);
  if (selectedField == HOUR) renderer.fillRect(startX - 4, y - 2, blockW - 6, 24);
  renderer.drawText(UI_12_FONT_ID, startX, y, buf, selectedField != HOUR);

  renderer.drawText(UI_10_FONT_ID, startX + blockW, y - 20, "Minute");
  snprintf(buf, sizeof(buf), "%02d", minute);
  if (selectedField == MINUTE) renderer.fillRect(startX + blockW - 4, y - 2, blockW - 6, 24);
  renderer.drawText(UI_12_FONT_ID, startX + blockW, y, buf, selectedField != MINUTE);

  // Button hints
  const auto labels = mappedInput.mapLabels(tr(STR_BACK), tr(STR_SAVE), tr(STR_DIR_UP), tr(STR_DIR_DOWN));
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);

  renderer.displayBuffer();
}

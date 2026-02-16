#include "StatsManager.h"

#include <HalStorage.h>

static const char* STATS_FILE_PATH = "/.crosspoint/stats.bin";

#include <time.h>

StatsManager::StatsManager() { load(); }

void StatsManager::load() {
  FsFile file;
  if (!Storage.openFileForRead("STM", STATS_FILE_PATH, file)) {
    // initialize defaults if file doesn't exist
    stats.totalPagesRead = 0;
    stats.booksOpened = 0;
    return;
  }

  size_t readLen = file.read(reinterpret_cast<uint8_t*>(&stats), sizeof(GlobalStats));
  if (readLen == 0) {
    // Empty file
    stats = GlobalStats{};
  } else if (readLen < sizeof(GlobalStats)) {
    // graceful migration from older, smaller struct
    // The missing fields will be implicitly 0 from the constructor
    dirty = true;  // Needs saving to expand
  }

  file.close();

  checkDateReset();
}

void StatsManager::save() {
  if (!dirty) return;
  FsFile file;
  if (Storage.openFileForWrite("STM", STATS_FILE_PATH, file)) {
    file.write(reinterpret_cast<const uint8_t*>(&stats), sizeof(GlobalStats));
    file.close();
    dirty = false;
  }
}

void StatsManager::incrementPagesRead(uint32_t count) {
  checkDateReset();
  stats.totalPagesRead += count;
  stats.pagesReadToday += count;
  dirty = true;
}

void StatsManager::incrementBooksOpened() {
  checkDateReset();
  stats.booksOpened += 1;
  dirty = true;
}

void StatsManager::incrementBooksFinished() {
  checkDateReset();
  stats.booksFinished += 1;
  dirty = true;
}

void StatsManager::addReadingTimeSeconds(uint32_t seconds) {
  checkDateReset();
  stats.totalReadingTimeSeconds += seconds;
  stats.readingTimeTodaySeconds += seconds;
  dirty = true;
}

int StatsManager::getCurrentDate() const {
  time_t now;
  struct tm timeinfo;
  time(&now);
  localtime_r(&now, &timeinfo);

  if (timeinfo.tm_year < 100) {
    // Time not synced yet (esp32 defaults to 1970)
    // We'll just return 0, which won't trigger standard day rollover cleanly if time never syncs,
    // but the alternative is logging everything to 1970-01-01.
    return 0;
  }

  return (timeinfo.tm_year + 1900) * 10000 + (timeinfo.tm_mon + 1) * 100 + timeinfo.tm_mday;
}

void StatsManager::checkDateReset() {
  int today = getCurrentDate();
  if (today == 0) return;  // Time not set

  if (stats.lastActiveDate == 0 || stats.lastActiveDate != today) {
    // Date changed! Reset daily counters
    stats.readingTimeTodaySeconds = 0;
    stats.pagesReadToday = 0;
    stats.lastActiveDate = today;
    dirty = true;
  }
}

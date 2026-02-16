#include "BookmarkManager.h"

#include <HalStorage.h>

#include <algorithm>
#include <functional>


BookmarkManager::BookmarkManager(const std::string& bookPath) : bookPath(bookPath) { load(); }

std::string BookmarkManager::getBookmarksFilePath() const {
  // Use the same hashing scheme as progress.bin, but for bookmarks.bin
  std::string hashStr = std::to_string(std::hash<std::string>{}(bookPath));
  return std::string("/.crosspoint/cache/") + hashStr + "/bookmarks.bin";
}

void BookmarkManager::load() {
  bookmarks.clear();
  std::string path = getBookmarksFilePath();
  FsFile file;
  if (!Storage.openFileForRead("BKM", path.c_str(), file)) return;

  uint32_t count = 0;
  if (file.read(reinterpret_cast<uint8_t*>(&count), sizeof(count)) != sizeof(count)) {
    file.close();
    return;
  }

  for (uint32_t i = 0; i < count; ++i) {
    Bookmark bm;
    if (file.read(reinterpret_cast<uint8_t*>(&bm), sizeof(Bookmark)) == sizeof(Bookmark)) {
      bookmarks.push_back(bm);
    }
  }
  file.close();
}

void BookmarkManager::save() const {
  std::string path = getBookmarksFilePath();
  FsFile file;
  if (!Storage.openFileForWrite("BKM", path.c_str(), file)) {
    // Ensure cache directory exists
    std::string hashStr = std::to_string(std::hash<std::string>{}(bookPath));
    std::string dirPath = std::string("/.crosspoint/cache/") + hashStr;
    Storage.mkdir(dirPath.c_str());
    if (!Storage.openFileForWrite("BKM", path.c_str(), file)) return;
  }

  uint32_t count = bookmarks.size();
  file.write(reinterpret_cast<const uint8_t*>(&count), sizeof(count));
  for (const auto& bm : bookmarks) {
    file.write(reinterpret_cast<const uint8_t*>(&bm), sizeof(Bookmark));
  }
  file.close();
}

bool BookmarkManager::toggleBookmark(int spineIndex, int pageNumber) {
  Bookmark target{spineIndex, pageNumber};
  auto it = std::find(bookmarks.begin(), bookmarks.end(), target);

  if (it != bookmarks.end()) {
    bookmarks.erase(it);
    save();
    return false;  // Removed
  } else {
    bookmarks.push_back(target);
    save();
    return true;  // Added
  }
}

bool BookmarkManager::isBookmarked(int spineIndex, int pageNumber) const {
  Bookmark target{spineIndex, pageNumber};
  return std::find(bookmarks.begin(), bookmarks.end(), target) != bookmarks.end();
}

#pragma once

#include <string>
#include <vector>

struct Bookmark {
  int spineIndex;
  int pageNumber;

  bool operator==(const Bookmark& other) const {
    return spineIndex == other.spineIndex && pageNumber == other.pageNumber;
  }
};

class BookmarkManager {
  std::string bookPath;
  std::vector<Bookmark> bookmarks;

  std::string getBookmarksFilePath() const;

 public:
  explicit BookmarkManager(const std::string& bookPath);

  // Load bookmarks from disk
  void load();
  // Save bookmarks to disk
  void save() const;

  // Add or remove a bookmark for the given spine and page (returns true if it was added, false if removed)
  bool toggleBookmark(int spineIndex, int pageNumber);

  // Check if a specific page is bookmarked
  bool isBookmarked(int spineIndex, int pageNumber) const;

  const std::vector<Bookmark>& getBookmarks() const { return bookmarks; }
};

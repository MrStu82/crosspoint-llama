#pragma once
#include <Epub.h>

#include <memory>

#include "../ActivityWithSubactivity.h"
#include "BookmarkManager.h"
#include "util/ButtonNavigator.h"

class EpubReaderBookmarkSelectionActivity final : public ActivityWithSubactivity {
  std::shared_ptr<Epub> epub;
  std::vector<Bookmark> bookmarks;
  ButtonNavigator buttonNavigator;
  int selectorIndex = 0;

  const std::function<void()> onGoBack;
  const std::function<void(int newSpineIndex, int newPage)> onSelectBookmark;

  int getPageItems() const;
  int getTotalItems() const;

 public:
  explicit EpubReaderBookmarkSelectionActivity(
      GfxRenderer& renderer, MappedInputManager& mappedInput, const std::shared_ptr<Epub>& epub,
      const std::vector<Bookmark>& bookmarks, const std::function<void()>& onGoBack,
      const std::function<void(int newSpineIndex, int newPage)>& onSelectBookmark)
      : ActivityWithSubactivity("EpubReaderBookmarkSelection", renderer, mappedInput),
        epub(epub),
        bookmarks(bookmarks),
        onGoBack(onGoBack),
        onSelectBookmark(onSelectBookmark) {}

  void onEnter() override;
  void onExit() override;
  void loop() override;
  void render(Activity::RenderLock&&) override;
};

#include "EpubReaderBookmarkSelectionActivity.h"

#include <GfxRenderer.h>
#include <I18n.h>

#include <cstdio>

#include "MappedInputManager.h"
#include "components/UITheme.h"
#include "fontIds.h"


int EpubReaderBookmarkSelectionActivity::getTotalItems() const { return bookmarks.size(); }

int EpubReaderBookmarkSelectionActivity::getPageItems() const {
  constexpr int lineHeight = 30;
  const int screenHeight = renderer.getScreenHeight();
  const auto orientation = renderer.getOrientation();
  const bool isPortraitInverted = orientation == GfxRenderer::Orientation::PortraitInverted;
  const int hintGutterHeight = isPortraitInverted ? 50 : 0;
  const int startY = 60 + hintGutterHeight;
  const int availableHeight = screenHeight - startY - lineHeight;
  return std::max(1, availableHeight / lineHeight);
}

void EpubReaderBookmarkSelectionActivity::onEnter() {
  ActivityWithSubactivity::onEnter();
  selectorIndex = 0;
  requestUpdate();
}

void EpubReaderBookmarkSelectionActivity::onExit() { ActivityWithSubactivity::onExit(); }

void EpubReaderBookmarkSelectionActivity::loop() {
  const int pageItems = getPageItems();
  const int totalItems = getTotalItems();

  if (totalItems == 0) {
    if (mappedInput.wasReleased(MappedInputManager::Button::Confirm) ||
        mappedInput.wasReleased(MappedInputManager::Button::Back)) {
      onGoBack();
    }
    return;
  }

  if (mappedInput.wasReleased(MappedInputManager::Button::Confirm)) {
    const auto& bm = bookmarks[selectorIndex];
    onSelectBookmark(bm.spineIndex, bm.pageNumber);
  } else if (mappedInput.wasReleased(MappedInputManager::Button::Back)) {
    onGoBack();
  }

  buttonNavigator.onNextRelease([this, totalItems] {
    selectorIndex = ButtonNavigator::nextIndex(selectorIndex, totalItems);
    requestUpdate();
  });

  buttonNavigator.onPreviousRelease([this, totalItems] {
    selectorIndex = ButtonNavigator::previousIndex(selectorIndex, totalItems);
    requestUpdate();
  });

  buttonNavigator.onNextContinuous([this, totalItems, pageItems] {
    selectorIndex = ButtonNavigator::nextPageIndex(selectorIndex, totalItems, pageItems);
    requestUpdate();
  });

  buttonNavigator.onPreviousContinuous([this, totalItems, pageItems] {
    selectorIndex = ButtonNavigator::previousPageIndex(selectorIndex, totalItems, pageItems);
    requestUpdate();
  });
}

void EpubReaderBookmarkSelectionActivity::render(Activity::RenderLock&&) {
  renderer.clearScreen();

  const auto pageWidth = renderer.getScreenWidth();
  const auto orientation = renderer.getOrientation();
  const bool isLandscapeCw = orientation == GfxRenderer::Orientation::LandscapeClockwise;
  const bool isLandscapeCcw = orientation == GfxRenderer::Orientation::LandscapeCounterClockwise;
  const bool isPortraitInverted = orientation == GfxRenderer::Orientation::PortraitInverted;
  const int hintGutterWidth = (isLandscapeCw || isLandscapeCcw) ? 30 : 0;
  const int contentX = isLandscapeCw ? hintGutterWidth : 0;
  const int contentWidth = pageWidth - hintGutterWidth;
  const int hintGutterHeight = isPortraitInverted ? 50 : 0;
  const int contentY = hintGutterHeight;
  const int pageItems = getPageItems();
  const int totalItems = getTotalItems();

  const int titleX =
      contentX + (contentWidth - renderer.getTextWidth(UI_12_FONT_ID, tr(STR_GO_TO_BOOKMARK), EpdFontFamily::BOLD)) / 2;
  renderer.drawText(UI_12_FONT_ID, titleX, 15 + contentY, tr(STR_GO_TO_BOOKMARK), true, EpdFontFamily::BOLD);

  if (totalItems == 0) {
    renderer.drawCenteredText(UI_12_FONT_ID, 200 + contentY, "No bookmarks set", true);
  } else {
    const auto pageStartIndex = selectorIndex / pageItems * pageItems;
    renderer.fillRect(contentX, 60 + contentY + (selectorIndex % pageItems) * 30 - 2, contentWidth - 1, 30);

    for (int i = 0; i < pageItems; i++) {
      int itemIndex = pageStartIndex + i;
      if (itemIndex >= totalItems) break;
      const int displayY = 60 + contentY + i * 30;
      const bool isSelected = (itemIndex == selectorIndex);

      const auto& bm = bookmarks[itemIndex];
      std::string label = "Chapter ";

      int tocIndex = epub->getTocIndexForSpineIndex(bm.spineIndex);
      if (tocIndex != -1) {
        label = epub->getTocItem(tocIndex).title;
      } else {
        label += std::to_string(bm.spineIndex + 1);
      }

      char pageStr[32];
      snprintf(pageStr, sizeof(pageStr), " - %s %d", tr(STR_PAGE), bm.pageNumber + 1);
      label += pageStr;

      const std::string displayName =
          renderer.truncatedText(UI_10_FONT_ID, label.c_str(), contentWidth - 40 - contentX - 20);

      renderer.drawText(UI_10_FONT_ID, contentX + 20, displayY, displayName.c_str(), !isSelected);
    }
  }

  const auto labels = mappedInput.mapLabels(tr(STR_BACK), tr(STR_SELECT), tr(STR_DIR_UP), tr(STR_DIR_DOWN));
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);

  renderer.displayBuffer();
}

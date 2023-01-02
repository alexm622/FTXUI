#include <algorithm>   // for max, min
#include <cstddef>     // for size_t
#include <functional>  // for function
#include <memory>      // for shared_ptr
#include <sstream>
#include <string>                                 // for string, allocator
#include <utility>                                // for move
#include <vector>                                 // for vector
#include "ftxui/component/captured_mouse.hpp"     // for CapturedMouse
#include "ftxui/component/component.hpp"          // for Make, Input
#include "ftxui/component/component_base.hpp"     // for ComponentBase
#include "ftxui/component/component_options.hpp"  // for TextAreaOption
#include "ftxui/component/event.hpp"  // for Event, Event::ArrowLeft, Event::ArrowLeftCtrl, Event::ArrowRight, Event::ArrowRightCtrl, Event::Backspace, Event::Custom, Event::Delete, Event::End, Event::Home, Event::Return
#include "ftxui/component/mouse.hpp"  // for Mouse, Mouse::Left, Mouse::Pressed
#include "ftxui/component/screen_interactive.hpp"  // for Component
#include "ftxui/dom/elements.hpp"  // for operator|, text, Element, reflect, operator|=, flex, inverted, hbox, size, bold, dim, focus, focusCursorBarBlinking, frame, select, Decorator, EQUAL, HEIGHT
#include "ftxui/screen/box.hpp"    // for Box
#include "ftxui/screen/string.hpp"  // for GlyphPosition, WordBreakProperty, GlyphCount, Utf8ToWordBreakProperty, CellToGlyphIndex, WordBreakProperty::ALetter, WordBreakProperty::CR, WordBreakProperty::Double_Quote, WordBreakProperty::Extend, WordBreakProperty::ExtendNumLet, WordBreakProperty::Format, WordBreakProperty::Hebrew_Letter, WordBreakProperty::Katakana, WordBreakProperty::LF, WordBreakProperty::MidLetter, WordBreakProperty::MidNum, WordBreakProperty::MidNumLet, WordBreakProperty::Newline, WordBreakProperty::Numeric, WordBreakProperty::Regional_Indicator, WordBreakProperty::Single_Quote, WordBreakProperty::WSegSpace, WordBreakProperty::ZWJ
#include "ftxui/screen/util.hpp"    // for clamp
#include "ftxui/util/ref.hpp"       // for StringRef, Ref, ConstStringRef

namespace ftxui {

namespace {

std::vector<std::string> Split(const std::string& input) {
  std::vector<std::string> output;
  std::stringstream ss(input);
  std::string line;
  while (std::getline(ss, line)) {
    output.push_back(line);
  }
  return output;
}

// Group together several propertiej so they appear to form a similar group.
// For instance, letters are grouped with number and form a single word.
//bool IsWordCharacter(WordBreakProperty property) {
  //switch (property) {
    //case WordBreakProperty::ALetter:
    //case WordBreakProperty::Hebrew_Letter:
    //case WordBreakProperty::Katakana:
    //case WordBreakProperty::Numeric:
      //return true;

    //case WordBreakProperty::CR:
    //case WordBreakProperty::Double_Quote:
    //case WordBreakProperty::LF:
    //case WordBreakProperty::MidLetter:
    //case WordBreakProperty::MidNum:
    //case WordBreakProperty::MidNumLet:
    //case WordBreakProperty::Newline:
    //case WordBreakProperty::Single_Quote:
    //case WordBreakProperty::WSegSpace:
    //// Unsure:
    //case WordBreakProperty::Extend:
    //case WordBreakProperty::ExtendNumLet:
    //case WordBreakProperty::Format:
    //case WordBreakProperty::Regional_Indicator:
    //case WordBreakProperty::ZWJ:
      //return false;
  //}
  //return true;  // NOT_REACHED();
//}

//std::string PasswordField(size_t size) {
  //std::string out;
  //out.reserve(2 * size);
  //while (size--) {
    //out += "•";
  //}
  //return out;
//}

// An input box. The user can type text into it.
class TextAreaBase : public ComponentBase {
 public:
  // NOLINTNEXTLINE
  TextAreaBase(StringRef content, Ref<TextAreaOption> option)
      : content_(std::move(content)), option_(std::move(option)) {}

  // Component implementation:
  Element Render() override {
    Elements lines;
    std::vector<std::string> content_lines = Split(*content_);
    for (size_t i = 0; i < content_lines.size(); ++i) {
      if (int(i) != *(option_->cursor_line)) {
        lines.push_back(text(content_lines[i]));
        continue;
      }

      const int size = GlyphCount(content_lines[i]);

      int& cursor_column = *(option_->cursor_column);
      cursor_column = std::max(0, std::min<int>(size, cursor_column));

      const bool is_focused = Focused();

      // placeholder.
      if (size == 0) {
        auto element = text(*(option_->placeholder)) | dim | reflect(box_);
        if (is_focused) {
          element |= focus;
        }
        if (hovered_ || is_focused) {
          element |= inverted;
        }
        lines.push_back(element);
        continue;
      }

      // Not focused.
      if (!is_focused) {
        auto element = text(content_lines[i]) | reflect(box_);
        if (hovered_) {
          element |= inverted;
        }
        lines.push_back(std::move(element));
        continue;
      }

      const int index_before_cursor =
          GlyphPosition(content_lines[i], cursor_column);
      const int index_after_cursor =
          GlyphPosition(content_lines[i], 1, index_before_cursor);
      const std::string part_before_cursor =
          content_lines[i].substr(0, index_before_cursor);
      std::string part_at_cursor = " ";
      if (cursor_column < size) {
        part_at_cursor = content_lines[i].substr(
            index_before_cursor, index_after_cursor - index_before_cursor);
      }
      const std::string part_after_cursor =
          content_lines[i].substr(index_after_cursor);
      auto focused = (is_focused || hovered_) ? focusCursorBarBlinking : select;
      auto element = hbox({
                         text(part_before_cursor),
                         text(part_at_cursor) | focused | reflect(cursor_box_),
                         text(part_after_cursor),
                     }) |
                     xflex | frame | bold | reflect(box_);
      lines.push_back(element);
    }
    lines.push_back(text("end") | flex);
    return vbox(std::move(lines));
  }

  bool OnEvent(Event event) override {
    std::vector<std::string> content_lines = Split(*content_);
    int& cursor_line = option_->cursor_line();
    int& cursor_column = option_->cursor_column();
    int line_start_position = 0;
    for(int i = 0; i<cursor_line; ++i) {
      line_start_position += (int)content_lines[i].size() + 1;
    }
    std::string empty_string;
    cursor_line = std::max(std::min(cursor_line, (int)content_lines.size()), 0);
    const std::string& line = cursor_line < (int)content_lines.size()
                                  ? content_lines[cursor_line]
                                  : empty_string;
    cursor_column = std::max(std::min(cursor_column, (int)line.size()), 0);

    if (event.is_mouse()) {
      return OnMouseEvent(event);
    }

    // Backspace.
    if (event == Event::Backspace) {
      if (cursor_column == 0) {
        return false;
      }
      const size_t start =
          GlyphPosition(*content_, cursor_column - 1, line_start_position);
      const size_t end =
          GlyphPosition(*content_, cursor_column, line_start_position);
      content_->erase(start, end - start);
      cursor_column--;
      option_->on_change();
      return true;
    }

    // Delete
    if (event == Event::Delete) {
      if (cursor_column == int(content_->size())) {
        return false;
      }
      const size_t start = GlyphPosition(*content_, cursor_column);
      const size_t end = GlyphPosition(*content_, cursor_column + 1);
      content_->erase(start, end - start);
      option_->on_change();
      return true;
    }

    if (event == Event::Custom) {
      return false;
    }


    // Arrow
    if (event == Event::ArrowUp && cursor_line > 0) {
      cursor_line--;
      return true;
    }

    if (event == Event::ArrowDown && cursor_line < (int)content_lines.size()) {
      cursor_line++;
      return true;
    }

    if (event == Event::ArrowLeft && cursor_column > 0) {
      cursor_column--;
      return true;
    }

    if (event == Event::ArrowRight &&
        cursor_column < (int)line.size()) {
      cursor_column++;
      return true;
    }

    // CTRL + Arrow:
    if (event == Event::ArrowLeftCtrl) {
      HandleLeftCtrl();
      return true;
    }
    if (event == Event::ArrowRightCtrl) {
      HandleRightCtrl();
      return true;
    }

    if (event == Event::Home) {
      cursor_line = 0;
      cursor_column = 0;
      return true;
    }

    if (event == Event::End) {
      //cursor_position() = GlyphCount(*content_);
      return true;
    }

    // Enter.
    if (event == Event::Return) {
      const size_t start =
          GlyphPosition(*content_, cursor_column, line_start_position);
      content_->insert(start, "\n");
      cursor_line++;
      cursor_column = 0;
      option_->on_change();
      return true;
    }

    // Content
    if (event.is_character()) {
      const size_t start =
          GlyphPosition(*content_, cursor_column, line_start_position);
      content_->insert(start, event.character());
      cursor_column++;
      option_->on_change();
      return true;
    }
    return false;
  }

 private:
  void HandleLeftCtrl() {
    auto properties = Utf8ToWordBreakProperty(*content_);

    //// Move left, as long as left is not a word character.
    //while (cursor_position() > 0 &&
           //!IsWordCharacter(properties[cursor_position() - 1])) {
      //cursor_position()--;
    //}

    //// Move left, as long as left is a word character:
    //while (cursor_position() > 0 &&
           //IsWordCharacter(properties[cursor_position() - 1])) {
      //cursor_position()--;
    //}
  }

  void HandleRightCtrl() {
    //auto properties = Utf8ToWordBreakProperty(*content_);
    //const int max = (int)properties.size();

    //// Move right, as long as right is not a word character.
    //while (cursor_position() < max &&
           //!IsWordCharacter(properties[cursor_position()])) {
      //cursor_position()++;
    //}

    //// Move right, as long as right is a word character:
    //while (cursor_position() < max &&
           //IsWordCharacter(properties[cursor_position()])) {
      //cursor_position()++;
    //}
  }

  bool OnMouseEvent(Event event) {
    hovered_ =
        box_.Contain(event.mouse().x, event.mouse().y) && CaptureMouse(event);
    if (!hovered_) {
      return false;
    }

    if (event.mouse().button != Mouse::Left ||
        event.mouse().motion != Mouse::Pressed) {
      return false;
    }

    TakeFocus();
    if (content_->empty()) {
      return true;
    }

    //auto mapping = CellToGlyphIndex(*content_);
    //int original_glyph = cursor_position();
    //original_glyph = util::clamp(original_glyph, 0, int(mapping.size()));
    //size_t original_cell = 0;
    //for (size_t i = 0; i < mapping.size(); i++) {
      //if (mapping[i] == original_glyph) {
        //original_cell = (int)i;
        //break;
      //}
    //}
    //if (mapping[original_cell] != original_glyph) {
      //original_cell = mapping.size();
    //}
    //const int target_cell =
        //int(original_cell) + event.mouse().x - cursor_box_.x_min;
    //int target_glyph = target_cell < (int)mapping.size() ? mapping[target_cell]
                                                         //: (int)mapping.size();
    //target_glyph = util::clamp(target_glyph, 0, GlyphCount(*content_));
    //if (cursor_position() != target_glyph) {
      //cursor_position() = target_glyph;
      //option_->on_change();
    //}
    return true;
  }

  bool Focusable() const final { return true; }

  bool hovered_ = false;
  StringRef content_;

  Box box_;
  Box cursor_box_;
  Ref<TextAreaOption> option_;
};

}  // namespace

/// @brief An input box for editing text.
/// @param content The editable content.
/// @param placeholder The text displayed when content is still empty.
/// @param option Additional optional parameters.
/// @ingroup component
/// @see TextAreaBase
///
/// ### Example
///
/// ```cpp
/// auto screen = ScreenInteractive::FitComponent();
/// std::string content= "";
/// std::string placeholder = "placeholder";
/// Component input = Input(&content, &placeholder);
/// screen.Loop(input);
/// ```
///
/// ### Output
///
/// ```bash
/// placeholder
/// ```
Component TextArea(StringRef content, Ref<TextAreaOption> option) {
  return Make<TextAreaBase>(std::move(content), std::move(option));
}

}  // namespace ftxui

// Copyright 2022 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

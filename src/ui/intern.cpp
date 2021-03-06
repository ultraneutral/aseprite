// Aseprite UI Library
// Copyright (C) 2001-2013, 2015  David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ui/manager.h"
#include "ui/theme.h"
#include "ui/widget.h"
#include "ui/window.h"

#include <list>

namespace ui {

static std::list<Widget*>* widgets;

int _ji_widgets_init()
{
  widgets = new std::list<Widget*>;
  return 0;
}

void _ji_widgets_exit()
{
  delete widgets;
}

void addWidget(Widget* widget)
{
  widgets->push_back(widget);
}

void removeWidget(Widget* widget)
{
  auto it = std::find(widgets->begin(), widgets->end(), widget);
  if (it != widgets->end())
    widgets->erase(it);
}

void resetFontAllWidgets()
{
  for (auto widget : *widgets)
    widget->resetFont();
}

void reinitThemeForAllWidgets()
{
  // Reinitialize the theme of each widget
  for (auto widget : *widgets) {
    widget->setTheme(CurrentTheme::get());
    widget->initTheme();
  }

  // Remap the windows
  for (auto widget : *widgets) {
    if (widget->type == kWindowWidget)
      static_cast<Window*>(widget)->remapWindow();
  }

  // Redraw the whole screen
  Manager::getDefault()->invalidate();
}

} // namespace ui

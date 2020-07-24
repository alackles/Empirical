//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2020.
//  Released under the MIT Software license; see doc/LICENSE

#include <iostream>

#include "web/web.h"
#include "web/Div.h"
#include "prefab/CommentBox.h"

namespace UI = emp::web;

UI::Document doc("emp_base");

int main()
{
  emp::prefab::CommentBox box("comment_box");
  doc << box;
  box.AddContent("<h1>Comment Box Title</h1>");
  UI::Div body("body");
  box.AddContent(body);
  body << "More content for comment box";
}

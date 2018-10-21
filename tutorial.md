All the dom element are declared in one header:
~~~cpp
#include <ftxui/dom/elements.hpp>
~~~

It declares the following set of elements:

~~~cpp
// --- Layout ----
Element vbox(Children);
Element hbox(Children);
Element flex();

// --- Widget --
Element text(std::wstring text);
Element separator();
Element gauge(float ratio);
Element frame(Child);
Element frame(Child title, Child content);

// -- Decorator (Style) ---
Element bold(Element);
Element dim(Element);
Element inverted(Element);
Element underlined(Element);
Element color(Color,Element);
Element bgcolor(Element);

// --- Decorator ---
Element hcenter(Element);
Element vcenter(Element);
Element center(Element);
Element flex(Element);
~~~

# Layout elements.

vbox (Vertical-box) and hbox (Horizontal-box) are containers. They are used to
compose all the elements together. They will display their children one by one in one direction.
Each elements will occupy the space it required plus a fraction of the remaining
space dispatched to all the flexible elements.

flex() is used to make an element flexible.

## Examples
~~~cpp
  hbox(
    frame(text(L"left")),
    flex(frame(text(L"middle"))),
    frame(text(L"right"))
  );
~~~
~~~bash
┌────┐┌─────────────────────────────────────────────────────────────────┐┌─────┐
│left││middle                                                           ││right│
└────┘└─────────────────────────────────────────────────────────────────┘└─────┘
~~~

~~~cpp
  hbox(
    frame(text(L"left")),
    flex(frame(text(L"middle"))),
    flex(frame(text(L"right")))
  );
~~~
~~~bash
┌────┐┌───────────────────────────────────┐┌───────────────────────────────────┐
│left││middle                             ││right                              │
└────┘└───────────────────────────────────┘└───────────────────────────────────┘
~~~

# Widget elements.

## text

The more simple widget. It display a text.
~~~cpp
  text(L"I am a piece of text");
~~~
~~~bash
I am a piece of text.
~~~

## frame
Add a border arround an element
~~~c+
frame(text(L"The element"))
~~~

~~~bash
┌───────────┐
│The element│
└───────────┘
~~~

## separator

Display a vertical or horizontal line to visually split the content of a
container in two.

~~~cpp
frame(hbox(
	vbox(
		text(L"left top"),
		text(L"left bottom")
	),
	separator(),
	vbox(
		text(L"right top"),
		text(L"right bottom")
	)
));
~~~

~~~bash
┌───────────┬────────────┐
│left top   │right top   │
│left bottom│right bottom│
└───────────┴────────────┘
~~~

## gauge


A gauge. It can be used to represent a progress bar.
~~~c+
frame(gauge(0.5))
~~~

~~~bash
┌────────────────────────────────────────────────────────────────────────────┐
│██████████████████████████████████████                                      │
└────────────────────────────────────────────────────────────────────────────┘
~~~

# Decorator (style)
A terminal console can usually display colored text and colored background.
The text can also have different effects: bold, dim, underlined, inverted.

Element bold(Element);
Element dim(Element);
Element inverted(Element);
Element underlined(Element);
Element color(Element);
Element bgcolor(Element);
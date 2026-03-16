# qlippy — Options Slider Full Spec

This document fully defines the options slider system.
No additional specs required.

--------------------------------
SETTINGS
--------------------------------

layout = compact | normal | big
opacity = float 0.6-1.0
theme = nord | catppuccin

expand_mode = on/off
compact_image_expand = on/off

dedupe = on/off
save_images = on/off

max_history = int


--------------------------------
LAYOUT RULES
--------------------------------

compact
- small row height
- small font
- small padding

normal
- default

big
- larger font
- larger padding
- larger preview


--------------------------------
EXPAND MODE
--------------------------------

expand_mode off
- no expand
- fixed height

expand_mode on
- allow expand

compact_image_expand on
- small preview

compact_image_expand off
- full preview


--------------------------------
THEME
--------------------------------

Theme object:

bg
panel
text
accent
border


--------------------------------
OPTIONS SLIDER UI
--------------------------------

gear icon in status bar

click → open slider

slider:
- right side
- 35% width
- animated
- dim background

sections:

Appearance
Behavior
History


--------------------------------
APPEARANCE
--------------------------------

theme
layout
opacity


--------------------------------
BEHAVIOR
--------------------------------

expand_mode
compact_image_expand
dedupe
save_images


--------------------------------
HISTORY
--------------------------------

max_history


--------------------------------
STORAGE
--------------------------------

sqlite table settings

key
value


--------------------------------
CODE RULES
--------------------------------

Settings class required
SettingsModel required

UI must not hardcode values

all values from settings


--------------------------------
IPC
--------------------------------

daemon reload settings
popup updates live


--------------------------------
IMPLEMENTATION ORDER
--------------------------------

1 settings schema
2 settings class
3 settings model
4 bindings
5 slider ui
6 theme
7 layout rules


--------------------------------
CONSTRAINTS
--------------------------------

do not redesign UI
do not add settings
follow spec exactly

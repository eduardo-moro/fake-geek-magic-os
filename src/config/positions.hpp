#ifndef POSITIONS_H
#define POSITIONS_H

struct
{
  int x = 144;
  int y = 130;
} main_item_pos;

struct
{
  int x = 40;
  int y = 35;
  int w = 160;
  int h = 30;
} top_item_pos;

struct
{
  int x = 40;
  int y = 198;
  int w = 160;
  int h = 30;
} bottom_item_pos;

struct
{
  int x = 40;
  int y = 110;
  int w = 40;
  int h = 40;
  long anim_delay = 200;
} icon_pos;

struct
{
  int x = 80;
  int y = 110;
  int w = 130;
  int h = 40;
} main_item_boundaries;

#endif // POSITIONS_H
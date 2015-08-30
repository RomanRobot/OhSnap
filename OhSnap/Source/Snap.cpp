#include "Snap.h"

enum SnapPosition
{
  Center,
  LeftCenter,
  LeftHalf,
  RightCenter,
  RightHalf,
  TopCenter,
  TopHalf,
  BottomCenter,
  BottomHalf,
  TopLeft,
  TopRight,
  BottomLeft,
  BottomRight,
  // Conditionals
  LeftSide,
  RightSide,
  TopSide,
  BottomSide,
  LeftSideTop,
  LeftSideCenter,
  LeftSideBottom,
  RightSideTop,
  RightSideCenter,
  RightSideBottom,
  TopSideLeft,
  TopSideCenter,
  TopSideRight,
  BottomSideLeft,
  BottomSideCenter,
  BottomSideRight
};

SnapPosition GetSnapPosition( const RECT& work_area, const POINT& point_in_snap_rect )
{
  const int width = (work_area.right-work_area.left) / 2;
  const int height = (work_area.bottom-work_area.top) / 2;
  const int x_offset = point_in_snap_rect.x - width;
  const int y_offset = point_in_snap_rect.y - height;
  const int x_distance = abs(x_offset);
  const int y_distance = abs(y_offset);

  SnapPosition snap_position;
  if( x_distance < width/4 && y_distance < height/4 )
  {
    snap_position = Center;
  }
  else
  {
    // Outside of center
    if( y_distance < height/4 )
    {
      // Horizontal
      if( x_offset < 0 )
      {
        // Left
        if( x_distance < width/4*3 )
        {
          snap_position = LeftCenter;
        }
        else
        {
          snap_position = LeftHalf;
        }
      }
      else
      {
        // Right
        if( x_distance < width/4*3 )
        {
          snap_position = RightCenter;
        }
        else
        {
          snap_position = RightHalf;
        }
      }
    }
    else if( x_distance < width/4 )
    {
      // Vertical
      if( y_offset < 0 )
      {
        // Top
        if( y_distance < height/4*3 )
        {
          snap_position = TopCenter;
        }
        else
        {
          snap_position = TopHalf;
        }
      }
      else
      {
        // Bottom
        if( y_distance < height/4*3 )
        {
          snap_position = BottomCenter;
        }
        else
        {
          snap_position = BottomHalf;
        }
      }
    }
    else
    {
      // Corner
      if( y_offset < 0 )
      {
        // Top
        if( x_offset < 0 )
        {
          snap_position = TopLeft;
        }
        else
        {
          snap_position = TopRight;
        }
      }
      else
      {
        // Bottom
        if( x_offset < 0 )
        {
          snap_position = BottomLeft;
        }
        else
        {
          snap_position = BottomRight;
        }
      }
    }
  }
  // TODO: Conditional snap positions.

  return snap_position;
}

RECT GetSnapRect( const RECT& work_area, const POINT& point_in_snap_rect )
{
  const int width = (work_area.right-work_area.left) / 2;
  const int height = (work_area.bottom-work_area.top) / 2;

  RECT snap_rect = {};
  snap_rect.right = width;
  snap_rect.bottom = height;

  switch( GetSnapPosition(work_area, point_in_snap_rect) )
  {
  case Center:
    snap_rect.left = width;
    snap_rect.top  = height/2;
    break;
  case LeftCenter:
    snap_rect.left = width/2;
    snap_rect.top  = height/2;
    break;
  case LeftHalf:
    snap_rect.left   = width/2;
    snap_rect.top    = 0;
    snap_rect.bottom = height*2;
    break;
  case RightCenter:
    snap_rect.left = width+width/2;
    snap_rect.top  = height/2;
    break;
  case RightHalf:
    snap_rect.left   = width+width/2;
    snap_rect.top    = 0;
    snap_rect.bottom = height*2;
    break;
  case TopCenter:
    snap_rect.left = width;
    snap_rect.top  = 0;
    break;
  case TopHalf:
    snap_rect.left  = width;
    snap_rect.top   = 0;
    snap_rect.right = width*2;
    break;
  case BottomCenter:
    snap_rect.left = width;
    snap_rect.top  = height;
    break;
  case BottomHalf:
    snap_rect.left  = width;
    snap_rect.top   = height;
    snap_rect.right = width*2;
    break;
  case TopLeft:
    snap_rect.left = width/2;
    snap_rect.top  = 0;
    break;
  case TopRight:
    snap_rect.left = width+width/2;
    snap_rect.top  = 0;
    break;
  case BottomLeft:
    snap_rect.left = width/2;
    snap_rect.top  = height;
    break;
  case BottomRight:
    snap_rect.left = width+width/2;
    snap_rect.top  = height;
    break;
  }

  snap_rect.left -= snap_rect.right / 2;

  snap_rect.left += work_area.left;
  snap_rect.top += work_area.top;

  return snap_rect;
}

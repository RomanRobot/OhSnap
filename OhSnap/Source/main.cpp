#include <vector>

#include "Snap.h"

std::vector<BYTE> GetRawInput( HRAWINPUT raw_input_handle )
{
  std::vector<BYTE> raw_input;

  UINT size = 0;
  GetRawInputData( raw_input_handle, RID_INPUT, NULL, &size, sizeof(RAWINPUTHEADER) );
  if( size > 0 )
  {
    raw_input.resize( size );
    if( GetRawInputData(raw_input_handle, RID_INPUT, raw_input.data(), &size, sizeof(RAWINPUTHEADER)) != size )
    {
      raw_input.clear();
    }
  }

  return raw_input;
}

RECT GetWorkArea()
{
  RECT work_area = {};

  HWND desktop_window = GetDesktopWindow();
  HDC hdc = GetDC( desktop_window );
  work_area.right = GetDeviceCaps( hdc, HORZRES );
  work_area.bottom = GetDeviceCaps( hdc, VERTRES );
  ReleaseDC( desktop_window, hdc );

  HWND taskbar = FindWindow( "Shell_TrayWnd", nullptr );
  if( taskbar != NULL )
  {
    APPBARDATA taskbar_data;
    taskbar_data.cbSize = sizeof(APPBARDATA);
    taskbar_data.hWnd = taskbar;
    UINT taskbar_state = SHAppBarMessage( ABM_GETSTATE, &taskbar_data );
    bool taskbar_autohide = (taskbar_state & ABS_AUTOHIDE) != 0;
    if( taskbar_autohide == false )
    {
      if( SHAppBarMessage(ABM_GETTASKBARPOS, &taskbar_data) == TRUE )
      {
        RECT taskbar_rect = taskbar_data.rc;
        switch( taskbar_data.uEdge )
        {
        case 0: // Left
          work_area.left = taskbar_rect.right;
          break;
        case 1: // Top
          work_area.top = taskbar_rect.bottom;
          break;
        case 2: // Right
          work_area.right = taskbar_rect.left;
          break;
        case 3: // Bottom
          work_area.bottom = taskbar_rect.top;
          break;
        }
      }
    }
  }

  return work_area;
}

LRESULT __stdcall WindowProcedure( HWND window_handle, UINT message, WPARAM wParam, LPARAM lParam )
{
  if( message == WM_INPUT )
  {
    std::vector<BYTE> raw_input_data = GetRawInput( (HRAWINPUT)lParam );
    const RAWINPUT* raw_input = (const RAWINPUT*)raw_input_data.data();

    if( raw_input != nullptr && raw_input->header.dwType == RIM_TYPEMOUSE )
    {
      static HWND snapping_window;
      if( raw_input->data.mouse.usButtonFlags == RI_MOUSE_RIGHT_BUTTON_DOWN )
      {
        POINT cursor_pos;
        GetCursorPos( &cursor_pos );
        HWND window_from_point = WindowFromPoint( cursor_pos );
        TITLEBARINFO title_bar_info;
        title_bar_info.cbSize = sizeof(TITLEBARINFO);
        if( GetTitleBarInfo(window_from_point, &title_bar_info) == TRUE )
        {
          if( PtInRect(&title_bar_info.rcTitleBar, cursor_pos) == TRUE )
          {
            snapping_window = window_from_point;
            // TODO: Show grid.
          }
        }
      }
      else if( snapping_window != NULL )
      {
        if( raw_input->data.mouse.usButtonFlags == RI_MOUSE_RIGHT_BUTTON_UP )
        {
          snapping_window = NULL;
          // TODO: Hide grid.
          // TODO: Disable context menu.
        }
        else
        {
          POINT cursor_pos;
          if( GetCursorPos(&cursor_pos) == TRUE )
          {
            RECT window_rect = GetSnapRect( GetWorkArea(), cursor_pos );

            static RECT last_window_rect = {};
            if( window_rect.left   != last_window_rect.left
                || window_rect.right  != last_window_rect.right
                || window_rect.top    != last_window_rect.top
                || window_rect.bottom != last_window_rect.bottom )
            {
              SetWindowPos( snapping_window, NULL, window_rect.left, window_rect.top, window_rect.right, window_rect.bottom, SWP_NOZORDER | SWP_NOSENDCHANGING );
              last_window_rect = window_rect;
            }
          }
        }
      }
    }

    return 0;
  }
  return DefWindowProc( window_handle, message, wParam, lParam );
}

HWND CreateMessageOnlyWindow( const char* class_name, HINSTANCE instance_handle )
{
  WNDCLASS window_class = {};
  window_class.style = CS_HREDRAW | CS_VREDRAW;
  window_class.hInstance = instance_handle;
  window_class.lpszClassName = class_name;
  window_class.lpfnWndProc = (WNDPROC)WindowProcedure;
  RegisterClass( &window_class );

  HWND window_handle = CreateWindow( class_name, NULL, NULL, 0, 0, 0, 0, HWND_MESSAGE, NULL, instance_handle, 0 );

  if( window_handle != NULL )
  {
    SetWindowPos( window_handle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
    UpdateWindow( window_handle );
  }

  return window_handle;
}

bool RegisterMouse( HWND window_handle )
{
  RAWINPUTDEVICE mouse_rid = {};
  mouse_rid.usUsagePage = 0x01;
  mouse_rid.usUsage = 0x02;
  mouse_rid.dwFlags = RIDEV_NOLEGACY | RIDEV_INPUTSINK;
  mouse_rid.hwndTarget = window_handle;

  return RegisterRawInputDevices( &mouse_rid, 1, sizeof(mouse_rid) ) == TRUE;
}

int APIENTRY WinMain( HINSTANCE instance_handle, HINSTANCE previous_instance_handle, LPSTR command_line, int command_show )
{
  const char* class_name = "OhSnap";

  HWND window_handle = CreateMessageOnlyWindow( class_name, instance_handle );
  if( window_handle == NULL )
  {
    MessageBox( NULL, "Failed to create window.", class_name, MB_ICONERROR );
  }

  bool registered_mouse = RegisterMouse( window_handle );
  if( registered_mouse == false )
  {
    MessageBox( NULL, "Failed to register mouse.", class_name, MB_ICONERROR );
  }

  if( window_handle != NULL && registered_mouse == true )
  {
    MSG msg;
    while( GetMessage(&msg, NULL, 0, 0) )
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }

  UnregisterClass( class_name, instance_handle );

  return 0;
}

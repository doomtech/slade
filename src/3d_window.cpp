
#include "main.h"
#include "structs.h"
#include "camera.h"
#include "render.h"
#include "keybind.h"
#include "input.h"
#include "3dmode.h"
#include "editor_window.h"
#include "3d_window.h"
#include "doom_map.h"
#include "bsp.h"
#include "splash.h"

#include <wx/image.h>

wxFrame *frame_3d = NULL;

double elapsed = 0;
CVAR(Bool, invert_mouse_3d, false, CVAR_SAVE)
CVAR(Int, vid_width_3d, -1, CVAR_SAVE)
CVAR(Int, vid_height_3d, -1, CVAR_SAVE)
CVAR(Int, vid_bpp_3d, -1, CVAR_SAVE)
CVAR(Int, vid_refresh_3d, -1, CVAR_SAVE)
CVAR(Int, vid_frametime_3d, 10, CVAR_SAVE)

//extern wxGLContext *gl_context;
extern wxGLCanvas *share_canvas;
extern EditorWindow *editor_window;
extern Camera camera;
extern BindList binds;
extern DoomMap d_map;
extern bool* lines_visible;

EXTERN_CVAR(Float, mouse_speed_3d)

#ifdef WIN32
DEVMODE current_mode;
#endif

void set_3d_resolution()
{
#ifdef WIN32
	EnumDisplaySettings(0, ENUM_CURRENT_SETTINGS, &current_mode);

	if (vid_width_3d != -1 &&
		vid_height_3d != -1 &&
		vid_bpp_3d != -1 &&
		vid_refresh_3d != -1)
	{
		DEVMODE new_mode;
		new_mode.dmFields = DM_PELSWIDTH|DM_PELSHEIGHT|DM_BITSPERPEL|DM_DISPLAYFREQUENCY;
		new_mode.dmPelsWidth = vid_width_3d;
		new_mode.dmPelsHeight = vid_height_3d;
		new_mode.dmBitsPerPel = vid_bpp_3d;
		new_mode.dmDisplayFrequency = vid_refresh_3d;

		long ret = ChangeDisplaySettings(&new_mode, CDS_FULLSCREEN);

		if (ret == DISP_CHANGE_RESTART)
			wxMessageBox(_T("Restart required to set screen resolution"));

		if (ret == DISP_CHANGE_BADFLAGS)
			wxMessageBox(_T("Error: Bad flags"));

		if (ret == DISP_CHANGE_BADPARAM)
			wxMessageBox(_T("Error: Bad parameters"));

		if (ret == DISP_CHANGE_FAILED)
			wxMessageBox(_T("Resolution change failed"));

		if (ret == DISP_CHANGE_BADMODE)
			wxMessageBox(_T("Resolution not supported"));

		//if (ret == DISP_CHANGE_NOTUPDATED)
		//	wxMessageBox(_T("Restart required to set screen resolution!"));
	}
#endif
}

void restore_resolution()
{
#ifdef WIN32
	ChangeDisplaySettings(&current_mode, 0);
#endif
}

Render3dCanvas::Render3dCanvas(wxWindow *parent)
:	wxGLCanvas(parent, share_canvas, -1)
{
}

Render3dCanvas::~Render3dCanvas()
{
	editor_window->Show(true);
	restore_resolution();
	frame_3d = NULL;
}

void Render3dCanvas::render()
{
	SetCurrent();

	glViewport(0, 0, GetClientSize().x, GetClientSize().y);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(60.0f, (GLfloat)320/(GLfloat)200, 0.1f, 1000.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	render_3d_view(GetClientSize().x, GetClientSize().y);

	SwapBuffers();
}

void Render3dCanvas::init()
{
	SetCurrent();

	GLfloat fogColor[4]= {0.0f, 0.0f, 0.0f, 1.0f};

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0);
	glShadeModel(GL_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glCullFace(GL_FRONT);
	glEnable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDepthRange(0, 1);

	glFogi(GL_FOG_MODE, GL_LINEAR);
	glFogfv(GL_FOG_COLOR, fogColor);
	glFogf(GL_FOG_DENSITY, 2.0f);
	glHint(GL_FOG_HINT, GL_NICEST);
	glFogf(GL_FOG_START, 0.0f);
	glFogf(GL_FOG_END, 40.0f);
	glEnable(GL_FOG);
}

BEGIN_EVENT_TABLE(Render3dCanvas, wxGLCanvas)
	EVT_IDLE(Render3dCanvas::idle_event)
	EVT_PAINT(Render3dCanvas::paint_event)
	EVT_KEY_DOWN(Render3dCanvas::key_down)
	EVT_KEY_UP(Render3dCanvas::key_up)
	EVT_MOUSE_EVENTS(Render3dCanvas::mouse_event)
	//EVT_SIZE(Render3dCanvas::size_event)
END_EVENT_TABLE()

wxStopWatch sw;

void Render3dCanvas::do_loop()
{
	elapsed = sw.Time();
	if (elapsed >= vid_frametime_3d)
	{
		// Reset timer
		sw.Start();

		// Process input
		if (!keys_3d(elapsed * 0.1))
		{
			if (GetParent())
				GetParent()->Destroy();

			return;
		}

		// Gravity
		if (camera.gravity)
			apply_gravity();
	}

	// Render
	render();
}

void Render3dCanvas::idle_event(wxIdleEvent &event)
{
	do_loop();

	event.RequestMore();
}

void Render3dCanvas::paint_event(wxPaintEvent &event)
{
	wxPaintDC dc(this);
	render();
}

void Render3dCanvas::key_down(wxKeyEvent &event)
{
	binds.set(get_key_name(event.GetKeyCode()), NULL, event.ShiftDown(), event.ControlDown(), event.AltDown());
}

void Render3dCanvas::key_up(wxKeyEvent &event)
{
	binds.unset(get_key_name(event.GetKeyCode()), NULL, event.ShiftDown(), event.ControlDown(), event.AltDown());
}

bool reset = true;
void Render3dCanvas::mouse_event(wxMouseEvent &event)
{
	if (event.LeftDown() || event.LeftDClick())
		binds.set(_T("Mouse1"), NULL, event.ShiftDown(), event.ControlDown(), event.AltDown());

	if (event.MiddleDown() || event.MiddleDClick())
		binds.set(_T("Mouse2"), NULL, event.ShiftDown(), event.ControlDown(), event.AltDown());

	if (event.RightDown() || event.RightDClick())
		binds.set(_T("Mouse3"), NULL, event.ShiftDown(), event.ControlDown(), event.AltDown());

	if (event.LeftUp())
		binds.unset(_T("Mouse1"), NULL, event.ShiftDown(), event.ControlDown(), event.AltDown());

	if (event.MiddleUp())
		binds.unset(_T("Mouse2"), NULL, event.ShiftDown(), event.ControlDown(), event.AltDown());

	if (event.RightUp())
		binds.unset(_T("Mouse3"), NULL, event.ShiftDown(), event.ControlDown(), event.AltDown());

	if (event.GetWheelRotation() > 0)
	{
		binds.set(_T("MWheel Up"), NULL, event.ShiftDown(), event.ControlDown(), event.AltDown());
		keys_3d(1.0f, true);
	}
	else if (event.GetWheelRotation() < 0)
	{
		binds.set(_T("MWheel Down"), NULL, event.ShiftDown(), event.ControlDown(), event.AltDown());
		keys_3d(1.0f, true);
	}

	binds.unset(_T("MWheel Up"), NULL, event.ShiftDown(), event.ControlDown(), event.AltDown());
	binds.unset(_T("MWheel Down"), NULL, event.ShiftDown(), event.ControlDown(), event.AltDown());

	if (event.Moving())
	{
		if (reset)
		{
			reset = false;
			return;
		}

		int center_x = GetClientSize().x * 0.5;
		int center_y = GetClientSize().y * 0.5;
		float angle_x = -(event.GetX() - center_x) * (0.001f * mouse_speed_3d);
		float angle_y = -(event.GetY() - center_y) * (0.001f * mouse_speed_3d);

		if (invert_mouse_3d)
			angle_y = -angle_y;

		point3_t axis = (camera.view - camera.position).cross(camera.up_vector);
		axis = axis.normalize();

		camera.rotate_view(angle_x, 0, 0, 1);
		camera.rotate_view(angle_y, axis.x, axis.y, axis.z);

		reset = true;
		WarpPointer(center_x, center_y);
	}
}

void Render3dCanvas::size_event(wxSizeEvent &event)
{
	SetCurrent();

	GLfloat fogColor[4]= {0.0f, 0.0f, 0.0f, 1.0f};

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0);
	glShadeModel(GL_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glCullFace(GL_FRONT);
	glEnable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDepthRange(0, 1);

	glFogi(GL_FOG_MODE, GL_LINEAR);
	glFogfv(GL_FOG_COLOR, fogColor);
	glFogf(GL_FOG_DENSITY, 2.0f);
	glHint(GL_FOG_HINT, GL_NICEST);
	glFogf(GL_FOG_START, 0.0f);
	glFogf(GL_FOG_END, 40.0f);
	glEnable(GL_FOG);

	glViewport(0, 0, GetClientSize().x, GetClientSize().y);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(60.0f, (GLfloat)320/(GLfloat)200, 0.1f, 1000.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	render();
}

void start_3d_mode()
{
	if (d_map.n_sectors() <= 1)
	{
		wxMessageBox(_T("You need at least 2 sectors to start 3d mode"));
		return;
	}

	// Setup
	if (!build_gl_nodes())
		return;
	remove("sladetemp");

	splash(_T("Setup 3d mode data"));

	if (lines_visible) delete lines_visible;
	lines_visible = new bool[d_map.n_lines()];

	init_3d_mode();

	splash_hide();

	// Set resolution
	set_3d_resolution();

	// Create window
	frame_3d = new wxFrame(NULL, -1, _T("SLADE (3d Mode)"));
	Render3dCanvas *canvas_3d = new Render3dCanvas(frame_3d);
	wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
	frame_3d->SetSizer(sizer);
	sizer->Add(canvas_3d, 1, wxEXPAND);

	// Hide cursor
	wxImage img(32, 32, true);
	img.SetMask(true);
	img.SetMaskColour(0, 0, 0);
	frame_3d->SetCursor(wxCursor(img));

	editor_window->Hide();
	frame_3d->ShowFullScreen(true);
	canvas_3d->SetFocus();
	canvas_3d->init();
	canvas_3d->render();

	reset_3d_mouse();
}

void reset_3d_mouse()
{
	if (!frame_3d)
		return;

	reset = true;
	frame_3d->WarpPointer(frame_3d->GetClientSize().x * 0.5, frame_3d->GetClientSize().y * 0.5);
}

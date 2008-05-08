
#include "main.h"
#include "structs.h"
#include "camera.h"

#include <GL/glu.h>

// Most of this stuff was taken from various tutorials on the net,
// hence the unusual comments :P

Camera camera;

Camera::Camera()
{
	position.set(40.0f, 40.0f, 20.0f);
	view.set(40.0f, 41.0f, 20.8f);
	up_vector.set(0.0f, 0.0f, 1.0f);
	gravity = true;
}

bool Camera::in_frustum(point3_t point, float size)
{
	for (int d = 0; d < 6; d++)
	{
		if(frustum[d].a * point.x + frustum[d].b * point.y + frustum[d].c * point.z + frustum[d].d <= -size)
			return false;
	}

	return true;
}

void Camera::position_camera(float pos_x,	float pos_y,	float pos_z,
			 				float view_x,	float view_y,	float view_z,
							float up_x,		float up_y,		float up_z)
{
	position.set(pos_x, pos_y, pos_z);
	view.set(view_x, view_y, view_z);
	up_vector.set(up_x, up_y, up_z);
}

void Camera::rotate_view(float angle, float x, float y, float z)
{
	point3_t new_view;

	// Get the view vector (The direction we are facing)
	point3_t v_view = view - position;

	// Calculate the sine and cosine of the angle once
	float cosTheta = (float)cos(angle);
	float sinTheta = (float)sin(angle);

	// Find the new x position for the new rotated point
	new_view.x  = (cosTheta + (1 - cosTheta) * x * x)		* v_view.x;
	new_view.x += ((1 - cosTheta) * x * y - z * sinTheta)	* v_view.y;
	new_view.x += ((1 - cosTheta) * x * z + y * sinTheta)	* v_view.z;

	// Find the new y position for the new rotated point
	new_view.y  = ((1 - cosTheta) * x * y + z * sinTheta)	* v_view.x;
	new_view.y += (cosTheta + (1 - cosTheta) * y * y)		* v_view.y;
	new_view.y += ((1 - cosTheta) * y * z - x * sinTheta)	* v_view.z;

	// Find the new z position for the new rotated point
	new_view.z  = ((1 - cosTheta) * x * z - y * sinTheta)	* v_view.x;
	new_view.z += ((1 - cosTheta) * y * z + x * sinTheta)	* v_view.y;
	new_view.z += (cosTheta + (1 - cosTheta) * z * z)		* v_view.z;

	// Now we just add the newly rotated vector to our position to set
	// our new rotated view of our camera.
	view = position + new_view;
}

void Camera::strafe_camera(float speed)
{	
	// Add the strafe vector to our position
	position.x += strafe.x * speed;
	position.y += strafe.y * speed;

	// Add the strafe vector to our view
	view.x += strafe.x * speed;
	view.y += strafe.y * speed;
}

void Camera::move_camera(float speed)
{
	// Get the current view vector (the direction we are looking)
	point3_t vector = view - position;
	vector.set(vector.normalize());

	position.x += vector.x * speed;		// Add our acceleration to our position's X
	position.y += vector.y * speed;		// Add our acceleration to our position's Y

	if (!gravity)
		position.z += vector.z * speed;		// Add our acceleration to our position's Z

	view.x += vector.x * speed;			// Add our acceleration to our view's X
	view.y += vector.y * speed;			// Add our acceleration to our view's Y

	if (!gravity)
		view.z += vector.z * speed;			// Add our acceleration to our view's Z
}

void Camera::update() 
{
	point3_t v_cross = (view - position).cross(up_vector);
	strafe = v_cross.normalize();
}

void Camera::look()
{
	gluLookAt(position.x, position.y, position.z,	
			  view.x,	 view.y,     view.z,	
			  up_vector.x, up_vector.y, up_vector.z);
}

struct matrix_44_t
{
	float _11, _21, _31, _41;
	float _12, _22, _32, _42;
	float _13, _23, _33, _43;
	float _14, _24, _34, _44;

	matrix_44_t() {}

	matrix_44_t(float *matrix)
	{
		_11 = matrix[0];
		_21 = matrix[1];
		_31 = matrix[2];
		_41 = matrix[3];
		_12 = matrix[4];
		_22 = matrix[5];
		_32 = matrix[6];
		_42 = matrix[7];
		_13 = matrix[8];
		_23 = matrix[9];
		_33 = matrix[10];
		_43 = matrix[11];
		_14 = matrix[12];
		_24 = matrix[13];
		_34 = matrix[14];
		_44 = matrix[15];
	}
};

void Camera::get_frustum()
{
	float   proj[16];	// This will hold our projection matrix
	float   modl[16];	// This will hold our modelview matrix
	float   clip[16];	// This will hold the clipping planes

	// glGetFloatv() is used to extract information about our OpenGL world.
	// Below, we pass in GL_PROJECTION_MATRIX to abstract our projection matrix.
	// It then stores the matrix into an array of [16].
	glGetFloatv( GL_PROJECTION_MATRIX, proj );

	// By passing in GL_MODELVIEW_MATRIX, we can abstract our model view matrix.
	// This also stores it in an array of [16].
	glGetFloatv( GL_MODELVIEW_MATRIX, modl );

	// Now that we have our modelview and projection matrix, if we combine these 2 matrices,
	// it will give us our clipping planes.  To combine 2 matrices, we multiply them.
	clip[ 0] = modl[ 0] * proj[ 0] + modl[ 1] * proj[ 4] + modl[ 2] * proj[ 8] + modl[ 3] * proj[12];
	clip[ 1] = modl[ 0] * proj[ 1] + modl[ 1] * proj[ 5] + modl[ 2] * proj[ 9] + modl[ 3] * proj[13];
	clip[ 2] = modl[ 0] * proj[ 2] + modl[ 1] * proj[ 6] + modl[ 2] * proj[10] + modl[ 3] * proj[14];
	clip[ 3] = modl[ 0] * proj[ 3] + modl[ 1] * proj[ 7] + modl[ 2] * proj[11] + modl[ 3] * proj[15];

	clip[ 4] = modl[ 4] * proj[ 0] + modl[ 5] * proj[ 4] + modl[ 6] * proj[ 8] + modl[ 7] * proj[12];
	clip[ 5] = modl[ 4] * proj[ 1] + modl[ 5] * proj[ 5] + modl[ 6] * proj[ 9] + modl[ 7] * proj[13];
	clip[ 6] = modl[ 4] * proj[ 2] + modl[ 5] * proj[ 6] + modl[ 6] * proj[10] + modl[ 7] * proj[14];
	clip[ 7] = modl[ 4] * proj[ 3] + modl[ 5] * proj[ 7] + modl[ 6] * proj[11] + modl[ 7] * proj[15];

	clip[ 8] = modl[ 8] * proj[ 0] + modl[ 9] * proj[ 4] + modl[10] * proj[ 8] + modl[11] * proj[12];
	clip[ 9] = modl[ 8] * proj[ 1] + modl[ 9] * proj[ 5] + modl[10] * proj[ 9] + modl[11] * proj[13];
	clip[10] = modl[ 8] * proj[ 2] + modl[ 9] * proj[ 6] + modl[10] * proj[10] + modl[11] * proj[14];
	clip[11] = modl[ 8] * proj[ 3] + modl[ 9] * proj[ 7] + modl[10] * proj[11] + modl[11] * proj[15];

	clip[12] = modl[12] * proj[ 0] + modl[13] * proj[ 4] + modl[14] * proj[ 8] + modl[15] * proj[12];
	clip[13] = modl[12] * proj[ 1] + modl[13] * proj[ 5] + modl[14] * proj[ 9] + modl[15] * proj[13];
	clip[14] = modl[12] * proj[ 2] + modl[13] * proj[ 6] + modl[14] * proj[10] + modl[15] * proj[14];
	clip[15] = modl[12] * proj[ 3] + modl[13] * proj[ 7] + modl[14] * proj[11] + modl[15] * proj[15];

	matrix_44_t comboMatrix(clip);
	memcpy(view_matrix, clip, 16 * sizeof(float));

	// Left clipping plane
	frustum[0].a = comboMatrix._41 + comboMatrix._11;
	frustum[0].b = comboMatrix._42 + comboMatrix._12;
	frustum[0].c = comboMatrix._43 + comboMatrix._13;
	frustum[0].d = comboMatrix._44 + comboMatrix._14;
	frustum[0].normalize();

	// Right clipping plane
	frustum[1].a = comboMatrix._41 - comboMatrix._11;
	frustum[1].b = comboMatrix._42 - comboMatrix._12;
	frustum[1].c = comboMatrix._43 - comboMatrix._13;
	frustum[1].d = comboMatrix._44 - comboMatrix._14;
	frustum[1].normalize();

	// Top clipping plane
	frustum[2].a = comboMatrix._41 - comboMatrix._21;
	frustum[2].b = comboMatrix._42 - comboMatrix._22;
	frustum[2].c = comboMatrix._43 - comboMatrix._23;
	frustum[2].d = comboMatrix._44 - comboMatrix._24;
	frustum[2].normalize();

	// Bottom clipping plane
	frustum[3].a = comboMatrix._41 + comboMatrix._21;
	frustum[3].b = comboMatrix._42 + comboMatrix._22;
	frustum[3].c = comboMatrix._43 + comboMatrix._23;
	frustum[3].d = comboMatrix._44 + comboMatrix._24;
	frustum[3].normalize();

	// Near clipping plane
	frustum[4].a = comboMatrix._41 + comboMatrix._31;
	frustum[4].b = comboMatrix._42 + comboMatrix._32;
	frustum[4].c = comboMatrix._43 + comboMatrix._33;
	frustum[4].d = comboMatrix._44 + comboMatrix._34;
	frustum[4].normalize();

	// Far clipping plane
	frustum[5].a = comboMatrix._41 - comboMatrix._31;
	frustum[5].b = comboMatrix._42 - comboMatrix._32;
	frustum[5].c = comboMatrix._43 - comboMatrix._33;
	frustum[5].d = comboMatrix._44 - comboMatrix._34;
	frustum[5].normalize();
}

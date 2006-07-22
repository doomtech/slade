
class Camera {

public:
	point3_t	position;
	point3_t	view;
	point3_t	up_vector;
	point3_t	strafe;

	plane_t		frustum[6];
	float		view_matrix[16];

	point3_t	view_point;
	float		view_radius;

	bool		gravity;

	Camera();	

	void position_camera(float pos_x,	float pos_y,	float pos_z,
			 		    float view_x,	float view_y,	float view_z,
						float up_x,		float up_y,		float up_z);

	void rotate_view(float angle, float x, float y, float z);
	//void SetViewByMouse(); 
	void rotate_point(point3_t center, float x, float y, float z);
	void strafe_camera(float speed);
	void move_camera(float speed);
	//void CheckForMovement();
	void update();
	void look();
	void get_frustum();

	bool in_frustum(point3_t point, float size);
};

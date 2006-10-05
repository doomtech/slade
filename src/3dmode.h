
void init_3d_mode();
void apply_gravity();
void camera_to_start();
void change_offsets_3d(int x, int y);
void change_sector_height_3d(int amount, bool floor = true, bool force = false);
void change_texture_3d(bool paint);
void reset_3d_mouse();
void paste_texture_3d(bool paint);
void copy_texture_3d();
void copy_side_3d(bool xoff, bool yoff, bool textures = false);
void paste_side_3d(bool xoff, bool yoff, bool textures = false);
void toggle_texture_peg_3d(bool upper);
void change_light_3d(int amount);
void reset_offsets_3d(bool x = true, bool y = true);
void auto_align_x_3d();
void change_thing_angle_3d(int amount);
void change_thing_z_3d(int amount);

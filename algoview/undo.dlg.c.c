
DIALOG dlg_undo_list[] =
{
   /* (proc)            (x)  (y) (w)  (h)  (fg) (bg)   (key) (flags) (d1) (d2) (dp)              (dp2) (dp3) */
   { d_shadow_box_proc, 24,  24, 596, 448, 0,   65535, 0,    0,      0,   0,   NULL,             NULL, NULL },
   { d_list_proc,       36,  72, 568, 388, 0,   65535, 0,    D_EXIT, 0,   0,   undo_list_getter, NULL, NULL },
   { d_text_proc,       36,  32, 160, 32,  0,   65535, 0,    0,      0,   0,   "Undo list",      NULL, NULL },
   { d_button_proc,     204, 32, 396, 32,  0,   65535, 0,    D_EXIT, 0,   0,   "CANCEL",         NULL, NULL },
   { NULL,              0,   0,  0,   0,   0,   0,     0,    0,      0,   0,   NULL,             NULL, NULL }
};

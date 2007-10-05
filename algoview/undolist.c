DIALOG dlg[] =
{
   /* (proc)            (x)  (y) (w)  (h)  (fg) (bg)   (key) (flags) (d1) (d2) (dp)          (dp2) (dp3) */
   { d_shadow_box_proc, 64,  24, 368, 448, 0,   65535, 0,    0,      0,   0,   NULL,         NULL, NULL },
   { d_list_proc,       76,  72, 344, 388, 0,   65535, 0,    0,      0,   0,   dummy_getter, NULL, NULL },
   { d_text_proc,       76,  32, 160, 32,  0,   65535, 0,    0,      0,   0,   "Undo list",  NULL, NULL },
   { d_button_proc,     248, 32, 168, 32,  0,   65535, 0,    0,      0,   0,   "CANCEL",     NULL, NULL },
   { NULL,              0,   0,  0,   0,   0,   0,     0,    0,      0,   0,   NULL,         NULL, NULL }
};
